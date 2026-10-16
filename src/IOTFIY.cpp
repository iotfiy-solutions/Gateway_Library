#include "IOTFIY.h"

// Constructor
IOTFIYManager::IOTFIYManager() : _sendCount(0), _receiveCount(0), _mqttClient(_wifiClient), _lwtEnabled(false) {}

// Config access
DeviceConfig& IOTFIYManager::config() { 
  return _config; 
}

void IOTFIYManager::enableLWT(bool enabled) {
  _lwtEnabled = enabled;
  Serial.print("MQTT LWT ");
  Serial.println(enabled ? "ENABLED (offline on disconnect)" : "disabled");
}

// Initialize
void IOTFIYManager::begin() {
  // Keep PubSubClient buffer aligned with MQTT_MAX_PACKET_SIZE (ESP8266 + ESP32).
  // Older PubSubClient builds use the compile-time define; newer ones also honor setBufferSize.
  _mqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);
  _mqttClient.setServer(_config.getMQTTServer(), _config.getMQTTPort());
  _mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->handleCallback(topic, payload, length);
  });
}

// Register all capabilities
void IOTFIYManager::registerAllCapabilities() {
  // Send start message
  DynamicJsonDocument startDoc(128);
  startDoc["deviceId"] = _config.getDeviceID();
  startDoc["event"] = "capabilities_registration_start";
  startDoc["total_controls"] = _receiveCount;
  startDoc["total_sensors"] = _sendCount;
  String startPayload;
  serializeJson(startDoc, startPayload);
  _mqttClient.publish(_config.getStatusTopic().c_str(), startPayload.c_str());
  
  // Register receives and sends
  for (int i = 0; i < _receiveCount; i++) {
    _receives[i]->registerCapability();
  }
  
  for (int i = 0; i < _sendCount; i++) {
    _sends[i]->registerCapability();
  }
  
  // Send completion message
  DynamicJsonDocument endDoc(128);
  endDoc["deviceId"] = _config.getDeviceID();
  endDoc["event"] = "capabilities_registration_complete";
  endDoc["controls_registered"] = _receiveCount;
  endDoc["sensors_registered"] = _sendCount;
  String endPayload;
  serializeJson(endDoc, endPayload);
  _mqttClient.publish(_config.getStatusTopic().c_str(), endPayload.c_str());
}

// Publish all send data
void IOTFIYManager::launch() {
  for (int i = 0; i < _sendCount; i++) {
    _sends[i]->publishData();
  }
}

bool IOTFIYManager::publishStringOutput(const char* name, const char* pin, const String& value) {
  if (!_mqttClient.connected()) {
    return false;
  }

  DynamicJsonDocument capDoc(256);
  capDoc["deviceId"] = _config.getDeviceID();
  capDoc["type"] = "capabilities_output";
  capDoc["outputName"] = name;
  JsonObject output = capDoc.createNestedObject("output");
  output["type"] = "string";
  output["pin"] = pin;
  String capPayload;
  serializeJson(capDoc, capPayload);
  String capTopic = String("devices/") + _config.getDeviceID() + "/capabilities_output/" + name;
  _mqttClient.publish(capTopic.c_str(), capPayload.c_str(), true);

  DynamicJsonDocument dataDoc(256);
  dataDoc["type"] = "output";
  dataDoc["deviceId"] = _config.getDeviceID();
  JsonObject data = dataDoc.createNestedObject("data");
  data[name] = value;
  data[pin] = value;
  String dataPayload;
  serializeJson(dataDoc, dataPayload);
  Serial.print("String output ");
  Serial.print(pin);
  Serial.print("=");
  Serial.println(value);
  return _mqttClient.publish(_config.getDataTopic().c_str(), dataPayload.c_str());
}

