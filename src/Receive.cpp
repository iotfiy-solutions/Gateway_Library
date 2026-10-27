#include "Receive.h"

Receive::Receive(const char* name, const char* type, const char* pin, float* entryPoint,
                 PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_FLOAT;
  floatEntry = entryPoint;
  intEntry = nullptr;
  boolEntry = nullptr;
  stringEntry = nullptr;
  hasRange = false;
  isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, float* entryPoint,
                 PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_FLOAT;
  floatEntry = entryPoint;
  intEntry = nullptr;
  boolEntry = nullptr;
  stringEntry = nullptr;
  hasRange = true;
  this->min = min;
  this->max = max;
  isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, int* entryPoint,
                 PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_INT;
  floatEntry = nullptr;
  intEntry = entryPoint;
  boolEntry = nullptr;
  stringEntry = nullptr;
  hasRange = false;
  isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, int* entryPoint,
                 PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_INT;
  floatEntry = nullptr;
  intEntry = entryPoint;
  boolEntry = nullptr;
  stringEntry = nullptr;
  hasRange = true;
  this->min = min;
  this->max = max;
  isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, bool* entryPoint,
                 PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_BOOL;
  floatEntry = nullptr;
  intEntry = nullptr;
  boolEntry = entryPoint;
  stringEntry = nullptr;
  hasRange = false;
  isRegistered = false;
}

Receive::Receive(const char* name, const char* type, const char* pin, String* stringEntryPoint,
                 PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_STRING;
  floatEntry = nullptr;
  intEntry = nullptr;
  boolEntry = nullptr;
  stringEntry = stringEntryPoint;
  hasRange = false;
  isRegistered = false;
}

const char* Receive::getName() { return name; }
const char* Receive::getType() { return type; }
const char* Receive::getPin() { return pin; }
float* Receive::getEntryPoint() { return floatEntry; }

bool Receive::registerCapability() {
  DynamicJsonDocument receiveDoc(200);
  receiveDoc["deviceId"] = _config->getDeviceID();
  receiveDoc["type"] = "capabilities_control";
  receiveDoc["controlName"] = name;

  JsonObject receive = receiveDoc.createNestedObject("control");
  receive["type"] = type;
  receive["pin"] = pin;

  if ((strcmp(type, "float") == 0 || strcmp(type, "integer") == 0) && hasRange) {
    receive["min"] = min;
    receive["max"] = max;
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

bool Receive::handleCommand(const JsonVariantConst& value, JsonObject& result) {
  // Prefer explicit capability type string, then pointer kind
  const bool asString = (strcmp(type, "string") == 0) || (kind == KIND_STRING);
  const bool asBool = (strcmp(type, "boolean") == 0) || (kind == KIND_BOOL);
  const bool asInt = (strcmp(type, "integer") == 0) || (kind == KIND_INT);
  const bool asFloat = (strcmp(type, "float") == 0) || (kind == KIND_FLOAT);

  if (asString) {
    String strValue = value.as<String>();
    if (stringEntry) {
      *stringEntry = strValue;
    }
    if (floatEntry) {
      *floatEntry = 1.0f;
    }
    Serial.print("String control ");
    Serial.print(pin);
    Serial.print(" = ");
    Serial.println(strValue);
    result["newState"] = strValue;
    return stringEntry != nullptr || floatEntry != nullptr;
  }

  if (asBool) {
    bool boolValue = false;
    if (value.is<bool>()) {
      boolValue = value.as<bool>();
    } else {
      String strValue = value.as<String>();
      strValue.toLowerCase();
      boolValue = (strValue == "on" || strValue == "true" || strValue == "1");
    }
    if (boolEntry) {
      *boolEntry = boolValue;
    } else if (floatEntry) {
      *floatEntry = boolValue ? 1.0f : 0.0f;
    } else {
      return false;
    }
    result["newState"] = boolValue;
    return true;
  }

  if (asInt) {
    int intValue = value.as<int>();
    if (intEntry) {
      *intEntry = intValue;
    } else if (floatEntry) {
      *floatEntry = (float)intValue;
    } else {
      return false;
    }
    result["newState"] = intValue;
    return true;
  }

  if (asFloat) {
    if (!floatEntry) {
      return false;
    }
    float floatValue = value.as<float>();
    *floatEntry = floatValue;
    result["newState"] = floatValue;
    return true;
  }

  return false;
}
