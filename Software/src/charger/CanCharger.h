#ifndef CAN_CHARGER_H
#define CAN_CHARGER_H

#include "../communication/Transmitter.h"
#include "../communication/can/CanReceiver.h"
#include "../communication/can/comm_can.h"
#include "../datalayer/datalayer.h"
#include "../devboard/utils/types.h"

enum class ChargerType { None, NissanLeaf, ChevyVolt, Highest };

extern ChargerType user_selected_charger_type;

extern std::vector<ChargerType> supported_charger_types();
extern const char* name_for_charger_type(ChargerType type);

// Generic base class for all chargers
class Charger {
 public:
  ChargerType type() { return m_type; }

  virtual const char* name() = 0;

  virtual float outputPowerDC() = 0;

  virtual float HVDC_output_voltage() { return datalayer.charger.charger_stat_HVvol; }
  virtual float HVDC_output_current() { return datalayer.charger.charger_stat_HVcur; }

  virtual float LVDC_output_voltage() { return datalayer.charger.charger_stat_LVvol; }
  virtual float LVDC_output_current() { return datalayer.charger.charger_stat_LVcur; }

  virtual float AC_input_voltage() { return datalayer.charger.charger_stat_ACvol; }
  virtual float AC_input_current() { return datalayer.charger.charger_stat_ACcur; }

  virtual bool efficiencySupported() { return false; }
  virtual float efficiency() { return 0; }

 protected:
  Charger(ChargerType type) : m_type(type) {}

 private:
  ChargerType m_type;
};

// Base class for chargers on a CAN bus
class CanCharger : public Charger, Transmitter, CanReceiver {
 public:
  virtual void map_can_frame_to_variable(CAN_frame rx_frame) = 0;
  virtual void transmit_can(unsigned long currentMillis) = 0;

  void transmit(unsigned long currentMillis) { transmit_can(currentMillis); }

  void receive_can_frame(CAN_frame* frame) { map_can_frame_to_variable(*frame); }

  CAN_Interface interface() { return can_interface; }

 protected:
  CAN_Interface can_interface;

  CanCharger(ChargerType type) : Charger(type) {
    can_interface = can_config.charger;
    register_transmitter(this);
    register_can_receiver(this, can_interface);
  }

  void transmit_can_frame(CAN_frame* frame) { transmit_can_frame_to_interface(frame, can_interface); }
};

#endif
