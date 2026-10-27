#include "Send.h"

Send::Send(const char* name, const char* type, const char* pin, float* exitPoint,
           PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_FLOAT;
  floatExit = exitPoint;
  intExit = nullptr;
  boolExit = nullptr;
  hasRange = false;
  isRegistered = false;
}

Send::Send(const char* name, const char* type, const char* pin, float* exitPoint,
           PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_FLOAT;
  floatExit = exitPoint;
  intExit = nullptr;
  boolExit = nullptr;
  hasRange = true;
  this->min = min;
  this->max = max;
  isRegistered = false;
}

Send::Send(const char* name, const char* type, const char* pin, int* exitPoint,
           PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_INT;
  floatExit = nullptr;
  intExit = exitPoint;
  boolExit = nullptr;
  hasRange = false;
  isRegistered = false;
}

Send::Send(const char* name, const char* type, const char* pin, int* exitPoint,
           PubSubClient* client, DeviceConfig* config, float min, float max) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_INT;
  floatExit = nullptr;
  intExit = exitPoint;
  boolExit = nullptr;
  hasRange = true;
  this->min = min;
  this->max = max;
  isRegistered = false;
}

Send::Send(const char* name, const char* type, const char* pin, bool* exitPoint,
           PubSubClient* client, DeviceConfig* config) {
  _client = client;
  _config = config;
  this->name = name;
  this->type = type;
  this->pin = pin;
  kind = KIND_BOOL;
  floatExit = nullptr;
  intExit = nullptr;
  boolExit = exitPoint;
  hasRange = false;
  isRegistered = false;
}

float Send::readValue() const {
  if (kind == KIND_INT && intExit) {
    return (float)(*intExit);
  }
  if (kind == KIND_BOOL && boolExit) {
    return (*boolExit) ? 1.0f : 0.0f;
  }
  if (floatExit) {
    return *floatExit;
  }
  return 0.0f;
}

void Send::writeValue(float value) {
  if (kind == KIND_INT && intExit) {
    *intExit = (int)value;
  } else if (kind == KIND_BOOL && boolExit) {
    *boolExit = value > 0.5f;
  } else if (floatExit) {
    *floatExit = value;
  }
}

const char* Send::getName() { return name; }
const char* Send::getType() { return type; }
const char* Send::getPin() { return pin; }
bool Send::hasRangeValues() { return hasRange; }
float Send::getMin() { return min; }
float Send::getMax() { return max; }
bool Send::isCapabilityRegistered() { return isRegistered; }
float Send::getCurrentValue() { return readValue(); }

void Send::setDelay(int ms) { defaultDelay = ms; }

void Send::resetPublishState() {
  hasLastPublished = false;
  isRegistered = false;
}

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
  _client->loop();

  Serial.print(name);
  Serial.print(" output published: ");
  Serial.println(published ? "SUCCESS" : "FAILED");

  if (published) {
    isRegistered = true;
  }

  delay(defaultDelay);
  return published;
}

bool Send::publishData() {
  float value = readValue();

  if (hasRange && max > min) {
    if (value > max) {
      value = min;
      writeValue(value);
    } else if (value < min) {
      value = max;
      writeValue(value);
    }
  }

  if (hasLastPublished && value == lastPublishedValue) {
    return true;
  }

  if (!isRegistered) {
    registerCapability();
  }

  DynamicJsonDocument dataDoc(256);
  dataDoc["type"] = "output";
  dataDoc["deviceId"] = _config->getDeviceID();
  JsonObject data = dataDoc.createNestedObject("data");

  // Backend expects numeric values (0/1 for boolean, ints for integer)
  const bool asBool = (kind == KIND_BOOL) || (strcmp(type, "boolean") == 0);
  const bool asInt = (kind == KIND_INT) || (strcmp(type, "integer") == 0);

  if (asBool) {
    int iv = value > 0.5f ? 1 : 0;
    data[name] = iv;
    if (pin && strlen(pin) > 0) {
      data[pin] = iv;
    }
  } else if (asInt) {
    int iv = (int)value;
    data[name] = iv;
    if (pin && strlen(pin) > 0) {
      data[pin] = iv;
    }
  } else {
    data[name] = value;
    if (pin && strlen(pin) > 0) {
      data[pin] = value;
    }
  }

  String dataPayload;
  serializeJson(dataDoc, dataPayload);
  String topic = _config->getDataTopic();

  Serial.print("Send data ");
  Serial.print(name);
  Serial.print(" -> ");
  Serial.print(topic);
  Serial.print(" (");
  Serial.print(dataPayload.length());
  Serial.println(" bytes)");

  bool published = _client->publish(topic.c_str(), dataPayload.c_str());
  _client->loop();
  if (!published) {
    delay(20);
    published = _client->publish(topic.c_str(), dataPayload.c_str());
    _client->loop();
  }

  Serial.print("Send data published: ");
  Serial.println(published ? "SUCCESS" : "FAILED");

  if (published) {
    lastPublishedValue = value;
    hasLastPublished = true;
  }

  return published;
}
