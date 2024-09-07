//#############################################################################################
//#############################################################################################
//#
//#   The MIT License (MIT)
//#   
//#   Copyright (c) 2024 http://odelay.io 
//#   
//#   Permission is hereby granted, free of charge, to any person obtaining a copy
//#   of this software and associated documentation files (the "Software"), to deal
//#   in the Software without restriction, including without limitation the rights
//#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//#   copies of the Software, and to permit persons to whom the Software is
//#   furnished to do so, subject to the following conditions:
//#   
//#   The above copyright notice and this permission notice shall be included in all
//#   copies or substantial portions of the Software.
//#   
//#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//#   SOFTWARE.
//#   
//#   Contact : <everett@odelay.io>
//#  
//#   Description : Blynk Deep Sleep Verification
//#                   Voltage Level               : 5.10V
//#                   Power Consump Active State  : 0.400W
//#                   Power Consump Deep Sleep    : 0.000001W (~7uA)
//#
//#
//#
//#       Time struct information:
//#
//#             %A	Full weekday name
//#             %B	Full month name
//#             %d	Day of the month
//#             %Y	Year
//#             %H	Hour in 24h format
//#             %I	Hour in 12h format
//#             %M	Minute
//#             %S	Second
//#           
//#             timeinfo.tm_sec	    int	seconds after the minute	0-61*
//#             timeinfo.tm_min	    int	minutes after the hour	0-59
//#             timeinfo.tm_hour	  int	hours since midnight	0-23
//#             timeinfo.tm_mday	  int	day of the month	1-31
//#             timeinfo.tm_mon	    int	months since January	0-11
//#             timeinfo.tm_year	  int	years since 1900	
//#             timeinfo.tm_wday	  int	days since Sunday	0-6
//#             timeinfo.tm_yday	  int	days since January 1	0-365
//#             timeinfo.tm_isdst	  int	Daylight Saving Time flag	
//#           
//#
//#   Version History:
//#   
//#       Date        Description
//#     -----------   -----------------------------------------------------------------------
//#      2024-09-05    Original Creation
//#      2024-09-07    Added time stamp to Blynk App using NTP
//#
//###########################################################################################


#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

// Your WiFi credentials.
char SSID[] = "";
char PASSCODE[] = "";


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -7*3600; // -7 hours
const int   daylightOffset_sec = 0;



//---------------------------------------------------------
// Device Parameters
//---------------------------------------------------------
#define TIME_TO_SLEEP  (30 * 60 * 1000000) // Seconds * 1e-6
int disable_deepsleep = 1;
BlynkTimer timer;


//---------------------------------------------------------
//  deepSleep function
//---------------------------------------------------------
void deepSleep() {
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 

  esp_deep_sleep_start();
}


//---------------------------------------------------------
//  Setup WiFi Module
//---------------------------------------------------------
void setupWifi() {
  WiFi.begin(SSID, PASSCODE);

  uint8_t retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);

    if ((++retry % 16) == 0) {
      Serial.println("WiFi Failed to Connect");
      delay(1000);
      deepSleep();
    }
  }

  Serial.print("WiFi Connected :");
  Serial.println(WiFi.localIP());
}


//---------------------------------------------------------
//  Check the Blynk status and update hw/App
//---------------------------------------------------------
void checkBlynkStatus() {

  // Get local time
  char locTime[20];
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    Blynk.virtualWrite(V2, "N/A");
  } else {
    sprintf(locTime, "%02d/%02d/%02d %02d:%02d:%02d",\
        timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_year+1900,\
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.println(locTime);
    Blynk.virtualWrite(V2, locTime);
  }

  // update random number 
  int a = random(4095);
  Blynk.virtualWrite(V1, a);
  Serial.print("Random Number Sent: ");
  Serial.println(a);

  // Check if deepSleep is enabled
  if (disable_deepsleep == 0) {
    Serial.println("Entering Deep Sleep");
    delay(1000);
    deepSleep();
  } else {
    Serial.println("Disabled Deep Sleep");
  }

}

//---------------------------------------------------------
//  Read Blynk disable_deepsleep value
//---------------------------------------------------------
BLYNK_WRITE(V0)
{
  disable_deepsleep = param.asInt();
  Serial.print("Deepsleep Status: ");
  Serial.println(!disable_deepsleep);
}


//---------------------------------------------------------
//  Print status and sync Blynk VirtualPins
//---------------------------------------------------------
BLYNK_CONNECTED() {
  Serial.println("Connected to blynk");
  Blynk.syncAll();
}



//---------------------------------------------------------
//  Setup function
//---------------------------------------------------------
void setup() {
  // Init system
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP);
  randomSeed(analogRead(3));
    
  setupWifi();
 
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
  // Configure Blynk
  Serial.println("Configure Blynk with AUTH...");
  Blynk.config(BLYNK_AUTH_TOKEN);

  // Blynk.connect() will wait for 30 seconds to connected
  // and return false if device doesn't connect 
  Serial.println("Connecting to Blynk...");
  uint8_t retry = 0;
  while (Blynk.connect() == false) {
    Serial.print("x");
     
    if ((++retry % 16) == 0) {
      Serial.println("FAILED connect to Blynk.");
      delay(1000);
      deepSleep();
    }
  }
  
  Serial.println("Execute Blynk_run()...");
  Blynk.run();
 
  // Update Blynk display with random number
  Serial.println("Blynk syncAll and push random number to App");
  Blynk.syncAll();
  Blynk.virtualWrite(V1, random(4095));
 
  // Check if deepSleep is enabled
  if (disable_deepsleep == 0) {
    Serial.println("Enabled Deep Sleep");
    delay(1000);
    deepSleep();
  } else {
    Serial.println("Disabled Deep Sleep");
    timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  }

}



//---------------------------------------------------------
//  Enter loop if disabled deep sleep
//---------------------------------------------------------
void loop() {
  Blynk.run();
  timer.run();
}
