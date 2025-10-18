#include "INVERTERS.h"

InverterProtocol* inverter = nullptr;

InverterProtocolType user_selected_inverter_protocol = InverterProtocolType::None;

std::vector<InverterProtocolType> supported_inverter_protocols() {
  std::vector<InverterProtocolType> types;

  for (int i = 0; i < (int)InverterProtocolType::Highest; i++) {
    types.push_back((InverterProtocolType)i);
  }

  return types;
}

extern const char* name_for_inverter_type(InverterProtocolType type) {
  switch (type) {
    case InverterProtocolType::None:
      return "None";

    case InverterProtocolType::AforeCan:
      return AforeCanInverter::Name;

    case InverterProtocolType::Highest:
      return "None";
  }
  return nullptr;
}

bool setup_inverter() {
  if (inverter) {
    return true;
  }

  switch (user_selected_inverter_protocol) {
    case InverterProtocolType::AforeCan:
      inverter = new AforeCanInverter();
      break;

    case InverterProtocolType::None:
      return true;
    case InverterProtocolType::Highest:
    default:
      inverter = nullptr;  // Or handle as error
      break;
  }

  if (inverter) {
    return inverter->setup();
  }

  return false;
}
