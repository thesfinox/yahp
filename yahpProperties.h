/*
Yet Another Hydroponics Project (YAHP)

Author: Riccardo Finotello

Constants and functions signatures.
*/
#ifndef YAHP
#define YAHP

#include <DHT.h>
#include "RTClib.h"


// Constants
const int DHTPIN = 33;  // humidity sensor data pin
const int DHTTYPE = DHT22;  // type of humidity sensor

const int PHOTOPIN = 32;  // photoresistor pin
const int PHOTO_THRESH_DARK = 20;  // night value of the photoresistor (%)
const int PHOTO_THRESH_LIGHT = 60;  // day value of the photoresistor (%)

const int LIGHTPIN = 16;  // light switch
const int DAY_DESCENT = 60;  // length of light rise and fall (minutes)

const int WATERPIN = 0;  // pin of the water fountain
const int MOIST_0 = 34;  // pins of the moisture sensor
const int MOIST_1 = 35;  // pins of the moisture sensor
const int MOIST_2 = 39;  // pins of the moisture sensor
const int MOIST_THRESH_DRY = 30;  // threshold for watering (%)
const int MOIST_THRESH_WET = 60;  // threshold for watering (%)

const int BUTTONPIN = 4;  // pin of the physical button

const int WAIT_THRESH = 10*1000;  // serial console and sensor update waiting time (ms)

// Functions
float onLightChange(int luminosity, int thresh_dark, int thresh_light)
{
  if (luminosity <= thresh_dark) {return 1.0;}
  if (luminosity >= thresh_light) {return 0.0;}
  
  // Linear interpolation
  float m = - 1.0 / (thresh_light - thresh_dark);
  return m*(luminosity - thresh_dark) + 1.0;
}

int* onYearPeriod(int month)
{
  static int yearPeriod[4];

  // Depending on the month of the year sunrise and sunset change
  switch(month)
  {
    case 1:  // January
      yearPeriod[0] = 8;
      yearPeriod[1] = 37;
      yearPeriod[2] = 17;
      yearPeriod[3] = 22;
      break;
    case 2:  // February
      yearPeriod[0] = 7;
      yearPeriod[1] = 58;
      yearPeriod[2] = 18;
      yearPeriod[3] = 11;
      break;
    case 3:  // March
      yearPeriod[0] = 7;
      yearPeriod[1] = 1;
      yearPeriod[2] = 18;
      yearPeriod[3] = 57;
      break;
    case 4:  // April
      yearPeriod[0] = 6;
      yearPeriod[1] = 56;
      yearPeriod[2] = 20;
      yearPeriod[3] = 44;
      break;
    case 5:  // May
      yearPeriod[0] = 6;
      yearPeriod[1] = 6;
      yearPeriod[2] = 9;
      yearPeriod[3] = 27;
      break;
    case 6:  // June
      yearPeriod[0] = 5;
      yearPeriod[1] = 44;
      yearPeriod[2] = 21;
      yearPeriod[3] = 58;
      break;
    case 7:  // July
      yearPeriod[0] = 6;
      yearPeriod[1] = 2;
      yearPeriod[2] = 21;
      yearPeriod[3] = 51;
      break;
    case 8:  // August
      yearPeriod[0] = 6;
      yearPeriod[1] = 42;
      yearPeriod[2] = 21;
      yearPeriod[3] = 7;
      break;
    case 9:  // September
      yearPeriod[0] = 7;
      yearPeriod[1] = 26;
      yearPeriod[2] = 20;
      yearPeriod[3] = 4;
      break;
    case 10:  // October
      yearPeriod[0] = 8;
      yearPeriod[1] = 10;
      yearPeriod[2] = 19;
      yearPeriod[3] = 2;
      break;
    case 11:  // November
      yearPeriod[0] = 7;
      yearPeriod[1] = 58;
      yearPeriod[2] = 17;
      yearPeriod[3] = 11;
      break;
    case 12:  // December
      yearPeriod[0] = 8;
      yearPeriod[1] = 35;
      yearPeriod[2] = 16;
      yearPeriod[3] = 56;
      break;
  }
  return yearPeriod;
}

float onDayPeriod(DateTime now, int max_minutes, int descent, int rise_hour, int rise_minute, int set_hour, int set_minute)
{
    // Compute the day progress
    int now_minutes = now.hour()*60 + now.minute();
    int sunrise_minute = rise_hour*60 + rise_minute;
    int sunset_minute = set_hour*60 + set_minute;
    int day_delta = sunset_minute - sunrise_minute;
    int diff_minutes = now_minutes - sunrise_minute;
    if (diff_minutes <= 0 || diff_minutes >= day_delta) {return 0.0;}

    // Linear interpolation (rise to 255 in X minutes, set to 0 in 60 minutes)
    if (diff_minutes >= descent && diff_minutes < day_delta - descent) {return 1.0;};
    float m = 1.0 / (float)descent;
    if (diff_minutes < descent)
    {
      return m * diff_minutes;
    } else
    {
      return 1.0 - m*(diff_minutes - (day_delta - descent));
    }
}

#endif