#include "Receive.h"

// Constructor for receives without range
Receive::Receive(const char* name, const char* type, const char* pin, float* entryPoint,
        PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  this->entryPoint = entryPoint;
  this->stringEntryPoint = nullptr;
  this->hasRange = false;
  this->isRegistered = false;
}

// Constructor for receives with range
Receive::Receive(const char* name, const char* type, const char* pin, float* entryPoint,
        PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  this->entryPoint = entryPoint;
  this->stringEntryPoint = nullptr;
  this->hasRange = true;
  this->min = min;
  this->max = max;
  this->isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, String* stringEntryPoint,
        PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  this->entryPoint = nullptr;
  this->stringEntryPoint = stringEntryPoint;
  this->hasRange = false;
  this->isRegistered = false;
}

// Getters
const char* Receive::getName() { return name; }
const char* Receive::getType() { return type; }
const char* Receive::getPin() { return pin; }
float* Receive::getEntryPoint() { return entryPoint; }

// Register capability without description
bool Receive::registerCapability() {
  DynamicJsonDocument receiveDoc(200);
  receiveDoc["deviceId"] = _config->getDeviceID();
  receiveDoc["type"] = "capabilities_control";
  receiveDoc["controlName"] = name;
  
  JsonObject receive = receiveDoc.createNestedObject("control");
  receive["type"] = type;
  receive["pin"] = pin;
  
  // Add min/max only for numeric types
  if (strcmp(type, "float") == 0 || strcmp(type, "integer") == 0) {
    if (hasRange) {
      receive["min"] = min;
      receive["max"] = max;
    }
  }
  
  String receivePayload;
  serializeJson(receiveDoc, receivePayload);
  
  String topic = String("devices/") + _config->getDeviceID() + "/capabilities_control/" + name;
  bool published = _client->publish(topic.c_str(), receivePayload.c_str(), true);
  
  Serial.print(name);
  Serial.print(" control published: ");
  Serial.println(published ? "SUCCESS" : "FAILED");
  
  if (published) {
    isRegistered = true;
  }
  
  delay(defaultDelay);
  return published;
}

// Handle command (will need to be overridden in child classes)
bool Receive::handleCommand(const JsonVariantConst& value, JsonObject& result) {
  // For basic types we can handle automatically
  if (strcmp(type, "float") == 0) {
    float floatValue = value.as<float>();
    *entryPoint = floatValue;  // Write to the variable
    result["newState"] = floatValue;
    return true;
  }
  else if (strcmp(type, "integer") == 0) {
    int intValue = value.as<int>();
    *entryPoint = intValue;  // Write to the variable
    result["newState"] = intValue;
    return true;
  }
  else if (strcmp(type, "boolean") == 0) {
    String strValue = value.as<String>();
    // Convert boolean string to numeric value (0 or 1)
    float boolValue = (strValue == "on" || strValue == "true" || strValue == "1") ? 1.0f : 0.0f;
    *entryPoint = boolValue;  // Write to the variable
    result["newState"] = strValue;
    return true;
  }
  else if (strcmp(type, "string") == 0) {
    String strValue = value.as<String>();
    if (stringEntryPoint) {
      *stringEntryPoint = strValue;
    }
    if (entryPoint) {
      *entryPoint = 1.0f;
    }
    Serial.print("String control ");
    Serial.print(pin);
    Serial.print(" = ");
    Serial.println(strValue);
    result["newState"] = strValue;
    return true;
  }
  
  return false;
}
