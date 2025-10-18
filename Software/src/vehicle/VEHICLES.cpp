#include "VEHICLES.h"
#include <soc/gpio_num.h>
#include <chrono>
#include "../datalayer/datalayer_extended.h"
#include "../devboard/hal/hal.h"
#include "CanVehicle.h"

Battery* battery = nullptr;

std::vector<BatteryType> supported_battery_types() {
  std::vector<BatteryType> types;

  for (int i = 0; i < (int)BatteryType::Highest; i++) {
    types.push_back((BatteryType)i);
  }

  return types;
}

const char* name_for_chemistry(battery_chemistry_enum chem) {
  switch (chem) {
    case battery_chemistry_enum::Autodetect:
      return "Autodetect";
    case battery_chemistry_enum::LFP:
      return "LFP";
    case battery_chemistry_enum::NCA:
      return "NCA";
    case battery_chemistry_enum::NMC:
      return "NMC";
    default:
      return nullptr;
  }
}

const char* name_for_comm_interface(comm_interface comm) {
  return esp32hal->name_for_comm_interface(comm);
}

const char* name_for_battery_type(BatteryType type) {
  switch (type) {
    case BatteryType::None:
      return "None";
    case BatteryType::NissanLeaf:
      return NissanLeafBattery::Name;
    default:
      return nullptr;
  }
}

const battery_chemistry_enum battery_chemistry_default = battery_chemistry_enum::NMC;

battery_chemistry_enum user_selected_battery_chemistry = battery_chemistry_default;

BatteryType user_selected_battery_type = BatteryType::NissanLeaf;
bool user_selected_second_battery = false;

Battery* create_battery(BatteryType type) {
  switch (type) {
    case BatteryType::None:
      return nullptr;
    case BatteryType::NissanLeaf:
      return new NissanLeafBattery();
    default:
      return nullptr;
  }
}

void setup_battery() {
  if (battery) {
    // Let's not create the battery again.
    return;
  }

  battery = create_battery(user_selected_battery_type);

  if (battery) {
    battery->setup();
  }
}
