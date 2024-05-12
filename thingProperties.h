// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "2c487995-835c-448e-9726-25863c16d3e4";

const char SSID[]               = SECRET_SSID;    // Network SSID (name)
const char PASS[]               = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = SECRET_DEVICE_KEY;    // Secret device password

void onFanButtonChange();
void onFanSwitchChange();
void onLightButtonChange();
void onLightSwitchChange();
void onVentilationChange();
void onWaterButtonChange();
void onWaterSwitchChange();

CloudSwitch fanButton;
CloudSwitch fanSwitch;
CloudSwitch lightButton;
CloudSwitch lighting;
CloudSwitch lightSwitch;
CloudSwitch ventilation;
CloudSwitch waterButton;
CloudSwitch watering;
CloudSwitch waterSwitch;
CloudTemperatureSensor temperature;
CloudLuminousIntensity luminosity;
CloudPercentage day_intensity;
CloudRelativeHumidity humidity;
CloudRelativeHumidity moist_0;
CloudRelativeHumidity moist_1;
CloudRelativeHumidity moist_2;
CloudRelativeHumidity moist_3;
CloudRelativeHumidity moisture;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(fanButton, READWRITE, ON_CHANGE, onFanButtonChange);
  ArduinoCloud.addProperty(fanSwitch, READWRITE, ON_CHANGE, onFanSwitchChange);
  ArduinoCloud.addProperty(lightButton, READWRITE, ON_CHANGE, onLightButtonChange);
  ArduinoCloud.addProperty(lighting, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(lightSwitch, READWRITE, ON_CHANGE, onLightSwitchChange);
  ArduinoCloud.addProperty(ventilation, READWRITE, ON_CHANGE, onVentilationChange);
  ArduinoCloud.addProperty(waterButton, READWRITE, ON_CHANGE, onWaterButtonChange);
  ArduinoCloud.addProperty(watering, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(waterSwitch, READWRITE, ON_CHANGE, onWaterSwitchChange);
  ArduinoCloud.addProperty(temperature, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(luminosity, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(day_intensity, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(humidity, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(moist_0, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(moist_1, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(moist_2, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(moist_3, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(moisture, READ, 60 * SECONDS, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
