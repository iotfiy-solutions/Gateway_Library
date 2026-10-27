#include "DeviceConfig.h"

// Default constructor
DeviceConfig::DeviceConfig() : _mqtt_port(1883) {}

static int base64Value(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;
  return -1;
}

static bool isPrintableDeviceId(const String& s) {
  if (s.length() == 0) {
    return false;
  }
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c < 33 || c > 126) {
      return false;
    }
  }
  return true;
}

String DeviceConfig::resolveDeviceId(const char* value) {
  if (value == nullptr || value[0] == '\0') {
    return String("");
  }

  String input = value;
  input.trim();

  // Keep only Base64 alphabet + padding (reject plain ids with '_' / '-' etc.)
  String b64;
  b64.reserve(input.length());
  bool hadPadding = false;
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      continue;
    }
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '+' || c == '/') {
      b64 += c;
    } else if (c == '=') {
      b64 += c;
      hadPadding = true;
    } else {
      return String(value);
    }
  }

  // Plain short ids (e.g. "DB7KY8") are often valid Base64 alphabet but not
  // multiple of 4 — leave them unchanged. Real Base64 is padded or length % 4 == 0.
  if (b64.length() < 4) {
    return String(value);
  }
  if (!hadPadding && (b64.length() % 4) != 0) {
    return String(value);
  }
  if (!hadPadding && b64.length() < 8) {
    return String(value);
  }

  while (b64.length() % 4 != 0) {
    b64 += '=';
  }

  String decoded;
  decoded.reserve((b64.length() / 4) * 3);

  for (unsigned int i = 0; i + 3 < b64.length(); i += 4) {
    int v0 = base64Value(b64.charAt(i));
    int v1 = base64Value(b64.charAt(i + 1));
    char c2 = b64.charAt(i + 2);
    char c3 = b64.charAt(i + 3);
    int v2 = (c2 == '=') ? 0 : base64Value(c2);
    int v3 = (c3 == '=') ? 0 : base64Value(c3);

    if (v0 < 0 || v1 < 0 || (c2 != '=' && v2 < 0) || (c3 != '=' && v3 < 0)) {
      return String(value);
    }

    decoded += (char)((v0 << 2) | (v1 >> 4));
    if (c2 != '=') {
      decoded += (char)(((v1 & 0x0F) << 4) | (v2 >> 2));
    }
    if (c3 != '=') {
      decoded += (char)(((v2 & 0x03) << 6) | v3);
    }
  }

  if (!isPrintableDeviceId(decoded)) {
    return String(value);
  }

  return decoded;
}

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

DeviceConfig& DeviceConfig::deviceId(const char* value) {
  String resolved = resolveDeviceId(value);
  _device_id = resolved;

  if (value && resolved.length() > 0 && resolved != value) {
    Serial.print("Device ID decoded from Base64: ");
    Serial.println(_device_id);
  }

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
         !_mqtt_server.isEmpty() && !_device_id.isEmpty();
}
