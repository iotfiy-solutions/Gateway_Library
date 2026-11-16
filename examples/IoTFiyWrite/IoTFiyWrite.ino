/*
  IOTFIY — Minimal Write
  Device → cloud / dashboard (IOTFIY.write)
  store: omit / store=false → no Influx history; store=true → Influx history
*/

#include <IOTFIY.h>

float sensorValue = 59.0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("IOTFIY minimal write test");

  IOTFIY.config("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD", "YOUR_DEVICE_ID");
  IOTFIY.begin();

  // --- write options (pick one) ---
  // pin only (name defaults to pin); type from variable; store default false
  // IOTFIY.write("pin6", &sensorValue);
  // IOTFIY.write("pin6", &sensorValue, store=false);
  IOTFIY.write("pin6", &sensorValue, store = true);

  // pin + min/max (+ optional store)
  // IOTFIY.write("pin6", &sensorValue, 0, 100);
  // IOTFIY.write("pin6", &sensorValue, 0, 100, store=false);
  // IOTFIY.write("pin6", &sensorValue, 0, 100, store=true);

  // name + pin (+ optional store / min/max)
  // IOTFIY.write("temperature", "pin6", &sensorValue);
  // IOTFIY.write("temperature", "pin6", &sensorValue, store=true);
  // IOTFIY.write("temperature", "pin6", &sensorValue, 0, 100);
  // IOTFIY.write("temperature", "pin6", &sensorValue, 0, 100, store=true);

  // name + type + pin (explicit type, float* storage)
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue);
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue, store=true);
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue, 0, 100);
  // IOTFIY.write("temperature", "float", "pin6", &sensorValue, 0, 100, store=true);
}

void loop() {
  // default: publish-on-change (no interval)
  IOTFIY.launch();
  // IOTFIY.launch(10000);   // optional: send write data every 10s
}
