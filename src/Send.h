#ifndef SEND_H
#define SEND_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DeviceConfig.h"

class Send {
private:
  // References to store client and config
  PubSubClient* _client;
  DeviceConfig* _config;
  
  const char* name;
  const char* type;
  const char* pin;
  float* exitPoint;  // Pointer to a float variable that contains the data
  bool hasRange;
  float min;
  float max;
  bool isRegistered;
  int defaultDelay = 50;
  
public:
  // Constructor for sends without range
  Send(const char* name, const char* type, const char* pin, float* exitPoint,
         PubSubClient* client, DeviceConfig* config);
  
  // Constructor with range
  Send(const char* name, const char* type, const char* pin, float* exitPoint,
         PubSubClient* client, DeviceConfig* config, float min, float max);
  
  // Getters
  const char* getName();
  const char* getType();
  const char* getPin();
  bool hasRangeValues();
  float getMin();
  float getMax();
  bool isCapabilityRegistered();
  float getCurrentValue();
  
  // Setters
  void setDelay(int ms);
  
  // Register capability similar to v3.3.3
  bool registerCapability();
  
  // Publish data for individual send
  bool publishData();
};

#endif // SEND_H