// Process incoming commands
void IOTFIYManager::handleCallback(char* topic, byte* payload, unsigned int length) {
  // Fixed + static buffer for ESP8266 and ESP32 (no VLA, off the stack).
  // MQTT callback is not re-entrant, so a shared static buffer is safe.
  static char message[MQTT_MAX_PACKET_SIZE + 1];
  const unsigned int kMaxPayload = MQTT_MAX_PACKET_SIZE;

  if (length > kMaxPayload) {
    Serial.print("MQTT payload too large (");
    Serial.print(length);
    Serial.print(" > ");
    Serial.print(kMaxPayload);
    Serial.println("), truncated");
    length = kMaxPayload;
  }

  memcpy(message, payload, length);
  message[length] = '\0';

  // Log received message for debugging
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  // Parse the JSON command
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Process command if it's the right topic
  if (String(topic) == _config.getCommandsTopic()) {
    JsonVariantConst value;
    const char* controlName = nullptr;
    const char* pinName = nullptr;
    
    // Check if this is a nested format with 'data' field
    if (doc.containsKey("data") && doc.containsKey("type")) {
      // Extract from nested structure
      JsonVariantConst data = doc["data"];
      value = data["value"];
      
      if (data.containsKey("control")) {
        controlName = data["control"];
      } else if (data.containsKey("pin")) {
        pinName = data["pin"];
      }
    } else {
      // Original flat format
      value = doc["value"];
      
      if (doc.containsKey("control")) {
        controlName = doc["control"];
      } else if (doc.containsKey("pin")) {
        pinName = doc["pin"];
      }
    }
    
    // Find the target receive by control name or pin
    Receive* targetReceive = nullptr;
    
    // Try to find by control name if available
    if (controlName) {
      for (int i = 0; i < _receiveCount; i++) {
        if (strcmp(_receives[i]->getName(), controlName) == 0) {
          targetReceive = _receives[i];
          break;
        }
      }
    }
    // Try to find by pin if control name didn't match or isn't available
    else if (pinName && !targetReceive) {
      for (int i = 0; i < _receiveCount; i++) {
        if (strcmp(_receives[i]->getPin(), pinName) == 0) {
          targetReceive = _receives[i];
          break;
        }
      }
    }
    
    // Process the command if receive was found
    if (targetReceive) {
      DynamicJsonDocument responseDoc(256);
      JsonObject result = responseDoc.createNestedObject("result");
      bool success = targetReceive->handleCommand(value, result);
      
      responseDoc["status"] = success ? "success" : "error";
      responseDoc["control"] = targetReceive->getName();
      
      String responsePayload;
      serializeJson(responseDoc, responsePayload);
      
      // Send response
      _mqttClient.publish(_config.getCommandResponseTopic().c_str(), responsePayload.c_str());
    } else {
      // Error handling when no matching control found
      Serial.println("ERROR: No matching control or pin found");
      
      DynamicJsonDocument errorDoc(256);
      errorDoc["status"] = "error";
      errorDoc["message"] = "No matching control or pin found";
      if (controlName) errorDoc["control"] = controlName;
      if (pinName) errorDoc["pin"] = pinName;
      
      String errorPayload;
      serializeJson(errorDoc, errorPayload);
      
      // Send error response
      _mqttClient.publish(_config.getCommandResponseTopic().c_str(), errorPayload.c_str());
    }
  }
}

