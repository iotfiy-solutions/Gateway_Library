#include "Send.h"

// Constructor for sends without range
Send::Send(const char* name, const char* type, const char* pin, float* exitPoint,
       PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  this->exitPoint = exitPoint;
  this->hasRange = false;  
  this->isRegistered = false;
}

// Constructor with range
Send::Send(const char* name, const char* type, const char* pin, float* exitPoint,
       PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  this->exitPoint = exitPoint;
  this->hasRange = true;
  this->min = min;
  this->max = max;
  this->isRegistered = false;
}

// Getters
const char* Send::getName() { return name; }
const char* Send::getType() { return type; }
const char* Send::getPin() { return pin; }
bool Send::hasRangeValues() { return hasRange; }
float Send::getMin() { return min; }
float Send::getMax() { return max; }
bool Send::isCapabilityRegistered() { return isRegistered; }
float Send::getCurrentValue() { 
  return *exitPoint;  // Read from the variable directly 
}

// Setters
void Send::setDelay(int ms) { defaultDelay = ms; }

// Register capability similar to v3.3.3
bool Send::registerCapability() {
  DynamicJsonDocument outputDoc(200);
  outputDoc["deviceId"] = _config->getDeviceID();
  outputDoc["type"] = "capabilities_output";
  outputDoc["outputName"] = name;
  
  JsonObject output = outputDoc.createNestedObject("output");
  output["type"] = type;
  output["pin"] = pin;
  
  String outputPayload;
  serializeJson(outputDoc, outputPayload);
  
  String topic = String("devices/") + _config->getDeviceID() + "/capabilities_output/" + name;
  bool published = _client->publish(topic.c_str(), outputPayload.c_str(), true);
  
  Serial.print(name);
  Serial.print(" output published: ");
  Serial.println(published ? "SUCCESS" : "FAILED");
  
  if (published) {
    isRegistered = true;
  }
  
  delay(defaultDelay);
  return published;
}

// Publish data for individual send
bool Send::publishData() {
  // Get value from the actual variable
  float value = *exitPoint;
  
  // Use min/max to constrain the value if needed
  if (hasRange) {
    value = constrain(value, min, max);
  }
  
  if (!isRegistered) {
    registerCapability();
  }
  
  // Publish individual data
  DynamicJsonDocument dataDoc(256);
  dataDoc["type"] = "output";
  dataDoc["deviceId"] = _config->getDeviceID();
  JsonObject data = dataDoc.createNestedObject("data");
  
  data[name] = value;
  if (strlen(pin) > 0) {
    data[pin] = value;
  }
  
  String dataPayload;
  serializeJson(dataDoc, dataPayload);
  
  Serial.print("Send data payload size: ");
  Serial.print(dataPayload.length());
  Serial.println(" bytes");
  
  bool published = _client->publish(_config->getDataTopic().c_str(), dataPayload.c_str());
  Serial.print("Send data published: ");
  Serial.println(published ? "SUCCESS" : "FAILED");
  
  return published;
}
