#ifndef INVERTERS_H
#define INVERTERS_H

#include "InverterProtocol.h"
extern InverterProtocol* inverter;

#include "AFORE-CAN.h"

// Call to initialize the build-time selected inverter. Safe to call even though inverter was not selected.
bool setup_inverter();

#endif
