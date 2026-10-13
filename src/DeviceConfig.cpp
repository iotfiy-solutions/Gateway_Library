#include "DeviceConfig.h"

// Default constructor
DeviceConfig::DeviceConfig() : _mqtt_port(1883) {}

// Chainable setters
DeviceConfig& DeviceConfig::ssid(const char* value) { 
  _ssid = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::password(const char* value) { 
  _password = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::server(const char* value) { 
  _mqtt_server = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::port(int value) { 
  _mqtt_port = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::clientId(const char* value) { 
  _mqtt_client_id = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::deviceId(const char* value) { 
  _device_id = value; 
  return *this; 
}

DeviceConfig& DeviceConfig::mqttUsername(const char* value) {
  _mqtt_username = value ? value : "";
  return *this;
}

DeviceConfig& DeviceConfig::mqttPassword(const char* value) {
  _mqtt_password = value ? value : "";
  return *this;
}

// Getters
const char* DeviceConfig::getSSID() const { 
  return _ssid.c_str(); 
}

const char* DeviceConfig::getPassword() const { 
  return _password.c_str(); 
}

const char* DeviceConfig::getMQTTServer() const { 
  return _mqtt_server.c_str(); 
}

int DeviceConfig::getMQTTPort() const { 
  return _mqtt_port; 
}

const char* DeviceConfig::getMQTTClientID() const { 
  return _mqtt_client_id.c_str(); 
}

const char* DeviceConfig::getDeviceID() const { 
  return _device_id.c_str(); 
}

const char* DeviceConfig::getMQTTUsername() const {
  return _mqtt_username.c_str();
}

const char* DeviceConfig::getMQTTPassword() const {
  return _mqtt_password.c_str();
}

// Helper methods for MQTT topics
String DeviceConfig::getDataTopic() const { 
  return String("devices/") + _device_id + "/data"; 
}

String DeviceConfig::getStatusTopic() const { 
  return String("devices/") + _device_id + "/status"; 
}

String DeviceConfig::getCommandsTopic() const { 
  return String("devices/") + _device_id + "/commands"; 
}

String DeviceConfig::getCommandResponseTopic() const { 
  return String("devices/") + _device_id + "/command-response"; 
}

bool DeviceConfig::isValid() const {
  return !_ssid.isEmpty() && !_password.isEmpty() && 
         !_mqtt_server.isEmpty() && !_mqtt_client_id.isEmpty() && 
         !_device_id.isEmpty();
}
