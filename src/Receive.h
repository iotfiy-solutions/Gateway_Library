#ifndef RECEIVE_H
#define RECEIVE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DeviceConfig.h"

class Receive {
private:
  // References to store client and config
  PubSubClient* _client;
  DeviceConfig* _config;
  
  const char* name;
  const char* type;
  const char* pin;
  float* entryPoint;  // Pointer to a variable that will receive the control value
  String* stringEntryPoint;  // For string controls (e.g. color hex on V4)
  bool hasRange;
  float min;
  float max;
  bool isRegistered;
  int defaultDelay = 50;
  
public:
  // Constructor for receives without range
  Receive(const char* name, const char* type, const char* pin, float* entryPoint,
          PubSubClient* client, DeviceConfig* config);
  
  // Constructor for receives with range
  Receive(const char* name, const char* type, const char* pin, float* entryPoint,
          PubSubClient* client, DeviceConfig* config, float min, float max);

  Receive(const char* name, const char* type, const char* pin, String* stringEntryPoint,
          PubSubClient* client, DeviceConfig* config);
  
  // Getters
  const char* getName();
  const char* getType();
  const char* getPin();
  float* getEntryPoint();
  
  // Register capability without description
  bool registerCapability();
  
  // Handle command (virtual for potential overriding in child classes)
  virtual bool handleCommand(const JsonVariantConst& value, JsonObject& result);
};

#endif // RECEIVE_H