bool IOTFIYManager::reconnectMQTT() {
  int attempts = 0;
  while (!_mqttClient.connected() && attempts < 5) {
    attempts++;
    Serial.print("MQTT connection attempt ");
    Serial.print(attempts);
    Serial.print("... ");

    // Build LWT payload (broker publishes this if device drops unexpectedly)
    String willTopic = _config.getStatusTopic();
    DynamicJsonDocument willDoc(128);
    willDoc["status"] = "offline";
    willDoc["deviceId"] = _config.getDeviceID();
    String willPayload;
    serializeJson(willDoc, willPayload);
    
    bool connected;
    if (_lwtEnabled) {
      if (_config.getMQTTUsername()[0] != '\0') {
        connected = _mqttClient.connect(
          _config.getMQTTClientID(),
          _config.getMQTTUsername(),
          _config.getMQTTPassword(),
          willTopic.c_str(),
          1,    // will QoS
          true, // will retain
          willPayload.c_str()
        );
      } else {
        connected = _mqttClient.connect(
          _config.getMQTTClientID(),
          willTopic.c_str(),
          1,
          true,
          willPayload.c_str()
        );
      }
    } else if (_config.getMQTTUsername()[0] != '\0') {
      connected = _mqttClient.connect(
        _config.getMQTTClientID(),
        _config.getMQTTUsername(),
        _config.getMQTTPassword()
      );
    } else {
      connected = _mqttClient.connect(_config.getMQTTClientID());
    }

    if (connected) {
      Serial.println("connected!");
      if (_lwtEnabled) {
        Serial.print("LWT registered on ");
        Serial.println(willTopic);
      }
      _mqttClient.subscribe(_config.getCommandsTopic().c_str());

      // Send online status
      DynamicJsonDocument statusDoc(128);
      statusDoc["status"] = "online";
      statusDoc["deviceId"] = _config.getDeviceID();
      String statusPayload;
      serializeJson(statusDoc, statusPayload);
      _mqttClient.publish(_config.getStatusTopic().c_str(), statusPayload.c_str(), true);
      
      // Register capabilities
      registerAllCapabilities();
      return true;
    } else {
      int state = _mqttClient.state();
      Serial.print("failed, state=");
      Serial.print(state);
      Serial.print(" (");
      // Print human-readable error
      switch(state) {
        case -4: Serial.print("MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial.print("MQTT_CONNECTION_LOST"); break;
        case -2: Serial.print("MQTT_CONNECT_FAILED"); break;
        case -1: Serial.print("MQTT_DISCONNECTED"); break;
        case 1: Serial.print("MQTT_CONNECT_BAD_PROTOCOL"); break;
        case 2: Serial.print("MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case 3: Serial.print("MQTT_CONNECT_UNAVAILABLE"); break;
        case 4: Serial.print("MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case 5: Serial.print("MQTT_CONNECT_UNAUTHORIZED"); break;
      }
      Serial.println(")");
      
      if (attempts < 5) {
        delay(5000);
      }
    }
  }
  return false;
}

bool IOTFIYManager::checkAndReconnectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println("WiFi connection lost. Reconnecting...");

  // ESP32 error "sta is connecting, cannot set config" happens if WiFi.begin()
  // is called again while a previous connect is still in progress (common when
  // loop() retries immediately). Clear the old attempt first on both boards.
  WiFi.disconnect();
#if defined(ESP8266)
  delay(100);
  WiFi.mode(WIFI_STA);
#else
  delay(200);
  WiFi.mode(WIFI_STA);
#endif

  WiFi.begin(_config.getSSID(), _config.getPassword());

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi reconnected. IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("WiFi reconnection failed!");
  return false;
}

bool IOTFIYManager::isMQTTConnected() {
  return _mqttClient.connected();
}

void IOTFIYManager::processMQTTMessages() {
  _mqttClient.loop();
}

// Direct configuration method
void IOTFIYManager::config(const char* ssid, const char* password, const char* mqtt_server, 
                          const char* mqtt_client_id, const char* device_id, int mqtt_port,
                          const char* mqtt_username, const char* mqtt_password) {
  _config.ssid(ssid)
         .password(password)
         .server(mqtt_server)
         .port(mqtt_port)
         .clientId(mqtt_client_id)
         .deviceId(device_id)
         .mqttUsername(mqtt_username)
         .mqttPassword(mqtt_password);
}

// Create a single global instance
IOTFIYManager IOTFIY;

Send* IOTFIYManager::write(const char* name, const char* type, const char* pin, float* dataSource) {
  if (_sendCount >= MAX_SENDS) return nullptr;
  
  Send* send = new Send(name, type, pin, dataSource, &_mqttClient, &_config);
  _sends[_sendCount++] = send;
  return send;
}

Send* IOTFIYManager::write(const char* name, const char* type, const char* pin, 
                     float* dataSource, float min, float max) {
  if (_sendCount >= MAX_SENDS) return nullptr;
  
  Send* send = new Send(name, type, pin, dataSource, &_mqttClient, &_config, min, max);
  _sends[_sendCount++] = send;
  return send;
}

Receive* IOTFIYManager::read(const char* name, const char* type, const char* pin, float* entryPoint) {
  if (_receiveCount >= MAX_RECEIVES) return nullptr;
  
  Receive* receive = new Receive(name, type, pin, entryPoint, &_mqttClient, &_config);
  _receives[_receiveCount++] = receive;
  return receive;
}

Receive* IOTFIYManager::read(const char* name, const char* type, const char* pin, 
                   float* entryPoint, float min, float max) {
  if (_receiveCount >= MAX_RECEIVES) return nullptr;
  
  Receive* receive = new Receive(name, type, pin, entryPoint, &_mqttClient, &_config, min, max);
  _receives[_receiveCount++] = receive;
  return receive;
}

Receive* IOTFIYManager::read(const char* name, const char* type, const char* pin, String* entryPoint) {
  if (_receiveCount >= MAX_RECEIVES) return nullptr;

  Receive* receive = new Receive(name, type, pin, entryPoint, &_mqttClient, &_config);
  _receives[_receiveCount++] = receive;
  return receive;
}
