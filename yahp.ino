/* 
  Yet Another Hydroponics Project (YAHP)
  
  Author: Riccardo Finotello
  
  A first project using a ESP32 Dev Module. A simple hydroponics capable to decide
  whether to irrigate our poor plants.
  
  Arduino IoT cloud reference sketch: https://create.arduino.cc/cloud/things/01230fa2-09ad-4d33-81da-7b189e58c540
  
  The following variables are automatically generated and updated when changes are made to the Thing

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

DateTime now;  // RTC now
int* yearPeriod;  // sunrise and set time (array - shape[4])
int day_minutes = 0;  // minutes of light in the day

bool wateringLast = false;  // last watering status
unsigned long changeTime = 0;  // watering change (ms)
unsigned long timeDelta = 0;  // (ms)

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
  Serial.print("    ver:  2.1.0          \n");
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
  pinMode(FANPIN, OUTPUT);
  pinMode(MOIST_0, INPUT);
  pinMode(MOIST_1, INPUT);
  pinMode(MOIST_2, INPUT);
  pinMode(MOIST_3, INPUT);

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
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.print(now.second());
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

  // Init the fans
  ventilation = false;
  fanSwitch = true;
  fanButton = false;

  // Run tests
  Serial.println("Launching tests...");
  testOutputPin(LIGHTPIN_0);
  testOutputPin(LIGHTPIN_1);
  testOutputPin(WATERPIN);
  testOutputPin(FANPIN);  
  
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
    // day_intensity = 100.0;  // DEBUG
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

    luminosity = map(4095 - analogRead(PHOTOPIN), 0, 4095, 0, 100);
    Serial.print("Light: ");
    Serial.print(luminosity);
    Serial.print("%");

    Serial.print(" - ");
    
    Serial.print("Out int.: ");
    Serial.print(lighting);

    Serial.print(" - ");

    moist_0 = map(4095 - analogRead(MOIST_0), 0, 4095, 0, 100);
    moist_1 = map(4095 - analogRead(MOIST_1), 0, 4095, 0, 100);
    moist_2 = map(4095 - analogRead(MOIST_2), 0, 4095, 0, 100);
    moist_3 = map(4095 - analogRead(MOIST_3), 0, 4095, 0, 100);
    moisture = (moist_0 + moist_1 + moist_2 + moist_3) / 4.0;
    Serial.print(" Moist: ");
    Serial.print(moist_0);
    Serial.print("% ");
    Serial.print(moist_1);
    Serial.print("% ");
    Serial.print(moist_2);
    Serial.print("% ");
    Serial.print(moist_3);
    Serial.print("%");

    Serial.print(" - ");
    
    Serial.print("Watering: ");
    Serial.print(watering);

    Serial.print(" - ");
    
    Serial.print("Ventilation: ");
    Serial.print(ventilation);

    Serial.print("\n");
  }

  // Actions
  watering = switchConditionLogic(waterSwitch, day_intensity, moisture, MOIST_THRESH_DRY, MOIST_THRESH_WET, watering, waterButton);  // decide what to do
  lighting = switchConditionLogic(lightSwitch, day_intensity, luminosity, INTENSITY_THRESHOLD_LOW, INTENSITY_THRESHOLD_HIGH, lighting, lightButton);  // decide what to do
  ventilation = switchConditionLogic(fanSwitch, 1.0, 100.0 - humidity, 100 - FAN_THRESH_WET, 100 - FAN_THRESH_DRY, ventilation, fanButton);  // decide what to do
  if (watering)
  {
    ventilation = false;
  }
  activateDigitalPin(watering, WATERPIN);  // finally, activate, if needed
  activateDigitalPin(lighting, LIGHTPIN_0, LIGHTPIN_1);  // finally, activate, if needed
  activateDigitalPin(ventilation, FANPIN);  // finally, activate, if needed


  // Keep track of time
  if (watering != wateringLast)  // if toggled...
  {
    changeTime = millis();  // ...record the time
  }
  timeDelta = millis() - changeTime;  // measure duration
  if (watering)  // if irrrigating...
  {
    if (timeDelta > TIMER)  // ...and time is up...
    {
      waterSwitch = false;  // ...turn the switch off
    }
  } else  // if it is not irrigating...
  {
    if (!waterSwitch && (timeDelta > TIMER))  // ...and the switch is off, and time is up...
    {
      waterSwitch = true;  // ...turn the switch on
    }
  }
  wateringLast = watering;
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

/*
  Since FanSwitch is READ_WRITE variable, onFanSwitchChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onFanSwitchChange()  {}

/*
  Since FanButton is READ_WRITE variable, onFanButtonChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onFanButtonChange()  {}

/*
  Since Ventilation is READ_WRITE variable, onVentilationChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onVentilationChange()  {}
