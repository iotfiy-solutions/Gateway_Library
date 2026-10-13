#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <Arduino.h>

class DeviceConfig {
private:
  // WiFi credentials
  String _ssid;
  String _password;
  
  // MQTT Broker settings
  String _mqtt_server;
  int _mqtt_port = 1883; // Default value
  String _mqtt_client_id;
  String _mqtt_username;
  String _mqtt_password;
  
  // Device info
  String _device_id;

public:
  // Default constructor
  DeviceConfig();
  
  // Chainable setters - returns reference to this object
  DeviceConfig& ssid(const char* value);
  DeviceConfig& password(const char* value);
  DeviceConfig& server(const char* value);
  DeviceConfig& port(int value);
  DeviceConfig& clientId(const char* value);
  DeviceConfig& deviceId(const char* value);
  DeviceConfig& mqttUsername(const char* value);
  DeviceConfig& mqttPassword(const char* value);
  
  // Getters
  const char* getSSID() const;
  const char* getPassword() const;
  const char* getMQTTServer() const;
  int getMQTTPort() const;
  const char* getMQTTClientID() const;
  const char* getDeviceID() const;
  const char* getMQTTUsername() const;
  const char* getMQTTPassword() const;
  
  // Helper methods for MQTT topics
  String getDataTopic() const;
  String getStatusTopic() const;
  String getCommandsTopic() const;
  String getCommandResponseTopic() const;

  bool isValid() const;
};

#endif // DEVICE_CONFIG_H
