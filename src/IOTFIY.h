#ifndef IOTFIY_H
#define IOTFIY_H

#include <Arduino.h>

// ESP32 uses WiFi.h; ESP8266 uses ESP8266WiFi.h (same WiFiClient / WiFi API surface)
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
  
  // Storage arrays
  Send* _sends[MAX_SENDS];
  Receive* _receives[MAX_RECEIVES];
  int _sendCount = 0;
  int _receiveCount = 0;
  
  // Connection management
  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  DeviceConfig _config;
  bool _lwtEnabled = false;
  
  // Process incoming commands
  void handleCallback(char* topic, byte* payload, unsigned int length);
  
public:
  // Constructor
  IOTFIYManager();
  
  // Config access
  DeviceConfig& config();
  
  // Initialize
  void begin();

  // MQTT Last Will: broker publishes offline on unexpected disconnect
  void enableLWT(bool enabled = true);
  
  // Registration and publishing
  void registerAllCapabilities();
  void launch();
  bool publishStringOutput(const char* name, const char* pin, const String& value);
  
  // Connection management
  bool reconnectMQTT();
  bool checkAndReconnectWiFi();
  bool isMQTTConnected();
  void processMQTTMessages();
  
  // Direct configuration method
  void config(const char* ssid, const char* password, const char* mqtt_server, 
              const char* mqtt_client_id, const char* device_id, int mqtt_port = 1883,
              const char* mqtt_username = "", const char* mqtt_password = "");
  
  Send* write(const char* name, const char* type, const char* pin, float* dataSource);
  Send* write(const char* name, const char* type, const char* pin, float* dataSource, float min, float max);
  Receive* read(const char* name, const char* type, const char* pin, float* entryPoint);
  Receive* read(const char* name, const char* type, const char* pin, float* entryPoint, float min, float max);
  Receive* read(const char* name, const char* type, const char* pin, String* entryPoint);
};

// Global instance declaration
extern IOTFIYManager IOTFIY;

#endif // IOTFIY_H
