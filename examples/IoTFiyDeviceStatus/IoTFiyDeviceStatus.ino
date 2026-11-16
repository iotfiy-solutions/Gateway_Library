/*
  IOTFIY — Device Status
  Device connects and shows online on the dashboard.
*/

#include <IOTFIY.h>

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("IOTFIY minimal online test");

  IOTFIY.config("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD", "YOUR_DEVICE_ID");
  IOTFIY.begin();
}

void loop() {
  // default: publish-on-change (no interval)
  IOTFIY.launch();
  // IOTFIY.launch(5000);   // optional: send write data every 5s
}
