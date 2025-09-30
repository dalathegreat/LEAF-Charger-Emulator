#include <Arduino.h>
#include "HardwareSerial.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "src/charger/CHARGERS.h"
#include "src/communication/Transmitter.h"
#include "src/communication/can/comm_can.h"
#include "src/communication/nvm/comm_nvm.h"
#include "src/datalayer/datalayer.h"
#include "src/devboard/sdcard/sdcard.h"
#include "src/devboard/utils/events.h"
#include "src/devboard/utils/led_handler.h"
#include "src/devboard/utils/logging.h"
#include "src/devboard/utils/time_meas.h"
#include "src/devboard/utils/timer.h"
#include "src/devboard/utils/types.h"
#include "src/devboard/utils/value_mapping.h"
#include "src/devboard/webserver/webserver.h"
#include "src/devboard/wifi/wifi.h"

// The current software version, shown on webserver
const char* version_number = "1.0.dev";

// Interval timers
volatile unsigned long currentMillis = 0;
unsigned long previousMillis10ms = 0;
unsigned long previousMillisUpdateVal = 0;
// Task time measurement for debugging
TaskHandle_t main_loop_task;
TaskHandle_t connectivity_loop_task;

Logging logging;

static std::list<Transmitter*> transmitters;
void register_transmitter(Transmitter* transmitter) {
  transmitters.push_back(transmitter);
  DEBUG_PRINTF("transmitter registered, total: %d\n", transmitters.size());
}

// Initialization functions
void init_serial() {
  // Init Serial monitor
  Serial.begin(115200);
#if HW_LILYGO2CAN
  // Wait up to 100ms for Serial to be available. On the ESP32S3 Serial is
  // provided by the USB controller, so will only work if the board is connected
  // to a computer.
  for (int i = 0; i < 10; i++) {
    if (Serial)
      break;
    delay(10);
  }
#else
  while (!Serial) {}
#endif
}

void connectivity_loop(void*) {
  esp_task_wdt_add(NULL);  // Register this task with WDT
  // Init wifi
  init_WiFi();

  init_webserver();

  if (mdns_enabled) {
    init_mDNS();
  }

  while (true) {
    START_TIME_MEASUREMENT(wifi);
    wifi_monitor();

    ota_monitor();

    END_TIME_MEASUREMENT_MAX(wifi, datalayer.system.status.wifi_task_10s_max_us);

    esp_task_wdt_reset();  // Reset watchdog
    delay(1);
  }
}

void check_reset_reason() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_UNKNOWN:  //Reset reason can not be determined
      set_event(EVENT_RESET_UNKNOWN, reason);
      break;
    case ESP_RST_POWERON:  //OK Reset due to power-on event
      set_event(EVENT_RESET_POWERON, reason);
      break;
    case ESP_RST_EXT:  //Reset by external pin (not applicable for ESP32)
      set_event(EVENT_RESET_EXT, reason);
      break;
    case ESP_RST_SW:  //OK Software reset via esp_restart
      set_event(EVENT_RESET_SW, reason);
      break;
    case ESP_RST_PANIC:  //Software reset due to exception/panic
      set_event(EVENT_RESET_PANIC, reason);
      break;
    case ESP_RST_INT_WDT:  //Reset (software or hardware) due to interrupt watchdog
      set_event(EVENT_RESET_INT_WDT, reason);
      break;
    case ESP_RST_TASK_WDT:  //Reset due to task watchdog
      set_event(EVENT_RESET_TASK_WDT, reason);
      break;
    case ESP_RST_WDT:  //Reset due to other watchdogs
      set_event(EVENT_RESET_WDT, reason);
      break;
    case ESP_RST_DEEPSLEEP:  //Reset after exiting deep sleep mode
      set_event(EVENT_RESET_DEEPSLEEP, reason);
      break;
    case ESP_RST_BROWNOUT:  //Brownout reset (software or hardware)
      set_event(EVENT_RESET_BROWNOUT, reason);
      break;
    case ESP_RST_SDIO:  //Reset over SDIO
      set_event(EVENT_RESET_SDIO, reason);
      break;
    case ESP_RST_USB:  //Reset by USB peripheral
      set_event(EVENT_RESET_USB, reason);
      break;
    case ESP_RST_JTAG:  //Reset by JTAG
      set_event(EVENT_RESET_JTAG, reason);
      break;
    case ESP_RST_EFUSE:  //Reset due to efuse error
      set_event(EVENT_RESET_EFUSE, reason);
      break;
    case ESP_RST_PWR_GLITCH:  //Reset due to power glitch detected
      set_event(EVENT_RESET_PWR_GLITCH, reason);
      break;
    case ESP_RST_CPU_LOCKUP:  //Reset due to CPU lock up
      set_event(EVENT_RESET_CPU_LOCKUP, reason);
      break;
    default:
      break;
  }
}

void core_loop(void*) {
  esp_task_wdt_add(NULL);  // Register this task with WDT
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1);  // Convert 1ms to ticks

  while (true) {

    // Input, Runs as fast as possible
    receive_can();  // Receive CAN messages

    ElegantOTA.loop();

    // Process
    currentMillis = millis();
    if (currentMillis - previousMillis10ms >= INTERVAL_10_MS) {
      previousMillis10ms = currentMillis;

      led_exe();
    }

    // Let all transmitter objects send their messages
    for (auto& transmitter : transmitters) {
      transmitter->transmit(currentMillis);
    }

    esp_task_wdt_reset();  // Reset watchdog to prevent reset
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Initialization
void setup() {
  init_hal();

  init_serial();

  // We print this after setting up serial, so that is also printed if configured to do so
  DEBUG_PRINTF("LEAF Charger emulator %s build " __DATE__ " " __TIME__ "\n", version_number);

  init_events();

  init_stored_settings();

  if (wifi_enabled) {
    xTaskCreatePinnedToCore((TaskFunction_t)&connectivity_loop, "connectivity_loop", 4096, NULL, TASK_CONNECTIVITY_PRIO,
                            &connectivity_loop_task, esp32hal->WIFICORE());
  }

  led_init();

  setup_charger();

  // Init CAN only after any CAN receivers have had a chance to register.
  init_CAN();

  // BOOT button at runtime is used as an input for various things
  pinMode(0, INPUT_PULLUP);

  check_reset_reason();

  // Initialize Task Watchdog for subscribed tasks
  esp_task_wdt_config_t wdt_config = {// 5s should be enough for the connectivity tasks (which are all contending
                                      // for the same core) to yield to each other and reset their watchdogs.
                                      .timeout_ms = INTERVAL_5_S,
                                      // We don't benefit from idle task watchdogs, our critical loops have their
                                      // own. The idle watchdogs can cause nuisance reboots under heavy load.
                                      .idle_core_mask = 0,
                                      // Panic (and reboot) on timeout
                                      .trigger_panic = true};
#ifdef CONFIG_ESP_TASK_WDT
  // ESP-IDF will have already initialized it, so reconfigure.
  // Arduino and PlatformIO have different watchdog defaults, so we reconfigure
  // for consistency.
  esp_task_wdt_reconfigure(&wdt_config);
#else
  // Otherwise initialize it for the first time.
  esp_task_wdt_init(&wdt_config);
#endif

  // Start tasks

  xTaskCreatePinnedToCore((TaskFunction_t)&core_loop, "core_loop", 4096, NULL, TASK_CORE_PRIO, &main_loop_task,
                          esp32hal->CORE_FUNCTION_CORE());

  DEBUG_PRINTF("Setup complete!\n");
}

// Loop empty, all functionality runs in tasks
void loop() {}
