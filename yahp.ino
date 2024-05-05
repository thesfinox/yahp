/* 
  Yet Another Hydroponics Project (YAHP)
  
  Author: Riccardo Finotello
  
  A first project using a ESP32 Dev Module. A simple hydroponics capable to decide
  whether to irrigate our poor plants.
  
  Arduino IoT cloud reference sketch: https://create.arduino.cc/cloud/things/01230fa2-09ad-4d33-81da-7b189e58c540
  
  The following variables are automatically generated and updated when changes are made to the Thing

  CloudSwitch lightButton;
  CloudSwitch lighting;
  CloudSwitch lightSwitch;
  CloudSwitch waterButton;
  CloudSwitch watering;
  CloudSwitch waterSwitch;
  CloudTemperatureSensor temperature;
  CloudLuminousIntensity luminosity;
  CloudRelativeHumidity humidity;
  CloudRelativeHumidity moist_0;
  CloudRelativeHumidity moist_1;
  CloudRelativeHumidity moist_2;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/
#include "yahpProperties.h"
#include "thingProperties.h"


// Variables
int SUNRISE_HOUR = 0;  // hour of sunrise
int SUNRISE_MINUTE = 0;  // minute sunrise
int SUNSET_HOUR = 0;  // hour of sunset
int SUNSET_MINUTE = 0;  // minute of sunset

DHT dht(DHTPIN, DHTTYPE);  // humidity sensor
RTC_DS3231 rtc;  // real time clock

float moisture = 0.0; // actual value of the moisture
float moistureState = 0.0;  // average moisture
float moistureLastState = 0.0;  // last average moisture

float luminosityState = 0.0;  // luminosity
float luminosityLastState = 0.0;  // last luminosity

DateTime now;  // RTC now
int* yearPeriod;  // sunrise and set time (array - shape[4])
float light_intensity = 0.0;  // intensity of the light entering the photorestor
float day_intensity = 0.0;  // intensity connected to the moment of the day
int day_minutes = 0;  // minutes of light in the day

int wait = 0;  // wait interval

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize YAHP
  Serial.print("\n");
  Serial.print("*************************\n");
  Serial.print("    Initializing YAHP    \n");
  Serial.print("                         \n");
  Serial.print("    auth: thesfinox      \n");
  Serial.print("    ver:  2.0.0          \n");
  Serial.print("*************************\n\n");

  // Init the RTC
  if (! rtc.begin()) {
    Serial.println("Error: couldn't find RTC!");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("Error: RTC lost power, setting time...");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // Set the builtin led to output
  pinMode(WATERPIN, OUTPUT);
  pinMode(LIGHTPIN_0, OUTPUT);
  pinMode(LIGHTPIN_1, OUTPUT);
  pinMode(MOIST_0, INPUT);
  pinMode(MOIST_1, INPUT);
  pinMode(MOIST_2, INPUT);

  // Initialize the DHT interface
  dht.begin();

  // Determine night and day boundaries
  // See: https://www.worlddata.info/europe/france/sunset.php
  now = rtc.now();
  yearPeriod = onYearPeriod(now.month());
  SUNRISE_HOUR = yearPeriod[0];
  SUNRISE_MINUTE = yearPeriod[1];
  SUNSET_HOUR = yearPeriod[2];
  SUNSET_MINUTE = yearPeriod[3];
  day_minutes = (SUNSET_HOUR*60 + SUNSET_MINUTE) - (SUNRISE_HOUR*60 + SUNRISE_MINUTE);
  Serial.print("Date: ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year());
  Serial.print("\n");
  Serial.print("Sunrise: ");
  Serial.print(SUNRISE_HOUR);
  Serial.print(":");
  Serial.print(SUNRISE_MINUTE);
  Serial.print(" - Sunset: ");
  Serial.print(SUNSET_HOUR);
  Serial.print(":");
  Serial.print(SUNSET_MINUTE);
  Serial.print("\n");

  // Init the irrigation system
  watering = false;
  waterSwitch = true;
  waterButton = false;

  // Init the lights
  lighting = false;
  lightSwitch = true;
  lightButton = false;

  // Run tests
  Serial.println("Launching tests...");
  testPin(LIGHTPIN_0);
  testPin(LIGHTPIN_1);
  testPin(WATERPIN);
  
  delay(3000);
}

void loop() {
  ArduinoCloud.update();
  if (millis() - wait > WAIT_THRESH)
  {
    wait = millis();

    // Read RTC
    now = rtc.now();
    yearPeriod = onYearPeriod(now.month());
    SUNRISE_HOUR = yearPeriod[0];
    SUNRISE_MINUTE = yearPeriod[1];
    SUNSET_HOUR = yearPeriod[2];
    SUNSET_MINUTE = yearPeriod[3];
    day_minutes = (SUNSET_HOUR*60 + SUNSET_MINUTE) - (SUNRISE_HOUR*60 + SUNRISE_MINUTE);
    Serial.print("Time: ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);

    Serial.print(" - ");

    day_intensity = onDayPeriod(now, day_minutes, DAY_DESCENT, SUNRISE_HOUR, SUNRISE_MINUTE, SUNSET_HOUR, SUNSET_MINUTE);
    // day_intensity = 1.0;  // DEBUG
    Serial.print("Day int.: ");
    Serial.print(day_intensity);

    Serial.print(" - ");

    // Read sensors
    humidity = dht.readHumidity();
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%");

    Serial.print(" - ");
   
    temperature = dht.readTemperature();
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C");

    Serial.print(" - ");

    luminosityState = map(4095 - analogRead(PHOTOPIN), 0, 4095, 0, 100);
    Serial.print("Light: ");
    Serial.print(luminosityState);
    Serial.print("%");

    Serial.print(" - ");
    
    Serial.print("Out int.: ");
    Serial.print(lighting);

    Serial.print(" - ");

    moist_0 = map(4095 - analogRead(MOIST_0), 0, 4095, 0, 100);
    moist_1 = map(4095 - analogRead(MOIST_1), 0, 4095, 0, 100);
    moist_2 = map(4095 - analogRead(MOIST_2), 0, 4095, 0, 100);
    moistureState = (moist_0 + moist_1 + moist_2) / 3.0;
    Serial.print(" Moist: ");
    Serial.print(moist_0);
    Serial.print("% ");
    Serial.print(moist_1);
    Serial.print("% ");
    Serial.print(moist_2);
    Serial.print("%");

    Serial.print(" - ");
    
    Serial.print("Watering: ");
    Serial.print(watering);

    Serial.print("\n");
  }

  // Irrigation actions
  (moistureLastState > 0.0) ? moisture = (moistureState + moistureLastState) / 2.0 : moisture = moistureState;
  watering = switchConditionLogic(waterSwitch, day_intensity, moisture, MOIST_THRESH_DRY, MOIST_THRESH_WET, watering, waterButton);  // decide what to do
  activateDigitalPin(watering, WATERPIN);  // finally, activate, if needed
  moistureLastState = moisture;

  // Lighting actions
  (luminosityLastState > 0.0) ? luminosity = (luminosityState + luminosityLastState) / 2.0 : luminosity = luminosityState;
  lighting = switchConditionLogic(lightSwitch, day_intensity, luminosity, INTENSITY_THRESHOLD_LOW, INTENSITY_THRESHOLD_HIGH, lighting, lightButton);  // decide what to do
  activateDigitalPin(lighting, LIGHTPIN_0, LIGHTPIN_1);  // finally, activate, if needed
  luminosityLastState = luminosity;
}


/*
  Since WaterSwitch is READ_WRITE variable, onWaterSwitchChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onWaterSwitchChange() {}


/*
  Since LightSwitch is READ_WRITE variable, onLightSwitchChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLightSwitchChange() {}


/*
  Since LightButton is READ_WRITE variable, onLightButtonChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLightButtonChange()  {}

/*
  Since WaterButton is READ_WRITE variable, onWaterButtonChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onWaterButtonChange()  {}