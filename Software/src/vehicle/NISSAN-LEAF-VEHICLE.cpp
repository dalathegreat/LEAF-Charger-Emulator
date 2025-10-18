#include "NISSAN-LEAF-VEHICLE.h"
#include <cstring>  //For unit test
#include "../charger/CHARGERS.h"
#include "../charger/CanCharger.h"
#include "../communication/can/comm_can.h"
#include "../datalayer/datalayer.h"
#include "../datalayer/datalayer_extended.h"  //For "More battery info" webpage
#include "../devboard/utils/events.h"
#include "../devboard/utils/logging.h"

void NissanLeafBattery::update_values() {  //Called every 10ms
  // We need to send these three CAN messages toward the vehicle:
  // 0x380 100ms
  // 0x5BF 100ms
  // 0x679 event driven when plug inserted
}

void NissanLeafBattery::transmit_can(unsigned long currentMillis) {
  //Charger has been plugged in and we need to wake vehicle
  if (datalayer.charger.plug_detected) {
    transmit_can_frame(&CHARGER_679);
    //TODO: Make sure to only send this once!
    datalayer.charger.plug_detected = false;  //This most likely wont work
  }

  //For now, we send periodic messages in the incoming queue, that way we dont need to keep track of when vehicle is turned on!
  // 0x380 100ms is sent inside the 50B message which arrives at 100ms

  //Send 100ms message FOR TESTING WITHOUT VEHICLE
  if (currentMillis - previousMillis100 >= INTERVAL_100_MS) {
    previousMillis100 = currentMillis;
  }
}

void NissanLeafBattery::handle_incoming_can_frame(CAN_frame rx_frame) {
  switch (rx_frame.ID) {
    case 0x11A:  //10ms Shifter module
      break;
    case 0x1D4:  //10ms VCU
      break;
    case 0x1DA:  //10ms Inverter
      break;
    case 0x1DB:  //10ms Battery
      if (is_message_corrupt(rx_frame)) {
        datalayer_battery->status.CAN_error_counter++;
        break;  //Message content malformed, abort reading data from it
      }
      battery_Current2 = (rx_frame.data.u8[0] << 3) | (rx_frame.data.u8[1] & 0xe0) >> 5;
      if (battery_Current2 & 0x0400) {
        // negative so extend the sign bit
        battery_Current2 |= 0xf800;
      }  //BatteryCurrentSignal , 2s comp, 1lSB = 0.5A/bit

      battery_TEMP = ((rx_frame.data.u8[2] << 2) | (rx_frame.data.u8[3] & 0xc0) >> 6);  //0.5V/bit
      if (battery_TEMP != 0x3ff) {  //3FF is unavailable value. Can happen directly on reboot.
        battery_Total_Voltage2 = battery_TEMP;
      }

      //Collect various data from the BMS
      battery_Relay_Cut_Request = ((rx_frame.data.u8[1] & 0x18) >> 3);
      battery_Failsafe_Status = (rx_frame.data.u8[1] & 0x07);
      battery_MainRelayOn_flag = (bool)((rx_frame.data.u8[3] & 0x20) >> 5);
      battery_Full_CHARGE_flag = (bool)((rx_frame.data.u8[3] & 0x10) >> 4);
      battery_Interlock = (bool)((rx_frame.data.u8[3] & 0x08) >> 3);
      break;
    case 0x1DC:
      if (is_message_corrupt(rx_frame)) {
        datalayer_battery->status.CAN_error_counter++;
        break;  //Message content malformed, abort reading data from it
      }
      battery_Discharge_Power_Limit = ((rx_frame.data.u8[0] << 2 | rx_frame.data.u8[1] >> 6) / 4.0);
      battery_Charge_Power_Limit = (((rx_frame.data.u8[1] & 0x3F) << 4 | rx_frame.data.u8[2] >> 4) / 4.0);
      battery_MAX_POWER_FOR_CHARGER = ((((rx_frame.data.u8[2] & 0x0F) << 6 | rx_frame.data.u8[3] >> 2) / 10.0) - 10);
      break;
    case 0x1F2:  //10ms Inverter
      break;
    case 0x284:  //20ms Inverter
      break;
    case 0x50A:
      break;
    case 0x50B:                       //100ms VCU (This message used to transmit 100ms messages)
      mprun100 = (mprun100 + 1) % 4;  // mprun100 cycles between 0-1-2-3-0-1...
      //If charger idle
      CHARGER_380.data.u8[0] = 0x02;
      CHARGER_380.data.u8[1] = 0x04;
      CHARGER_380.data.u8[2] = 0x10;
      CHARGER_380.data.u8[3] = 0x21;
      //If active charging (TODO)

      CHARGER_380.data.u8[7] = (mprun100 << 4);
      checksum = calculate_csum_380(CHARGER_380);
      CHARGER_380.data.u8[7] = ((mprun100 << 4) | (checksum & 0x0F));

      transmit_can_frame(&CHARGER_380);

      //If charger idle
      CHARGER_5BF.data.u8[0] = 0x00;
      CHARGER_5BF.data.u8[1] = 0x00;
      CHARGER_5BF.data.u8[2] = 0x00;  //J1772 maximum available current, 0 if we're not plugged in
      CHARGER_5BF.data.u8[3] = 0x00;
      CHARGER_5BF.data.u8[4] = CHARGER_STATUS_IDLE;
      CHARGER_5BF.data.u8[5] = 0x00;
      CHARGER_5BF.data.u8[6] = 0x00;
      //If active charging (TODO)

      CHARGER_5BF.data.u8[7] = (mprun100 << 4);
      checksum = calculate_csum_5BF(CHARGER_5BF);
      CHARGER_5BF.data.u8[7] = ((mprun100 << 4) | (checksum & 0x0F));
      transmit_can_frame(&CHARGER_5BF);
      break;
    case 0x50C:
      break;
    case 0x54A:
      break;
    case 0x54B:
      break;
    case 0x54C:
      break;
    case 0x54F:
      break;
    case 0x55A:  //Inverter 100ms
      break;
    case 0x55B:
      if (is_message_corrupt(rx_frame)) {
        datalayer_battery->status.CAN_error_counter++;
        break;  //Message content malformed, abort reading data from it
      }
      battery_TEMP = (rx_frame.data.u8[0] << 2 | rx_frame.data.u8[1] >> 6);
      if (battery_TEMP != 0x3ff) {  //3FF is unavailable value
        battery_SOC = battery_TEMP;
      }
      battery_Capacity_Empty = (bool)((rx_frame.data.u8[6] & 0x80) >> 7);
      break;
    case 0x56E:  //TCU 100ms
      break;
    case 0x5A9:  //VCU 500ms
      break;
    case 0x5B9:  //VCU 500ms
      break;
    case 0x5BC:
      battery_MAX = ((rx_frame.data.u8[5] & 0x10) >> 4);
      battery_AverageTemperature = (rx_frame.data.u8[3] - 40);  //In celcius, -40 to +55

      battery_TEMP = (rx_frame.data.u8[4] >> 1);
      if (battery_TEMP != 0) {
        battery_StateOfHealth = (uint8_t)battery_TEMP;  //Collect state of health from battery
      }
      break;
    case 0x5C0:  //LBCMSG5
      battery_HeatExist = (rx_frame.data.u8[4] & 0x01);
      battery_Heating_Stop = ((rx_frame.data.u8[0] & 0x10) >> 4);
      battery_Heating_Start = ((rx_frame.data.u8[0] & 0x20) >> 5);
      battery_Batt_Heater_Mail_Send_Request = (rx_frame.data.u8[1] & 0x01);
      break;
    case 0x68C:  //Bootup message VCM
    case 0x603:  //Bootup message VCM
      break;
    case 0x79B:
      break;
    case 0x7BB:
      break;
    default:
      break;
  }
}

