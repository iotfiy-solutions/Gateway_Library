#ifndef IOTFIY_H
#define IOTFIY_H

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #error "IOTFIY supports ESP8266 and ESP32 only"
#endif

#ifndef MQTT_MAX_PACKET_SIZE
  #define MQTT_MAX_PACKET_SIZE 512
#endif
#ifndef TCP_BUFFER_SIZE
  #define TCP_BUFFER_SIZE 1024
#endif
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DeviceConfig.h"
#include "Send.h"
#include "Receive.h"

class IOTFIYManager {
private:
  static const int MAX_SENDS = 10;
  static const int MAX_RECEIVES = 10;

  Send* _sends[MAX_SENDS];
  Receive* _receives[MAX_RECEIVES];
  int _sendCount = 0;
  int _receiveCount = 0;

  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  DeviceConfig _config;
  bool _lastWifiOk = false;
  bool _lastMqttOk = false;
  bool _statusInitialized = false;

  void handleCallback(char* topic, byte* payload, unsigned int length);
  bool reconnectMQTT();
  bool checkAndReconnectWiFi();
  void processMQTTMessages();
  void printConnectionStatus(bool wifiOk, bool mqttOk);
  static const char* resolveCapabilityName(const char* name, const char* pin);

public:
  IOTFIYManager();
  DeviceConfig& config();
  void begin();
  void registerAllCapabilities();
  void launch();
  bool publishStringOutput(const char* name, const char* pin, const String& value);
  bool isMQTTConnected();
  // device_id: plain or Base64 (decoded internally); also used as MQTT client id
  void config(const char* ssid, const char* password, const char* device_id);

  // Type is inferred from the variable pointer:
  //   float* → "float", int* → "integer", bool* → "boolean", String* → "string"
  // Name is optional — omit it and pin is used as the name.

  Send* write(const char* pin, float* dataSource);
  Send* write(const char* pin, float* dataSource, float min, float max);
  Send* write(const char* pin, int* dataSource);
  Send* write(const char* pin, int* dataSource, float min, float max);
  Send* write(const char* pin, bool* dataSource);
  Send* write(const char* name, const char* pin, float* dataSource);
  Send* write(const char* name, const char* pin, float* dataSource, float min, float max);
  Send* write(const char* name, const char* pin, int* dataSource);
  Send* write(const char* name, const char* pin, int* dataSource, float min, float max);
  Send* write(const char* name, const char* pin, bool* dataSource);

  // Explicit type still supported (float* storage)
  Send* write(const char* name, const char* type, const char* pin, float* dataSource);
  Send* write(const char* name, const char* type, const char* pin, float* dataSource, float min, float max);

  Receive* read(const char* pin, float* entryPoint);
  Receive* read(const char* pin, float* entryPoint, float min, float max);
  Receive* read(const char* pin, int* entryPoint);
  Receive* read(const char* pin, int* entryPoint, float min, float max);
  Receive* read(const char* pin, bool* entryPoint);
  Receive* read(const char* pin, String* entryPoint);
  Receive* read(const char* name, const char* pin, float* entryPoint);
  Receive* read(const char* name, const char* pin, float* entryPoint, float min, float max);
  Receive* read(const char* name, const char* pin, int* entryPoint);
  Receive* read(const char* name, const char* pin, int* entryPoint, float min, float max);
  Receive* read(const char* name, const char* pin, bool* entryPoint);
  Receive* read(const char* name, const char* pin, String* entryPoint);

  // Explicit type still supported
  Receive* read(const char* name, const char* type, const char* pin, float* entryPoint);
  Receive* read(const char* name, const char* type, const char* pin, float* entryPoint, float min, float max);
  Receive* read(const char* name, const char* type, const char* pin, String* entryPoint);
};

extern IOTFIYManager IOTFIY;

#endif // IOTFIY_H
