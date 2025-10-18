#include "../datalayer/datalayer.h"
#include "Vehicle.h"

float Battery::get_voltage() {
  return static_cast<float>(datalayer.battery.status.voltage_dV) / 10.0;
}
