#include "settings_html.h"
#include <Arduino.h>
#include "../../charger/CHARGERS.h"
#include "../../communication/can/comm_can.h"
#include "../../communication/nvm/comm_nvm.h"
#include "../../datalayer/datalayer.h"
#include "index_html.h"

const char* name_for_comm_interface(comm_interface comm) {
  switch (comm) {
    case comm_interface::Modbus:
      return "Modbus";
    case comm_interface::RS485:
      return "RS485";
    case comm_interface::CanNative:
      return "Native CAN";
    case comm_interface::CanFdNative:
      return "Native CAN FD";
    case comm_interface::CanAddonMcp2515:
      return "CAN MCP 2515 add-on";
    case comm_interface::CanFdAddonMcp2518:
      return "CAN FD MCP 2518 add-on";
    default:
      return nullptr;
  }
}

extern bool settingsUpdated;

template <typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

template <typename EnumType>
std::vector<EnumType> enum_values() {
  static_assert(std::is_enum_v<EnumType>, "Template argument must be an enum type.");

  constexpr auto count = to_underlying(EnumType::Highest);
  std::vector<EnumType> values;
  for (int i = 1; i < count; ++i) {
    values.push_back(static_cast<EnumType>(i));
  }
  return values;
}

template <typename EnumType, typename Func>
std::vector<std::pair<String, EnumType>> enum_values_and_names(Func name_for_type,
                                                               const EnumType* noneValue = nullptr) {
  auto values = enum_values<EnumType>();

  std::vector<std::pair<String, EnumType>> pairs;

  for (auto& type : values) {
    auto name = name_for_type(type);
    if (name != nullptr) {
      pairs.push_back(std::pair(String(name), type));
    }
  }

  std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  if (noneValue) {
    pairs.insert(pairs.begin(), std::pair(name_for_type(*noneValue), *noneValue));
  }

  return pairs;
}

template <typename TEnum, typename Func>
String options_for_enum_with_none(TEnum selected, Func name_for_type, TEnum noneValue) {
  String options;
  TEnum none = noneValue;
  auto values = enum_values_and_names<TEnum>(name_for_type, &none);
  for (const auto& [name, type] : values) {
    options +=
        ("<option value=\"" + String(static_cast<int>(type)) + "\"" + (selected == type ? " selected" : "") + ">");
    options += name;
    options += "</option>";
  }
  return options;
}

template <typename TEnum, typename Func>
String options_for_enum(TEnum selected, Func name_for_type) {
  String options;
  auto values = enum_values_and_names<TEnum>(name_for_type, nullptr);
  for (const auto& [name, type] : values) {
    options +=
        ("<option value=\"" + String(static_cast<int>(type)) + "\"" + (selected == type ? " selected" : "") + ">");
    options += name;
    options += "</option>";
  }
  return options;
}

template <typename TMap>
String options_from_map(int selected, const TMap& value_name_map) {
  String options;
  for (const auto& [value, name] : value_name_map) {
    options += "<option value=\"" + String(value) + "\"";
    if (selected == value) {
      options += " selected";
    }
    options += ">";
    options += name;
    options += "</option>";
  }
  return options;
}

