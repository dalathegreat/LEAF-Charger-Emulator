#ifndef BATTERIES_H
#define BATTERIES_H

class Battery;

// Currently initialized objects for primary and secondary battery.
// Null value indicates that battery is not configured/initialized
extern Battery* battery;
#include "NISSAN-LEAF-VEHICLE.h"

void setup_battery(void);
Battery* create_battery(BatteryType type);

#endif
