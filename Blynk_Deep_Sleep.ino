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
//#   Version History:
//#   
//#       Date        Description
//#     -----------   -----------------------------------------------------------------------
//#      2023-05-08    Original Creation
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
