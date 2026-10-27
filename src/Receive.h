#ifndef RECEIVE_H
#define RECEIVE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DeviceConfig.h"

class Receive {
public:
  enum ValueKind { KIND_FLOAT, KIND_INT, KIND_BOOL, KIND_STRING };

private:
  PubSubClient* _client;
  DeviceConfig* _config;

  const char* name;
  const char* type;
  const char* pin;
  ValueKind kind;
  float* floatEntry;
  int* intEntry;
  bool* boolEntry;
  String* stringEntry;
  bool hasRange;
  float min;
  float max;
  bool isRegistered;
  int defaultDelay = 50;

public:
  Receive(const char* name, const char* type, const char* pin, float* entryPoint,
          PubSubClient* client, DeviceConfig* config);
  Receive(const char* name, const char* type, const char* pin, float* entryPoint,
          PubSubClient* client, DeviceConfig* config, float min, float max);
  Receive(const char* name, const char* type, const char* pin, int* entryPoint,
          PubSubClient* client, DeviceConfig* config);
  Receive(const char* name, const char* type, const char* pin, int* entryPoint,
          PubSubClient* client, DeviceConfig* config, float min, float max);
  Receive(const char* name, const char* type, const char* pin, bool* entryPoint,
          PubSubClient* client, DeviceConfig* config);
  Receive(const char* name, const char* type, const char* pin, String* stringEntryPoint,
          PubSubClient* client, DeviceConfig* config);

  const char* getName();
  const char* getType();
  const char* getPin();
  float* getEntryPoint();

  bool registerCapability();
  virtual bool handleCommand(const JsonVariantConst& value, JsonObject& result);
};

#endif // RECEIVE_H
