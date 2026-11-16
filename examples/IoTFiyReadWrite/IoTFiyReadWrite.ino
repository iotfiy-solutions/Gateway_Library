/*
  IOTFIY — Minimal Read + Write
  Device → cloud (write) and cloud → device (read)
*/

#include <IOTFIY.h>

float sensorValue = 30.0;
float controlValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("IOTFIY minimal read/write test");

  IOTFIY.config("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD", "YOUR_DEVICE_ID");
  IOTFIY.begin();

  // --- write options (pick one) ---
  // IOTFIY.write("pin6", &sensorValue);
  // IOTFIY.write("pin6", &sensorValue, store=false);
  IOTFIY.write("pin6", &sensorValue, store = true);

  // IOTFIY.write("pin6", &sensorValue, 0, 100);
  // IOTFIY.write("pin6", &sensorValue, 0, 100, store=true);

  // IOTFIY.write("temperature", "pin6", &sensorValue);
  // IOTFIY.write("temperature", "pin6", &sensorValue, store=true);
  // IOTFIY.write("temperature", "pin6", &sensorValue, 0, 100);
  // IOTFIY.write("temperature", "pin6", &sensorValue, 0, 100, store=true);

  // IOTFIY.write("temperature", "float", "pin6", &sensorValue);
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue, store=true);
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue, 0, 100, store=true);

  // --- read options (pick one) ---
  // IOTFIY.read("V5", &controlValue);
  IOTFIY.read("V5", &controlValue, 0, 100);

  // IOTFIY.read("Display", "V5", &controlValue);
  // IOTFIY.read("Display", "V5", &controlValue, 0, 100);

  // IOTFIY.read("Display", "float", "V5", &controlValue);
  // IOTFIY.read("Display", "float", "V5", &controlValue, 0, 100);
}

void loop() {
  // default: publish-on-change (no interval)
  IOTFIY.launch();
  // IOTFIY.launch(5000);   // optional: send write data every 5s

  Serial.println(controlValue);
}
