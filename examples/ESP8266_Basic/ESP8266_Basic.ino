/*
  IOTFIY — ESP8266 Basic example (NodeMCU / Wemos D1, etc.)

  Board: Tools → Board → ESP8266 Boards → NodeMCU 1.0 (or your ESP8266)
  Needs: PubSubClient, ArduinoJson, IOTFIY library

  Same API as ESP32 / M5 examples:
    IOTFIY.config → WiFi → begin → write/read → reconnectMQTT → loop
*/

#include <IOTFIY.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

// WiFi — fill these in
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// MQTT / device — fill these in
const char* MQTT_SERVER    = "YOUR_MQTT_HOST";  // e.g. "192.168.1.10" (no mqtt:// prefix)
const char* MQTT_CLIENT_ID = "esp8266_client_1";
const char* DEVICE_ID      = "device_esp8266_1";
const int   MQTT_PORT      = 1883;
// Optional auth:
// const char* MQTT_USER = "";
// const char* MQTT_PASS = "";

// Sensor value sent to the dashboard (IOTFIY.write)
float sensorValue = 0;

// Control value received from the dashboard (IOTFIY.read)
float controlValue = 0;

unsigned long lastPublishMs = 0;
const unsigned long PUBLISH_INTERVAL_MS = 3000;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("IOTFIY ESP8266 Basic"));

  IOTFIY.config(
    WIFI_SSID,
    WIFI_PASSWORD,
    MQTT_SERVER,
    MQTT_CLIENT_ID,
    DEVICE_ID,
    MQTT_PORT
  );
  // With MQTT username/password:
  // IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_CLIENT_ID, DEVICE_ID, MQTT_PORT, MQTT_USER, MQTT_PASS);

  IOTFIY.enableLWT(true);

  if (!IOTFIY.checkAndReconnectWiFi()) {
    Serial.println(F("WiFi connect failed — will retry in loop"));
  } else {
    Serial.print(F("WiFi OK, IP: "));
    Serial.println(WiFi.localIP());
  }

  IOTFIY.begin();

  // Dashboard ← device (output / sensor)
  IOTFIY.write("SensorValue", "float", "A0", &sensorValue);

  // Dashboard → device (control)
  IOTFIY.read("ControlValue", "float", "Ctrl", &controlValue, 0, 100);

  if (IOTFIY.reconnectMQTT()) {
    Serial.println(F("MQTT connected"));
  } else {
    Serial.println(F("MQTT connect failed — will retry in loop"));
  }
}

void loop() {
  IOTFIY.checkAndReconnectWiFi();

  if (!IOTFIY.isMQTTConnected()) {
    IOTFIY.reconnectMQTT();
  }

  IOTFIY.processMQTTMessages();

  // Demo: publish a changing value every few seconds
  unsigned long now = millis();
  if (now - lastPublishMs >= PUBLISH_INTERVAL_MS) {
    lastPublishMs = now;

#if defined(ESP8266)
    // NodeMCU A0 is 0–1023; scale to 0–100 for a simple demo reading
    sensorValue = (analogRead(A0) / 1023.0f) * 100.0f;
#else
    // ESP32 has no single standard A0 — use a fake ramp for demo
    sensorValue = fmod((now / 1000.0f) * 3.0f, 100.0f);
#endif

    IOTFIY.launch();

    Serial.print(F("Published sensorValue="));
    Serial.print(sensorValue);
    Serial.print(F("  controlValue="));
    Serial.println(controlValue);
  }

  delay(10);
}
