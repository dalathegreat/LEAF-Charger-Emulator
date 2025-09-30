#include "comm_nvm.h"
#include <soc/gpio_num.h>
#include <chrono>
#include "../../charger/CanCharger.h"
#include "../../communication/can/comm_can.h"
#include "../../devboard/wifi/wifi.h"

// Parameters
Preferences settings;  // Store user settings

// Initialization functions

void init_stored_settings() {
  static uint32_t temp = 0;
  //  ATTENTION ! The maximum length for settings keys is 15 characters
  settings.begin("batterySettings", false);

  //settings.clear();  // If this clear function is executed, no settings will be read from storage. For dev

  char tempSSIDstring[63];  // Allocate buffer with sufficient size
  size_t lengthSSID = settings.getString("SSID", tempSSIDstring, sizeof(tempSSIDstring));
  if (lengthSSID > 0) {  // Successfully read the string from memory. Set it to SSID!
    ssid = tempSSIDstring;
  } else {  // Reading from settings failed. Do nothing with SSID. Raise event?
  }
  char tempPasswordString[63];  // Allocate buffer with sufficient size
  size_t lengthPassword = settings.getString("PASSWORD", tempPasswordString, sizeof(tempPasswordString));
  if (lengthPassword > 7) {  // Successfully read the string from memory. Set it to password!
    password = tempPasswordString;
  } else {  // Reading from settings failed. Do nothing with SSID. Raise event?
  }

  user_selected_charger_type = (ChargerType)settings.getUInt("CHGTYPE", (int)ChargerType::None);
  user_selected_can_addon_crystal_frequency_mhz = settings.getUInt("CANFREQ", 16);
  user_selected_canfd_addon_crystal_frequency_mhz = settings.getUInt("CANFDFREQ", 40);

  auto readIf = [](const char* settingName) {
    auto batt1If = (comm_interface)settings.getUInt(settingName, (int)comm_interface::CanNative);
    switch (batt1If) {
      case comm_interface::CanNative:
        return CAN_Interface::CAN_NATIVE;
      case comm_interface::CanFdNative:
        return CAN_Interface::CANFD_NATIVE;
      case comm_interface::CanAddonMcp2515:
        return CAN_Interface::CAN_ADDON_MCP2515;
      case comm_interface::CanFdAddonMcp2518:
        return CAN_Interface::CANFD_ADDON_MCP2518;
    }

    return CAN_Interface::CAN_NATIVE;
  };

  can_config.charger = readIf("CHGCOMM");

  use_canfd_as_can = settings.getBool("CANFDASCAN", false);

  datalayer.system.info.performance_measurement_active = settings.getBool("PERFPROFILE", false);
  datalayer.system.info.CAN_usb_logging_active = settings.getBool("CANLOGUSB", false);
  datalayer.system.info.usb_logging_active = settings.getBool("USBENABLED", false);
  datalayer.system.info.web_logging_active = settings.getBool("WEBENABLED", false);
  datalayer.system.info.CAN_SD_logging_active = settings.getBool("CANLOGSD", false);
  datalayer.system.info.SD_logging_active = settings.getBool("SDLOGENABLED", false);
  datalayer.battery.status.led_mode = (led_mode_enum)settings.getUInt("LEDMODE", false);

  // WIFI AP is enabled by default unless disabled in the settings
  wifiap_enabled = settings.getBool("WIFIAPENABLED", true);
  wifi_channel = settings.getUInt("WIFICHANNEL", 0);
  ssidAP = settings.getString("APNAME", "BatteryEmulator").c_str();
  passwordAP = settings.getString("APPASSWORD", "123456789").c_str();
  custom_hostname = settings.getString("HOSTNAME").c_str();

  static_IP_enabled = settings.getBool("STATICIP", false);
  static_local_IP1 = settings.getUInt("LOCALIP1", 192);
  static_local_IP2 = settings.getUInt("LOCALIP2", 168);
  static_local_IP3 = settings.getUInt("LOCALIP3", 10);
  static_local_IP4 = settings.getUInt("LOCALIP4", 150);
  static_gateway1 = settings.getUInt("GATEWAY1", 192);
  static_gateway2 = settings.getUInt("GATEWAY2", 168);
  static_gateway3 = settings.getUInt("GATEWAY3", 10);
  static_gateway4 = settings.getUInt("GATEWAY4", 1);
  static_subnet1 = settings.getUInt("SUBNET1", 255);
  static_subnet2 = settings.getUInt("SUBNET2", 255);
  static_subnet3 = settings.getUInt("SUBNET3", 255);
  static_subnet4 = settings.getUInt("SUBNET4", 0);

  settings.end();
}

void store_settings_equipment_stop() {
  settings.begin("batterySettings", false);
  settings.putBool("EQUIPMENT_STOP", datalayer.system.settings.equipment_stop_active);
  settings.end();
}

void store_settings() {
  //  ATTENTION ! The maximum length for settings keys is 15 characters
  if (!settings.begin("batterySettings", false)) {
    set_event(EVENT_PERSISTENT_SAVE_INFO, 0);
    return;
  }

  if (!settings.putString("SSID", String(ssid.c_str()))) {
    if (ssid != "")
      set_event(EVENT_PERSISTENT_SAVE_INFO, 1);
  }
  if (!settings.putString("PASSWORD", String(password.c_str()))) {
    if (password != "")
      set_event(EVENT_PERSISTENT_SAVE_INFO, 2);
  }

  settings.end();  // Close preferences handle
}
