/*
  IOTFIY — ESP8266 / ESP32 Basic example

  Board (ESP8266): Tools → Board → ESP8266 Boards → NodeMCU 1.0 (or your board)
  Board (ESP32):   Tools → Board → ESP32 Arduino → your module
  Libraries:       PubSubClient, ArduinoJson, IOTFIY

  Flow:
    config → begin → write / read → launch (setup)
    update variables → launch (loop)

  Built into the library:
    MQTT broker / auth, LWT, WiFi + MQTT reconnect, message processing

  write() / read():
    - Type is inferred from the variable (float* → float, int* → integer,
      bool* → boolean, String* → string)
    - Name is optional (if omitted, pin is used as the name)

    IOTFIY.write("A0", &sensorValue);
    IOTFIY.write("battery", "V9", &battery, 0, 100);   // int* → integer
    IOTFIY.read("Ctrl", &controlValue, 0, 100);
    IOTFIY.read("color", "V4", &colorHex);             // String* → string
*/

#include <IOTFIY.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_ID     = "device_esp8266_1";

float sensorValue  = 0;  // → type "float"
float controlValue = 0;  // → type "float"

unsigned long lastUpdateMs = 0;
const unsigned long UPDATE_INTERVAL_MS = 3000;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("IOTFIY Basic example"));

  IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);
  IOTFIY.begin();

  // Type from variable; name defaults to pin
  IOTFIY.write("A0", &sensorValue);
  IOTFIY.read("Ctrl", &controlValue, 0, 100);

  IOTFIY.launch();
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdateMs >= UPDATE_INTERVAL_MS) {
    lastUpdateMs = now;

#if defined(ESP8266)
    sensorValue = (analogRead(A0) / 1023.0f) * 100.0f;
#else
    sensorValue = fmod((now / 1000.0f) * 3.0f, 100.0f);
#endif

    Serial.print(F("sensorValue="));
    Serial.print(sensorValue);
    Serial.print(F("  controlValue="));
    Serial.println(controlValue);
  }

  IOTFIY.launch();
  delay(10);
}