String settings_processor(const String& var, BatteryEmulatorSettingsStore& settings) {

  if (var == "HOSTNAME") {
    return settings.getString("HOSTNAME");
  }

  if (var == "SSID") {
    return String(ssid.c_str());
  }

  if (var == "SAVEDCLASS") {
    if (!settingsUpdated) {
      return "hidden";
    }
  }

  if (var == "CHARGERCLASS") {
    if (!charger) {
      return "hidden";
    }
  }
  if (var == "INVCOMM") {
    return options_for_enum((comm_interface)settings.getUInt("INVCOMM", (int)comm_interface::CanNative),
                            name_for_comm_interface);
  }
  if (var == "CHGTYPE") {
    return options_for_enum_with_none((ChargerType)settings.getUInt("CHGTYPE", (int)ChargerType::None),
                                      name_for_charger_type, ChargerType::None);
  }
  if (var == "CHGCOMM") {
    return options_for_enum((comm_interface)settings.getUInt("CHGCOMM", (int)comm_interface::CanNative),
                            name_for_comm_interface);
  }

  if (var == "CANFDASCAN") {
    return settings.getBool("CANFDASCAN") ? "checked" : "";
  }

  if (var == "WIFIAPENABLED") {
    return settings.getBool("WIFIAPENABLED", wifiap_enabled) ? "checked" : "";
  }

  if (var == "APPASSWORD") {
    return settings.getString("APPASSWORD", "123456789");
  }

  if (var == "APNAME") {
    return settings.getString("APNAME", "BatteryEmulator");
  }

  if (var == "STATICIP") {
    return settings.getBool("STATICIP") ? "checked" : "";
  }

  if (var == "WIFICHANNEL") {
    return String(settings.getUInt("WIFICHANNEL", 0));
  }

  if (var == "LOCALIP1") {
    return String(settings.getUInt("LOCALIP1", 0));
  }

  if (var == "LOCALIP2") {
    return String(settings.getUInt("LOCALIP2", 0));
  }

  if (var == "LOCALIP3") {
    return String(settings.getUInt("LOCALIP3", 0));
  }

  if (var == "LOCALIP4") {
    return String(settings.getUInt("LOCALIP4", 0));
  }

  if (var == "GATEWAY1") {
    return String(settings.getUInt("GATEWAY1", 0));
  }

  if (var == "GATEWAY2") {
    return String(settings.getUInt("GATEWAY2", 0));
  }

  if (var == "GATEWAY3") {
    return String(settings.getUInt("GATEWAY3", 0));
  }

  if (var == "GATEWAY4") {
    return String(settings.getUInt("GATEWAY4", 0));
  }

  if (var == "SUBNET1") {
    return String(settings.getUInt("SUBNET1", 0));
  }

  if (var == "SUBNET2") {
    return String(settings.getUInt("SUBNET2", 0));
  }

  if (var == "SUBNET3") {
    return String(settings.getUInt("SUBNET3", 0));
  }

  if (var == "SUBNET4") {
    return String(settings.getUInt("SUBNET4", 0));
  }

  if (var == "WEBENABLED") {
    return settings.getBool("WEBENABLED") ? "checked" : "";
  }

  if (var == "CHARGER_CLASS") {
    if (!charger) {
      return "hidden";
    }
  }

  if (var == "CHG_HV_CLASS") {
    if (datalayer.charger.charger_HV_enabled) {
      return "active";
    } else {
      return "inactiveSoc";
    }
  }

  if (var == "CHG_HV") {
    if (datalayer.charger.charger_HV_enabled) {
      return "&#10003;";
    } else {
      return "&#10005;";
    }
  }

  if (var == "CHG_AUX12V_CLASS") {
    if (datalayer.charger.charger_aux12V_enabled) {
      return "active";
    } else {
      return "inactiveSoc";
    }
  }

  if (var == "CHG_AUX12V") {
    if (datalayer.charger.charger_aux12V_enabled) {
      return "&#10003;";
    } else {
      return "&#10005;";
    }
  }

  if (var == "CHG_VOLTAGE_SETPOINT") {
    return String(datalayer.charger.charger_setpoint_HV_VDC, 1);
  }

  if (var == "CHG_CURRENT_SETPOINT") {
    return String(datalayer.charger.charger_setpoint_HV_IDC, 1);
  }

  if (var == "CANFREQ") {
    return String(settings.getUInt("CANFREQ", 8));
  }

  if (var == "CANFDFREQ") {
    return String(settings.getUInt("CANFDFREQ", 40));
  }

  return String();
}

const char* getCANInterfaceName(CAN_Interface interface) {
  switch (interface) {
    case CAN_NATIVE:
      return "CAN";
    case CANFD_NATIVE:
      if (use_canfd_as_can) {
        return "CAN-FD Native (Classic CAN)";
      } else {
        return "CAN-FD Native";
      }
    case CAN_ADDON_MCP2515:
      return "Add-on CAN via GPIO MCP2515";
    case CANFD_ADDON_MCP2518:
      if (use_canfd_as_can) {
        return "Add-on CAN-FD via GPIO MCP2518 (Classic CAN)";
      } else {
        return "Add-on CAN-FD via GPIO MCP2518";
      }
    default:
      return "UNKNOWN";
  }
}

