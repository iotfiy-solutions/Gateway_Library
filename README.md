# IOTFIY

Arduino library for connecting **ESP32** and **ESP8266** devices to the IOTFIY MQTT gateway. Register outputs (`write`) and controls (`read`), then call `launch()` to maintain the connection and exchange JSON over MQTT.

Other boards will not compile (`IOTFIY supports ESP8266 and ESP32 only`).

| | |
|---|---|
| **Version** | 1.0.1 (`library.properties`) |
| **Repository** | [iotfiy-solutions/Gateway_Library](https://github.com/iotfiy-solutions/Gateway_Library) |

---

## Features

- WiFi + MQTT connection, reconnect, and message processing inside `launch()`
- MQTT broker, authentication, and LWT are built into the library (not configured in the sketch)
- `write()` / `read()` — type inferred from the variable (`float*`, `int*`, `bool*`, `String*`)
- Optional capability **name** (if omitted, **pin** is used as the name)
- Optional min/max on `write()`: value **wraps** when it crosses the range
- Publishes `write()` values only when they **change**
- Automatic capability registration after MQTT connect
- Up to **10** outputs and **10** controls per device

---

## Dependencies

Install from Library Manager (or accept them when installing IOTFIY):

| Library | Purpose |
|---------|---------|
| [PubSubClient](https://www.arduino.cc/reference/en/libraries/pubsubclient/) | MQTT |
| [ArduinoJson](https://www.arduino.cc/reference/en/libraries/arduinojson/) | JSON |

Also install the ESP32 or ESP8266 board package for your hardware.

---

## Installation

### Arduino Library Manager

1. **Sketch → Include Library → Manage Libraries…**
2. Search for **IOTFIY**
3. Install

### Manual

Copy the library folder (the one that contains `library.properties` and `src/`) into your Arduino `libraries` folder, then restart the IDE.

---

## Typical flow

```text
IOTFIY.config(ssid, password, deviceId);
IOTFIY.begin();
IOTFIY.write(...);     // outputs
IOTFIY.read(...);      // controls
IOTFIY.launch();       // connect (Serial shows WiFi / MQTT status)

loop:
  update your variables
  IOTFIY.launch();     // maintain connection, process commands, publish changes
```

The sketch only supplies WiFi credentials and a unique `deviceId`. `deviceId` is also used as the MQTT client id.

---

## Quick start

Pattern used by [`examples/ESP8266_Basic/ESP8266_Basic.ino`](examples/ESP8266_Basic/ESP8266_Basic.ino):

```cpp
#include <IOTFIY.h>

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_ID     = "device_1";

float sensorValue  = 0;
float controlValue = 0;
unsigned long lastUpdateMs = 0;

void setup() {
  Serial.begin(115200);

  IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);
  IOTFIY.begin();

  IOTFIY.write("A0", &sensorValue);              // float* → type "float", name = "A0"
  IOTFIY.read("Ctrl", &controlValue, 0, 100);    // float* → type "float", name = "Ctrl"

  IOTFIY.launch();
}

void loop() {
  if (millis() - lastUpdateMs >= 3000) {
    lastUpdateMs = millis();
#if defined(ESP8266)
    sensorValue = (analogRead(A0) / 1023.0f) * 100.0f;
#else
    sensorValue = fmod((millis() / 1000.0f) * 3.0f, 100.0f);
#endif
  }

  IOTFIY.launch();
  delay(10);
}
```

**Serial (on status change / connect)**

- `WiFi: connected  IP: ...` or `WiFi: not connected`
- `MQTT: connected` or `MQTT: not connected`
- `MQTT connection attempt N... connected!`
- Capability lines such as `A0 output published: SUCCESS`
- `Send data published: SUCCESS` when a `write()` value changes
- Incoming commands: `Message arrived [devices/.../commands] ...`

Use **115200** baud in Serial Monitor.

---

## Configuration

### `void config(ssid, password, device_id)`

| Parameter | Meaning |
|-----------|---------|
| `ssid` / `password` | WiFi credentials |
| `device_id` | Topic prefix and MQTT client id (must be unique per device) |

`device_id` may be a **plain** id or a **Base64-encoded** id. Base64 is decoded inside the library before MQTT connect; plain ids (for example with `_`, or short unpadded strings) are used as-is. On successful decode, Serial prints `Device ID decoded from Base64: ...`.

MQTT host, port, and credentials are fixed inside the library for the IOTFIY gateway.

### Buffer sizes (`IOTFIY.h`)

| Macro | Default | Role |
|-------|---------|------|
| `MQTT_MAX_PACKET_SIZE` | 512 | PubSubClient buffer; larger incoming payloads are truncated |
| `TCP_BUFFER_SIZE` | 1024 | Defined for PubSubClient alignment |

---

## API reference

Global instance: `IOTFIY` (`IOTFIYManager`).

### `void begin()`

Configures the MQTT client (buffer, server, callback). Call after `config()`. Does not connect — call `launch()` after `write()` / `read()`.

### `void launch()`

Call once at the end of `setup()` and often in `loop()`:

1. Reconnect WiFi if needed (up to 20 × 500 ms)
2. Reconnect MQTT if needed (up to 5 tries, 5 s between failures; re-subscribe and re-register capabilities)
3. Process MQTT keepalives and incoming commands
4. Print WiFi / MQTT status to Serial when it changes
5. Publish each `write()` value **only if it changed**

### `bool isMQTTConnected()`

Returns whether the MQTT client is connected (optional, e.g. for a display).

### `Send* write(...)`

Registers an output (device → backend). **Type is inferred from the variable pointer.**

| Variable | Capability type |
|----------|-----------------|
| `float*` | `"float"` |
| `int*` | `"integer"` |
| `bool*` | `"boolean"` |

| Form | Notes |
|------|--------|
| `write(pin, &var)` | Name = pin; type from `&var` |
| `write(pin, &var, min, max)` | Same + wrap range (`float*` / `int*`) |
| `write(name, pin, &var)` | Explicit name |
| `write(name, pin, &var, min, max)` | Explicit name + wrap |
| `write(name, type, pin, float*)` | Explicit type override (legacy) |

- Max **10** outputs; returns `nullptr` if full
- With min/max: above max → min, below min → max (variable updated)

```cpp
float temp = 0;
int battery = 0;
bool relay = false;

IOTFIY.write("A0", &temp);
IOTFIY.write("V9", &battery, 0, 100);
IOTFIY.write("relay", "V8", &relay);
```

### `Receive* read(...)`

Registers a control (dashboard → device). **Type is inferred from the variable pointer.**

| Variable | Capability type |
|----------|-----------------|
| `float*` | `"float"` |
| `int*` | `"integer"` |
| `bool*` | `"boolean"` |
| `String*` | `"string"` |

| Form | Notes |
|------|--------|
| `read(pin, &var)` | Name = pin; type from `&var` |
| `read(pin, &var, min, max)` | Same + range in capability |
| `read(name, pin, &var)` | Explicit name |
| `read(name, type, pin, ...)` | Explicit type override (legacy) |

- Max **10** controls; returns `nullptr` if full

```cpp
float setpoint = 0;
String colorHex;

IOTFIY.read("Ctrl", &setpoint, 0, 100);
IOTFIY.read("V4", &colorHex);
```

### `bool publishStringOutput(name, pin, value)`

One-shot string output without a prior `write()`: publishes a retained string capability, then data on the data topic. Returns `false` if MQTT is not connected.

### `void registerAllCapabilities()`

Registers all `write` / `read` capabilities. Called automatically when MQTT connects inside `launch()`; you normally do not call it yourself.

### `DeviceConfig& config()`

Returns the internal `DeviceConfig` object (getters / chainable setters). Normal sketches use the 3-argument `config(ssid, password, device_id)`.

---

## Send and Receive objects

Returned by `write()` / `read()` if you need direct access.

### `Send`

| Method | Purpose |
|--------|---------|
| `getName()`, `getType()`, `getPin()` | Accessors |
| `hasRangeValues()`, `getMin()`, `getMax()` | Range |
| `getCurrentValue()` | Current `float` value |
| `isCapabilityRegistered()` | Capability publish succeeded |
| `setDelay(int ms)` | Delay after capability register (default 50 ms) |
| `registerCapability()` | Publish retained output capability |
| `publishData()` | Publish one output (skips unchanged values; applies wrap if ranged) |

### `Receive`

| Method | Purpose |
|--------|---------|
| `getName()`, `getType()`, `getPin()`, `getEntryPoint()` | Accessors |
| `registerCapability()` | Publish retained control capability |
| `handleCommand(value, result)` | Apply command; sets `result["newState"]` |

**`handleCommand` by `type`**

| type | Effect |
|------|--------|
| `float` | Writes into `float*` |
| `integer` | Writes int into `float*` |
| `boolean` | `"on"` / `"true"` / `"1"` → `1.0`, otherwise `0.0` |
| `string` | Writes into `String*` when registered that way |

Unknown types return `false`.

---

## MQTT topics and payloads

`{deviceId}` is your `device_id`.

| Topic | Direction | Role |
|-------|-----------|------|
| `devices/{deviceId}/data` | Device → broker | Output values |
| `devices/{deviceId}/status` | Device → broker | Online / offline, registration events |
| `devices/{deviceId}/commands` | Broker → device | Dashboard commands |
| `devices/{deviceId}/command-response` | Device → broker | Command result |
| `devices/{deviceId}/capabilities_output/{name}` | Device → broker | Output capability (retained) |
| `devices/{deviceId}/capabilities_control/{name}` | Device → broker | Control capability (retained) |

### Output data

When name and pin differ:

```json
{
  "type": "output",
  "deviceId": "device_1",
  "data": {
    "battery": 42.0,
    "V9": 42.0
  }
}
```

When name defaults to pin, the capability name and pin are the same string (e.g. `"V9"`).

### Output capability

```json
{
  "deviceId": "device_1",
  "type": "capabilities_output",
  "outputName": "A0",
  "output": { "type": "float", "pin": "A0" }
}
```

### Control capability

```json
{
  "deviceId": "device_1",
  "type": "capabilities_control",
  "controlName": "ControlValue",
  "control": { "type": "float", "pin": "Ctrl", "min": 0, "max": 100 }
}
```

`min` / `max` are included only for `"float"` or `"integer"` when a range was given to `read()`.

### Incoming commands

**Flat**

```json
{ "value": 25, "control": "ControlValue" }
```

or `"pin": "Ctrl"` instead of `"control"`.

**Nested** (when both `"type"` and `"data"` are present)

```json
{
  "type": "...",
  "data": {
    "value": 25,
    "control": "ControlValue"
  }
}
```

Matching: control **name** first, then **pin**.

### Command response

```json
{
  "status": "success",
  "control": "ControlValue",
  "result": { "newState": 25 }
}
```

```json
{
  "status": "error",
  "message": "No matching control or pin found"
}
```

### Status and registration

Retained on connect:

```json
{ "status": "online", "deviceId": "device_1" }
```

During registration:

```json
{
  "deviceId": "device_1",
  "event": "capabilities_registration_start",
  "total_controls": 1,
  "total_sensors": 1
}
```

```json
{
  "deviceId": "device_1",
  "event": "capabilities_registration_complete",
  "controls_registered": 1,
  "sensors_registered": 1
}
```

---

## Last Will (LWT)

LWT is always enabled (not configurable from the sketch).

| | |
|---|---|
| Topic | `devices/{deviceId}/status` |
| Will payload | `{"status":"offline","deviceId":"..."}` |
| QoS / retain | QoS 1, retained |
| After connect | Retained `{"status":"online",...}` on the same topic |

Unexpected disconnect → broker publishes the will (**offline**).

---

## Reconnection

Handled inside `IOTFIY.launch()`:

| Layer | Behaviour |
|-------|-----------|
| WiFi | Up to 20 tries, 500 ms apart |
| MQTT | Up to 5 tries, 5 s between failures; then re-subscribe and re-register capabilities |
| Messages | Processed on every `launch()` |
| Publish | Only when a `write()` value changed |

MQTT failure states on Serial include: timeout, connection lost, connect failed, disconnected, bad protocol, bad client id, unavailable, bad credentials, unauthorized.

---

## Examples

| Example | Path | Notes |
|---------|------|--------|
| ESP8266 basic | [`examples/ESP8266_Basic/`](examples/ESP8266_Basic/ESP8266_Basic.ino) | Sensor update + float control; same pattern on ESP32 |
| M5Stack basic | [`examples/M5Stack_Basic/`](examples/M5Stack_Basic/M5Stack_Basic.ino) | ESP32 + **M5Unified**; buttons A/B/C |

### Buttons

```cpp
int btnA = 0, btnB = 0, btnC = 0;

IOTFIY.write("BtnA", &btnA);
IOTFIY.write("BtnB", &btnB);
IOTFIY.write("BtnC", &btnC);

btnB = 0; btnC = 0;
btnA = 1;
IOTFIY.launch();
```

### GPS-style floats

```cpp
float latitude = 24.8607;
float longitude = 67.0011;

IOTFIY.write("Lat", &latitude);
IOTFIY.write("Lng", &longitude);
IOTFIY.launch();
```

### String control

```cpp
String colorHex;
IOTFIY.read("V4", &colorHex);
```

### String output without `write()`

```cpp
IOTFIY.publishStringOutput("StatusText", "Msg", "ready");
```

---

## ESP32 vs ESP8266

| Topic | Detail |
|-------|--------|
| WiFi header | Included automatically (`WiFi.h` or `ESP8266WiFi.h`) |
| Sketch | Include the board WiFi header only if you use `WiFi.*` APIs yourself |
| Analog demo | ESP8266 example uses `A0`; ESP32 has no single standard `A0` |
| M5Stack example | ESP32 + M5 hardware |
| Limits | Same API and topics; max 10 sends / 10 receives |

---

## Troubleshooting

| Symptom | Check |
|---------|--------|
| Unsupported board | Use ESP32 or ESP8266 |
| WiFi fails | SSID/password; Serial `WiFi: not connected` |
| MQTT connect failed | Network path to the gateway; Serial MQTT state text |
| No dashboard updates | Change the `float` then call `launch()` |
| Commands ignored | Call `launch()` often; match `read()` name or pin to the command JSON |
| Payload truncated | Keep under `MQTT_MAX_PACKET_SIZE` (512) |
| `write` / `read` returns `nullptr` | Already at 10 outputs or 10 controls |
| Missing dependencies | Install PubSubClient and ArduinoJson |

---

## Project layout

```text
library.properties
keywords.txt
README.md
src/
  IOTFIY.h / IOTFIY.cpp
  DeviceConfig.h / DeviceConfig.cpp
  Send.h / Send.cpp
  Receive.h / Receive.cpp
examples/
  ESP8266_Basic/
  M5Stack_Basic/
```

---

## Support

Maintainer: **iotfiy.solution@gmail.com** (see `library.properties`).

Issues and updates: [GitHub repository](https://github.com/iotfiy-solutions/Gateway_Library).
