# Nano-Blynk-Deep-Sleep-Exp
Experimenting with the Arduino Nano ESP32 + Blynk operating in Deep Sleep Mode.



## Operation

The Arduino Nano ESP32 loads all the parameters and configures the modules to operate in deep sleep (~7uA) for 30 minutes, wake up and update the Blynk App on my phone.  The Nano is only on for ~10 seconds before going back in sleep.

|                     | Active Mode | Deep Sleep      |
| ------------------- | ----------- | --------------- |
| Power Dissipation   | 400mW       | 0.035mW         |
| Current Dissipation | 80mA        | 0.00007mA (7uA) |
| Duration Active     | 10 seconds  | 30 minutes      |

Note: Operating at 5.1V