#define SETTINGS_HTML_SCRIPTS \
  R"rawliteral(
    <script>

    function askFactoryReset() {
      if (confirm('Are you sure you want to reset the device to factory settings? This will erase all settings and data.')) {
        var xhr = new XMLHttpRequest();
        xhr.onload = function() {
          if (this.status == 200) {
            alert('Factory reset successful. The device will now restart.');
            reboot();
          } else {
            alert('Factory reset failed. Please try again.');
          }
        };
        xhr.onerror = function() {
          alert('An error occurred while trying to reset the device.');
        };
        xhr.open('POST', '/factoryReset', true);
        xhr.send();
      }
    }

    function editComplete(){if(this.status==200){window.location.reload();}}

    function editError(){alert('Invalid input');}

        function editSSID(){var value=prompt('Enter new SSID:');if(value!==null){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateSSID?value='+encodeURIComponent(value),true);xhr.send();}}
        
        function editPassword(){var value=prompt('Enter new password:');if(value!==null){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updatePassword?value='+encodeURIComponent(value),true);xhr.send();}}

        function editWh(){var value=prompt('How much energy the battery can store. Enter new Wh value (1-400000):');
          if(value!==null){if(value>=1&&value<=400000){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateBatterySize?value='+value,true);xhr.send();}else{
          alert('Invalid value. Please enter a value between 1 and 400000.');}}}

        function editUseScaledSOC(){var value=prompt('Extends battery life by rescaling the SOC within the configured minimum and maximum percentage. Should SOC scaling be applied? (0 = No, 1 = Yes):');
          if(value!==null){if(value==0||value==1){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateUseScaledSOC?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1.');}}}
    
        function editSocMax(){var value=prompt('Inverter will see fully charged (100pct)SOC when this value is reached. Enter new maximum SOC value that battery will charge to (50.0-100.0):');if(value!==null){if(value>=50&&value<=100){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateSocMax?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 50.0 and 100.0');}}}
    
        function editSocMin(){
          var value=prompt('Inverter will see completely discharged (0pct)SOC when this value is reached. Advanced users can set to negative values. Enter new minimum SOC value that battery will discharge to (-10.0to50.0):');
          if(value!==null){if(value>=-10&&value<=50){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateSocMin?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between -10 and 50.0');}}}
    
        function editMaxChargeA(){var value=prompt('Some inverters needs to be artificially limited. Enter new maximum charge current in A (0-1000.0):');if(value!==null){if(value>=0&&value<=1000){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateMaxChargeA?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1000.0');}}}
    
        function editMaxDischargeA(){var value=prompt('Some inverters needs to be artificially limited. Enter new maximum discharge current in A (0-1000.0):');if(value!==null){if(value>=0&&value<=1000){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateMaxDischargeA?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1000.0');}}}
    
        function editUseVoltageLimit(){var value=prompt('Enable this option to manually restrict charge/discharge to a specific voltage set below. If disabled the emulator automatically determines this based on battery limits. Restrict manually? (0 = No, 1 = Yes):');if(value!==null){if(value==0||value==1){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateUseVoltageLimit?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1.');}}}
    
        function editMaxChargeVoltage(){var value=prompt('Some inverters needs to be artificially limited. Enter new voltage setpoint batttery should charge to (0-1000.0):');if(value!==null){if(value>=0&&value<=1000){var 
        xhr=new XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateMaxChargeVoltage?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1000.0');}}}
    
        function editMaxDischargeVoltage(){var value=prompt('Some inverters needs to be artificially limited. Enter new voltage setpoint batttery should discharge to (0-1000.0):');if(value!==null){if(value>=0&&value<=1000){var 
        xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateMaxDischargeVoltage?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1000.0');}}}

        function editBMSresetDuration(){var value=prompt('Amount of seconds BMS power should be off during periodic daily resets. Requires "Periodic BMS reset" to be enabled. Enter value in seconds (1-59):');if(value!==null){if(value>=1&&value<=59){var 
        xhr=new XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateBMSresetDuration?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 1 and 59');}}}

        function editTeslaBalAct(){var value=prompt('Enable or disable forced LFP balancing. Makes the battery charge to 101percent. This should be performed once every month, to keep LFP batteries balanced. Ensure battery is fully charged before enabling, and also that you have enough sun or grid power to feed power into the battery while balancing is active. Enter 1 for enabled, 0 for disabled');if(value!==null){if(value==0||value==1){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/TeslaBalAct?value='+value,true);xhr.send();}}else{alert('Invalid value. Please enter 1 or 0');}}
    
        function editBalTime(){var value=prompt('Enter new max balancing time in minutes');if(value!==null){if(value>=1&&value<=300){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/BalTime?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 1 and 300');}}}
    
        function editBalFloatPower(){var value=prompt('Power level in Watt to float charge during forced balancing');if(value!==null){if(value>=100&&value<=2000){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/BalFloatPower?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 100 and 2000');}}}
    
        function editBalMaxPackV(){var value=prompt('Battery pack max voltage temporarily raised to this value during forced balancing. Value in V');if(value!==null){if(value>=380&&value<=410){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/BalMaxPackV?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 380 and 410');}}}

        function editBalMaxCellV(){var value=prompt('Cellvoltage max temporarily raised to this value during forced balancing. Value in mV');if(value!==null){if(value>=3400&&value<=3750){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/BalMaxCellV?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 3400 and 3750');}}}
    
        function editBalMaxDevCellV(){var value=prompt('Cellvoltage max deviation temporarily raised to this value during forced balancing. Value in mV');if(value!==null){if(value>=300&&value<=600){var xhr=new 
        XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/BalMaxDevCellV?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 300 and 600');}}}

          function editFakeBatteryVoltage(){var value=prompt('Enter new fake battery voltage');if(value!==null){if(value>=0&&value<=5000){var xhr=new 
          XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateFakeBatteryVoltage?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 1000');}}}

          function editChargerHVDCEnabled(){var value=prompt('Enable or disable HV DC output. Enter 1 for enabled, 0 for disabled');if(value!==null){if(value==0||value==1){var xhr=new 
          XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateChargerHvEnabled?value='+value,true);xhr.send();}}else{alert('Invalid value. Please enter 1 or 0');}}

          function editChargerAux12vEnabled(){var value=prompt('Enable or disable low voltage 12v auxiliary DC output. Enter 1 for enabled, 0 for disabled');if(value!==null){if(value==0||value==1){var xhr=new 
          XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateChargerAux12vEnabled?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter 1 or 0');}}}

          function editChargerSetpointVDC(){var value=prompt('Set charging voltage. Input will be validated against inverter and/or charger configuration parameters, but use sensible values like 200 to 420.');
            if(value!==null){if(value>=0&&value<=1000){var xhr=new XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateChargeSetpointV?value='+value,true);xhr.send();}else{
            alert('Invalid value. Please enter a value between 0 and 1000');}}}

          function editChargerSetpointIDC(){var value=prompt('Set charging amperage. Input will be validated against inverter and/or charger configuration parameters, but use sensible values like 6 to 48.');
            if(value!==null){if(value>=0&&value<=1000){var xhr=new           XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateChargeSetpointA?value='+value,true);xhr.send();}else{
              alert('Invalid value. Please enter a value between 0 and 100');}}}

          function editChargerSetpointEndI(){
            var value=prompt('Set amperage that terminates charge as being sufficiently complete. Input will be validated against inverter and/or charger configuration parameters, but use sensible values like 1-5.');
            if(value!==null){if(value>=0&&value<=1000){var xhr=new 
          XMLHttpRequest();xhr.onload=editComplete;xhr.onerror=editError;xhr.open('GET','/updateChargeEndA?value='+value,true);xhr.send();}else{alert('Invalid value. Please enter a value between 0 and 100');}}}

          function goToMainPage() { window.location.href = '/'; }

          document.querySelectorAll('select,input').forEach(function(sel) {
            function ch() {
              sel.closest('form').setAttribute('data-' + sel.name?.toLowerCase(), sel.type=='checkbox'?sel.checked:sel.value);
            }
            sel.addEventListener('change', ch);
            ch();
          });
    </script>
)rawliteral"

#define SETTINGS_STYLE \
  R"rawliteral(
    <style>
    body { background-color: black; color: white; }
        button { background-color: #505E67; color: white; border: none; padding: 10px 20px; margin-bottom: 20px;
        cursor: pointer; border-radius: 10px; }
    button:hover { background-color: #3A4A52; }
    h4 { margin: 0.6em 0; line-height: 1.2; }
    select, input { max-width: 250px; box-sizing: border-box; }
    .hidden {
      display: none;
    }
    .active {
      color: white;
    }
    .inactive {
      color: darkgrey;
    }

    .inactiveSoc {
      color: red;
    }

    .mqtt-settings, .mqtt-topics {
      display: none;
      grid-column: span 2;
    }

    .settings-card {
    background-color: #3a4b54; /* Slightly lighter than main background */
    padding: 15px 20px;
    margin-bottom: 20px;
    border-radius: 20px; /* Less rounded than 50px for a more card-like feel */
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
  }
  .settings-card h3 {
    color: #fff;
    margin-top: 0;
    margin-bottom: 15px;
    padding-bottom: 8px;
    border-bottom: 1px solid #4d5f69;
  }

    form .if-battery, form .if-inverter, form .if-charger, form .if-shunt { display: contents; }
    form[data-battery="0"] .if-battery { display: none; }
    form[data-inverter="0"] .if-inverter { display: none; }    
    form[data-charger="0"] .if-charger { display: none; }
    
    form .if-staticip { display: none; }
    form[data-staticip="true"] .if-staticip {
      display: contents;
    }

    </style>
)rawliteral"

#define SETTINGS_HTML_BODY \
  R"rawliteral(
  <button onclick='goToMainPage()'>Back to main page</button>

<div style='background-color: #303E47; padding: 10px; margin-bottom: 10px; border-radius: 50px'>
    <h4 style='color: white;'>SSID: <span id='SSID'>%SSID%</span><button onclick='editSSID()'>Edit</button></h4>
    <h4 style='color: white;'>Password: ######## <span id='Password'></span> <button onclick='editPassword()'>Edit</button></h4>
</div>

<div style='background-color: #404E47; padding: 10px; margin-bottom: 10px; border-radius: 50px'>
        <form action='saveSettings' method='post'>

        <div style='grid-column: span 2; text-align: center; padding-top: 10px;' class="%SAVEDCLASS%">
          <p>Settings saved. Reboot to take the new settings into use.<p> <button type='button' onclick='askReboot()'>Reboot</button>
        </div>

        <div class="settings-card">
        <h3>Battery config</h3>
        <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label for='battery'>Battery: </label>
        <select name='battery' id='battery'>
            %BATTTYPE%
        </select>

        <div class="if-nissan">
            <label for='interlock'>Interlock required: </label>
            <input type='checkbox' name='INTERLOCKREQ' id='interlock' value='on' %INTERLOCKREQ% />
        </div>

        <div class="if-tesla">
          <label for='digitalhvil'>Digital HVIL (2024+): </label>
          <input type='checkbox' name='DIGITALHVIL' id='digitalhvil' value='on' %DIGITALHVIL% />
          <label>Right hand drive: </label>
          <input type='checkbox' name='GTWRHD' value='on' %GTWRHD% />
          <label for='GTWCOUNTRY'>Country code: </label><select name='GTWCOUNTRY' id='GTWCOUNTRY'>
          %GTWCOUNTRY%
          </select>
          <label for='GTWMAPREG'>Map region: </label><select name='GTWMAPREG' id='GTWMAPREG'>
          %GTWMAPREG%
          </select>
          <label for='GTWCHASSIS'>Chassis type: </label><select name='GTWCHASSIS' id='GTWCHASSIS'>
          %GTWCHASSIS%
          </select>
          <label for='GTWPACK'>Pack type: </label><select name='GTWPACK' id='GTWPACK'>
          %GTWPACK%
          </select>
        </div>

        <div class="if-estimated">
        <label>Manual charging power, watt: </label>
        <input name='CHGPOWER' pattern="^[0-9]+$" type='text' value='%CHGPOWER%' />

        <label>Manual discharge power, watt: </label>
        <input name='DCHGPOWER' pattern="^[0-9]+$" type='text' value='%DCHGPOWER%' />
        </div>

        <div class="if-socestimated">
        <label>Use estimated SOC: </label>
        <input type='checkbox' name='SOCESTIMATED' value='on' %SOCESTIMATED% />
        </div>

        <div class="if-battery">
        <label for='BATTCOMM'>Battery interface: </label><select name='BATTCOMM' id='BATTCOMM'>
        %BATTCOMM%
        </select>

        <label>Battery chemistry: </label><select name='BATTCHEM'>
        %BATTCHEM%
        </select>
        </div>

        <div class="if-cbms">
        <label>Battery max design voltage (V): </label>
        <input name='BATTPVMAX' pattern="^[0-9]+(\.[0-9]+)?$" type='text' value='%BATTPVMAX%' />

        <label>Battery min design voltage (V): </label>
        <input name='BATTPVMIN' pattern="^[0-9]+(\.[0-9]+)?$" type='text' value='%BATTPVMIN%' />

        <label>Cell max design voltage (mV): </label>
        <input name='BATTCVMAX' pattern="^[0-9]+$" type='text' value='%BATTCVMAX%' />

        <label>Cell min design voltage (mV): </label>
        <input name='BATTCVMIN' pattern="^[0-9]+$" type='text' value='%BATTCVMIN%' />
        </div>

        <label>Double battery: </label>
        <input type='checkbox' name='DBLBTR' value='on' %DBLBTR% />

        <div class="if-dblbtr">
            <label>Battery 2 interface: </label>
            <select name='BATT2COMM'>
                %BATT2COMM%
            </select>
        </div>

        </div>
        </div>

        <div class="settings-card">
      <h3>Inverter config</h3>
      <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label>Inverter protocol: </label><select name='inverter'>
        %INVTYPE%
        </select>

        <div class="if-inverter">        
        <label>Inverter interface: </label><select name='INVCOMM'>
        %INVCOMM%     
        </select>
        </div>

        <div class="if-sofar">
        <label>Sofar Battery ID (0-15): </label>
        <input name='SOFAR_ID' type='text' value="%SOFAR_ID%" pattern="^[0-9]{1,2}$" />
        </div>

        <div class="if-pylonish">
        <label>Reported cell count (0 for default): </label>
        <input name='INVCELLS' type='text' value="%INVCELLS%" pattern="^[0-9]+$" />
        </div>

        <div class="if-pylonish if-solax">
        <label>Reported module count (0 for default): </label>
        <input name='INVMODULES' type='text' value="%INVMODULES%" pattern="^[0-9]+$" />
        </div>

        <div class="if-pylonish">
        <label>Reported cells per module (0 for default): </label>
        <input name='INVCELLSPER' type='text' value="%INVCELLSPER%" pattern="^[0-9]+$" />

        <label>Reported voltage level (0 for default): </label>
        <input name='INVVLEVEL' type='text' value="%INVVLEVEL%" pattern="^[0-9]+$" />

        <label>Reported Ah capacity (0 for default): </label>
        <input name='INVCAPACITY' type='text' value="%INVCAPACITY%" pattern="^[0-9]+$" />
        </div>

        <div class="if-solax">
        <label>Reported battery type (in decimal): </label>
        <input name='INVBTYPE' type='text' value="%INVBTYPE%" pattern="^[0-9]+$" />

        <label>Inverter should ignore contactors: </label>
        <input type='checkbox' name='INVICNT' value='on' %INVICNT% />
        </div>

        </div>
        </div>

        <div class="settings-card">
        <h3>Optional components config</h3>
        <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label>Charger: </label><select name='charger'>
        %CHGTYPE%
        </select>

        <div class="if-charger">
        <label>Charger interface: </label><select name='CHGCOMM'>
        %CHGCOMM%
        </select>
        </div>

        <label>Shunt: </label><select name='SHUNT'>
        %SHUNTTYPE%
        </select>

        <div class="if-shunt">
        <label>Shunt interface: </label><select name='SHUNTCOMM'>
        %SHUNTCOMM%
        </select>
        </div>

        </div>
        </div>

        <div class="settings-card">
        <h3>Hardware config</h3>
        <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label>Use CanFD as classic CAN: </label>
        <input type='checkbox' name='CANFDASCAN' value='on' %CANFDASCAN% /> 

        <label>CAN addon crystal (Mhz): </label>
        <input name='CANFREQ' type='text' value="%CANFREQ%" pattern="^[0-9]+$" />

        <label>CAN-FD-addon crystal (Mhz): </label>
        <input name='CANFDFREQ' type='text' value="%CANFDFREQ%" pattern="^[0-9]+$" />
        
        <label>Equipment stop button: </label><select name='EQSTOP'>
        %EQSTOP%  
        </select>

        <div class="if-dblbtr">
            <label>Double-Battery Contactor control via GPIO: </label>
            <input type='checkbox' name='CNTCTRLDBL' value='on' %CNTCTRLDBL% />
        </div>

        <label>Contactor control via GPIO: </label>
        <input type='checkbox' name='CNTCTRL' value='on' %CNTCTRL% />

        <div class="if-cntctrl">
            <label>Precharge time ms: </label>
            <input name='PRECHGMS' type='text' value="%PRECHGMS%" pattern="^[0-9]+$" />

            <label>PWM contactor control: </label>
            <input type='checkbox' name='PWMCNTCTRL' value='on' %PWMCNTCTRL% />

             <div class="if-pwmcntctrl">
            <label>PWM Frequency Hz: </label>
            <input name='PWMFREQ' type='text' value="%PWMFREQ%" pattern="^[0-9]+$" />

            <label>PWM Hold 0-1023: </label>
            <input name='PWMHOLD' type='text' value="%PWMHOLD%" pattern="^[0-9]+$" />
              </div>

        </div>

        <label>Periodic BMS reset every 24h: </label>
        <input type='checkbox' name='PERBMSRESET' value='on' %PERBMSRESET% /> 

        <label>External precharge via HIA4V1: </label>
        <input type='checkbox' name='EXTPRECHARGE' value='on' %EXTPRECHARGE% />

        <div class="if-extprecharge">
            <label>Precharge, maximum ms before fault: </label>
            <input name='MAXPRETIME' type='text' value="%MAXPRETIME%" pattern="^[0-9]+$" />

          <label>Normally Open (NO) inverter disconnect contactor: </label>
          <input type='checkbox' name='NOINVDISC' value='on' %NOINVDISC% />
        </div>

        <label for='LEDMODE'>Status LED pattern: </label><select name='LEDMODE' id='LEDMODE'>
        %LEDMODE%
        </select>

        </div>
        </div>

        <div class="settings-card">
        <h3>Connectivity settings</h3>
        <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label>Broadcast Wifi access point: </label>
        <input type='checkbox' name='WIFIAPENABLED' value='on' %WIFIAPENABLED% />

        <label>Access point name: </label>
        <input type='text' name='APNAME' value="%APNAME%" />

        <label>Access point password: </label>
        <input type='text' name='APPASSWORD' value="%APPASSWORD%" />

        <label>Wifi channel 0-14: </label>
        <input name='WIFICHANNEL' type='text' value="%WIFICHANNEL%" pattern="^[0-9]+$" />

        <label>Custom Wifi hostname: </label>
        <input type='text' name='HOSTNAME' value="%HOSTNAME%" />

        <label>Use static IP address: </label>
        <input type='checkbox' name='STATICIP' value='on' %STATICIP% />

        <div class='if-staticip'>
        <div>
          <div>Local IP:</div>
          <input type="number" name="LOCALIP1" min="0" max="255" size="3" value="%LOCALIP1%">.
          <input type="number" name="LOCALIP2" min="0" max="255" size="3" value="%LOCALIP2%">.
          <input type="number" name="LOCALIP3" min="0" max="255" size="3" value="%LOCALIP3%">.
          <input type="number" name="LOCALIP4" min="0" max="255" size="3" value="%LOCALIP4%">
        </div>
            
        <div>
            <div>Gateway:</div>
            <input type="number" name="GATEWAY1" min="0" max="255" size="3" value="%GATEWAY1%">.
            <input type="number" name="GATEWAY2" min="0" max="255" size="3" value="%GATEWAY2%">.
            <input type="number" name="GATEWAY3" min="0" max="255" size="3" value="%GATEWAY3%">.
            <input type="number" name="GATEWAY4" min="0" max="255" size="3" value="%GATEWAY4%">
        </div>
    
        <div>
          <div>Subnet:</div>
          <input type="number" name="SUBNET1" min="0" max="255" size="3" value="%SUBNET1%">.
          <input type="number" name="SUBNET2" min="0" max="255" size="3" value="%SUBNET2%">.
          <input type="number" name="SUBNET3" min="0" max="255" size="3" value="%SUBNET3%">.
          <input type="number" name="SUBNET4" min="0" max="255" size="3" value="%SUBNET4%">
        </div>
        <div></div>
        </div>

        <label>Enable MQTT: </label>
        <input type='checkbox' name='MQTTENABLED' value='on' %MQTTENABLED% />

        <div class='if-mqtt'>
        <label>MQTT server: </label><input type='text' name='MQTTSERVER' value="%MQTTSERVER%" />
        <label>MQTT port: </label><input type='text' name='MQTTPORT' value="%MQTTPORT%" />
        <label>MQTT user: </label><input type='text' name='MQTTUSER' value="%MQTTUSER%" />
        <label>MQTT password: </label><input type='password' name='MQTTPASSWORD' value="%MQTTPASSWORD%" />
        <label>MQTT timeout ms: </label><input name='MQTTTIMEOUT' type='text' value="%MQTTTIMEOUT%" pattern="^[0-9]+$" />
        <label>Send all cellvoltages via MQTT: </label><input type='checkbox' name='MQTTCELLV' value='on' %MQTTCELLV% />
        <label>Remote BMS reset via MQTT allowed: </label>
        <input type='checkbox' name='REMBMSRESET' value='on' %REMBMSRESET% />
        <label>Customized MQTT topics: </label>
        <input type='checkbox' name='MQTTTOPICS' value='on' %MQTTTOPICS% />

        <div class='if-topics'>

        <label>MQTT topic name: </label><input type='text' name='MQTTTOPIC' value="%MQTTTOPIC%" />
        <label>Prefix for MQTT object ID: </label><input type='text' name='MQTTOBJIDPREFIX' value="%MQTTOBJIDPREFIX%" />
        <label>HA device name: </label><input type='text' name='MQTTDEVICENAME' value="%MQTTDEVICENAME%" />
        <label>HA device ID: </label><input type='text' name='HADEVICEID' value="%HADEVICEID%" />
        
        </div>

        <label>Enable Home Assistant auto discovery: </label>
        <input type='checkbox' name='HADISC' value='on' %HADISC% />

        </div>

        </div>
        </div>

        <div class="settings-card">
        <h3>Debug options</h3>
        <div style='display: grid; grid-template-columns: 1fr 1.5fr; gap: 10px; align-items: center;'>

        <label>Enable performance profiling on main page: </label>
        <input type='checkbox' name='PERFPROFILE' value='on' %PERFPROFILE% />

        <label>Enable CAN message logging via USB serial: </label>
        <input type='checkbox' name='CANLOGUSB' value='on' %CANLOGUSB% />

        <label>Enable general logging via USB serial: </label>
        <input type='checkbox' name='USBENABLED' value='on' %USBENABLED% />

        <label>Enable general logging via Webserver: </label>
        <input type='checkbox' name='WEBENABLED' value='on' %WEBENABLED% />

        <label>Enable CAN message logging via SD card: </label>
        <input type='checkbox' name='CANLOGSD' value='on' %CANLOGSD% />

        <label>Enable general logging via SD card: </label>
        <input type='checkbox' name='SDLOGENABLED' value='on' %SDLOGENABLED% />

        </div>
         </div>

        <div style='grid-column: span 2; text-align: center; padding-top: 10px;'><button type='submit'>Save</button></div>

        <div style='grid-column: span 2; text-align: center; padding-top: 10px;' class="%SAVEDCLASS%">
          <p>Settings saved. Reboot to take the new settings into use.<p> <button type='button' onclick='askReboot()'>Reboot</button>
        </div>

        </form>
    </div>
    </div>

      <h4 style='color: white;'>Battery interface: <span id='Battery'>%BATTERYINTF%</span></h4>

      <h4 style='color: white;' class="%BATTERY2CLASS%">Battery interface: <span id='Battery2'>%BATTERY2INTF%</span></h4>

      <h4 style='color: white;' class="%INVCLASS%">Inverter interface: <span id='Inverter'>%INVINTF%</span></h4>
      
      <h4 style='color: white;' class="%SHUNTCLASS%">Shunt interface: <span id='Inverter'>%SHUNTINTF%</span></h4>

    </div>

    <div style='background-color: #2D3F2F; padding: 10px; margin-bottom: 10px;border-radius: 50px'>

      <h4 style='color: white;'>Battery capacity: <span id='BATTERY_WH_MAX'>%BATTERY_WH_MAX% Wh </span> <button onclick='editWh()'>Edit</button></h4>

      <h4 style='color: white;'>Rescale SOC: <span id='BATTERY_USE_SCALED_SOC'><span class='%SOC_SCALING_CLASS%'>%SOC_SCALING%</span>
                </span> <button onclick='editUseScaledSOC()'>Edit</button></h4>

      <h4 class='%SOC_SCALING_ACTIVE_CLASS%'><span>SOC max percentage: %SOC_MAX_PERCENTAGE%</span> <button onclick='editSocMax()'>Edit</button></h4>

      <h4 class='%SOC_SCALING_ACTIVE_CLASS%'><span>SOC min percentage: %SOC_MIN_PERCENTAGE%</span> <button onclick='editSocMin()'>Edit</button></h4>
      
      <h4 style='color: white;'>Max charge speed: %MAX_CHARGE_SPEED% A </span> <button onclick='editMaxChargeA()'>Edit</button></h4>

      <h4 style='color: white;'>Max discharge speed: %MAX_DISCHARGE_SPEED% A </span><button onclick='editMaxDischargeA()'>Edit</button></h4>

      <h4 style='color: white;'>Manual charge voltage limits: <span id='BATTERY_USE_VOLTAGE_LIMITS'>
        <span class='%VOLTAGE_LIMITS_CLASS%'>%VOLTAGE_LIMITS%</span>
                </span> <button onclick='editUseVoltageLimit()'>Edit</button></h4>

      <h4 class='%VOLTAGE_LIMITS_ACTIVE_CLASS%'>Target charge voltage: %CHARGE_VOLTAGE% V </span> <button onclick='editMaxChargeVoltage()'>Edit</button></h4>

      <h4 class='%VOLTAGE_LIMITS_ACTIVE_CLASS%'>Target discharge voltage: %DISCHARGE_VOLTAGE% V </span> <button onclick='editMaxDischargeVoltage()'>Edit</button></h4>

      <h4 style='color: white;'>Periodic BMS reset off time: %BMS_RESET_DURATION% s </span><button onclick='editBMSresetDuration()'>Edit</button></h4>

    </div>

    <div style='background-color: #2E37AD; padding: 10px; margin-bottom: 10px;border-radius: 50px' class="%FAKE_VOLTAGE_CLASS%">
      <h4 style='color: white;'><span>Fake battery voltage: %BATTERY_VOLTAGE% V </span> <button onclick='editFakeBatteryVoltage()'>Edit</button></h4>
    </div>

    <!--if (battery && battery->supports_manual_balancing()) {-->
      
    <div style='background-color: #303E47; padding: 10px; margin-bottom: 10px;border-radius: 50px' class="%MANUAL_BAL_CLASS%">

          <h4 style='color: white;'>Manual LFP balancing: <span id='TSL_BAL_ACT'><span class="%MANUAL_BALANCING_CLASS%">%MANUAL_BALANCING%</span>
          </span> <button onclick='editTeslaBalAct()'>Edit</button></h4>

          <h4 class="%BALANCING_CLASS%"><span>Balancing max time: %BAL_MAX_TIME% Minutes</span> <button onclick='editBalTime()'>Edit</button></h4>

          <h4 class="%BALANCING_CLASS%"><span>Balancing float power: %BAL_POWER% W </span> <button onclick='editBalFloatPower()'>Edit</button></h4>

           <h4 class="%BALANCING_CLASS%"><span>Max battery voltage: %BAL_MAX_PACK_VOLTAGE% V</span> <button onclick='editBalMaxPackV()'>Edit</button></h4>

           <h4 class="%BALANCING_CLASS%"><span>Max cell voltage: %BAL_MAX_CELL_VOLTAGE% mV</span> <button onclick='editBalMaxCellV()'>Edit</button></h4>

          <h4 class="%BALANCING_CLASS%"><span>Max cell voltage deviation: %BAL_MAX_DEV_CELL_VOLTAGE% mV</span> <button onclick='editBalMaxDevCellV()'>Edit</button></h4>

    </div>

     <div style='background-color: #FF6E00; padding: 10px; margin-bottom: 10px;border-radius: 50px' class="%CHARGER_CLASS%">

      <h4 style='color: white;'>
        Charger HVDC Enabled: <span class="%CHG_HV_CLASS%">%CHG_HV%</span>
        <button onclick='editChargerHVDCEnabled()'>Edit</button>
      </h4>

      <h4 style='color: white;'>
        Charger Aux12VDC Enabled: <span class="%CHG_AUX12V_CLASS%">%CHG_AUX12V%</span>
        <button onclick='editChargerAux12vEnabled()'>Edit</button>
      </h4>

      <h4 style='color: white;'><span>Charger Voltage Setpoint: %CHG_VOLTAGE_SETPOINT% V </span> <button onclick='editChargerSetpointVDC()'>Edit</button></h4>

      <h4 style='color: white;'><span>Charger Current Setpoint: %CHG_CURRENT_SETPOINT% A</span> <button onclick='editChargerSetpointIDC()'>Edit</button></h4>

      </div>

      <button onclick="askFactoryReset()">Factory reset</button>
    
  </div>

)rawliteral"

const char settings_html[] =
    INDEX_HTML_HEADER COMMON_JAVASCRIPT SETTINGS_STYLE SETTINGS_HTML_BODY SETTINGS_HTML_SCRIPTS INDEX_HTML_FOOTER;
