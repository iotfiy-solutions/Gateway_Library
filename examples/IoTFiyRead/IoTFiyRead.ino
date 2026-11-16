/*
  IOTFIY — Read
  Cloud / dashboard → device (IOTFIY.read)
*/

#include <IOTFIY.h>

float controlValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("IOTFIY minimal read test");

  IOTFIY.config("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD", "YOUR_DEVICE_ID");
  IOTFIY.begin();

  // --- read options (pick one) ---
  // pin only (name defaults to pin); type from variable
  IOTFIY.read("V5", &controlValue);
  // IOTFIY.read("V5", &controlValue, 0, 100);              // pin + min/max

  // name + pin
  // IOTFIY.read("Display", "V5", &controlValue);
  // IOTFIY.read("Display", "V5", &controlValue, 0, 100);   // name + pin + min/max

  // name + type + pin (explicit type)
  // IOTFIY.read("Display", "float", "V5", &controlValue);
  // IOTFIY.read("Display", "float", "V5", &controlValue, 0, 100);
}

void loop() {
  // default: publish-on-change (no interval)
  IOTFIY.launch();
  // IOTFIY.launch(5000);   // optional: send write data every 5s

  Serial.print("V5 = ");
  Serial.println(controlValue);
}
