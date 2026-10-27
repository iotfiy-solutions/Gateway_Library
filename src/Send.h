#ifndef SEND_H
#define SEND_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DeviceConfig.h"

class Send {
public:
  enum ValueKind { KIND_FLOAT, KIND_INT, KIND_BOOL };

private:
  PubSubClient* _client;
  DeviceConfig* _config;

  const char* name;
  const char* type;
  const char* pin;
  ValueKind kind;
  float* floatExit;
  int* intExit;
  bool* boolExit;
  bool hasRange;
  float min;
  float max;
  bool isRegistered;
  int defaultDelay = 50;
  bool hasLastPublished = false;
  float lastPublishedValue = 0;

  float readValue() const;
  void writeValue(float value);

public:
  Send(const char* name, const char* type, const char* pin, float* exitPoint,
       PubSubClient* client, DeviceConfig* config);
  Send(const char* name, const char* type, const char* pin, float* exitPoint,
       PubSubClient* client, DeviceConfig* config, float min, float max);
  Send(const char* name, const char* type, const char* pin, int* exitPoint,
       PubSubClient* client, DeviceConfig* config);
  Send(const char* name, const char* type, const char* pin, int* exitPoint,
       PubSubClient* client, DeviceConfig* config, float min, float max);
  Send(const char* name, const char* type, const char* pin, bool* exitPoint,
       PubSubClient* client, DeviceConfig* config);

  const char* getName();
  const char* getType();
  const char* getPin();
  bool hasRangeValues();
  float getMin();
  float getMax();
  bool isCapabilityRegistered();
  float getCurrentValue();

  void setDelay(int ms);
  void resetPublishState();
  bool registerCapability();
  bool publishData();
};

#endif // SEND_H