uint8_t NissanLeafBattery::calculate_csum_380(CAN_frame& rx_frame) {
  uint8_t csum = 0;
  // Sum all nibbles from byte 0 to byte 6
  for (uint8_t j = 0; j < 7; j++) {
    csum += (rx_frame.data.u8[j] & 0x0F);
    csum += ((rx_frame.data.u8[j] & 0xF0) >> 4);
  }
  //Finally add the mprun from byte 7
  csum += ((rx_frame.data.u8[7] & 0xF0) >> 4);
  //380 message has -5 on sum
  csum = (csum - 5);
  //Result is finanlly ANDed with 0x0F since we only return one nibble
  csum = (csum & 0x0F);
  return csum;
}

uint8_t NissanLeafBattery::calculate_csum_5BF(CAN_frame& rx_frame) {
  uint8_t csum = 0;
  // Sum all nibbles from byte 0 to byte 6
  for (uint8_t j = 0; j < 7; j++) {
    csum += (rx_frame.data.u8[j] & 0x0F);
    csum += ((rx_frame.data.u8[j] & 0xF0) >> 4);
  }
  //Finally add the mprun from byte 7
  csum += ((rx_frame.data.u8[7] & 0xF0) >> 4);
  //5BF message has -1 on sum
  csum = (csum - 1);
  //Result is finanlly ANDed with 0x0F since we only return one nibble
  csum = (csum & 0x0F);
  return csum;
}

uint8_t NissanLeafBattery::calculate_crc(CAN_frame& rx_frame) {
  uint8_t crc = 0;
  for (uint8_t j = 0; j < 7; j++) {
    crc = crctable[(crc ^ static_cast<uint8_t>(rx_frame.data.u8[j])) % 256];
  }
  return crc;
}

bool NissanLeafBattery::is_message_corrupt(CAN_frame rx_frame) {
  uint8_t crc = calculate_crc(rx_frame);
  return crc != rx_frame.data.u8[7];
}

void NissanLeafBattery::setup(void) {  // Performs one time setup at startup
  strncpy(datalayer.system.info.battery_protocol, Name, 63);
  datalayer.system.info.battery_protocol[63] = '\0';
  datalayer_battery->info.number_of_cells = 96;
}
