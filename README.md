# IOTFIY

Arduino library for connecting ESP32 and ESP8266 devices to an IOTFIY MQTT gateway. You register outputs (sensors / buttons) and controls (dashboard → device), then publish and receive JSON over MQTT.

Works on **ESP32** and **ESP8266** only. Other boards will fail to compile (`IOTFIY supports ESP8266 and ESP32 only`).

**Version:** 1.0.1 (see `library.properties`)

**Repository:** [iotfiy-solutions/Gateway_Library](https://github.com/iotfiy-solutions/Gateway_Library)

---

## Features

- WiFi connect / reconnect helpers
- MQTT connect with optional username/password
- Optional Last Will (LWT) for online / offline status
- `write()` — send pin/sensor values to the backend
- `read()` — receive dashboard commands into your variables
- Automatic capability registration after MQTT connect
- Up to **10** outputs and **10** controls per device

---

## Dependencies

Install these from Library Manager (or let the IDE offer them when you install IOTFIY):

| Library | Used for |
|---------|----------|
| [PubSubClient](https://www.arduino.cc/reference/en/libraries/pubsubclient/) | MQTT |
| [ArduinoJson](https://www.arduino.cc/reference/en/libraries/arduinojson/) | JSON payloads |

Also install the board package for your hardware (ESP32 or ESP8266).

---

## Installation

### Arduino Library Manager

1. Open **Sketch → Include Library → Manage Libraries…**
2. Search for **IOTFIY**
3. Install (dependencies may be offered automatically)

### Manual / GitHub

Clone or download [Gateway_Library](https://github.com/iotfiy-solutions/Gateway_Library) into your Arduino `libraries` folder, then restart the IDE.

---

## Typical flow

```text
IOTFIY.config(...)           // WiFi + MQTT + device id
IOTFIY.enableLWT(true);      // optional
IOTFIY.checkAndReconnectWiFi();
IOTFIY.begin();              // MQTT client setup
IOTFIY.write(...);           // register outputs (float*)
IOTFIY.read(...);            // register controls
IOTFIY.reconnectMQTT();      // connect, subscribe, register capabilities

loop:
  checkAndReconnectWiFi()
  reconnectMQTT() if needed
  processMQTTMessages()      // must call often
  update your float variables
  IOTFIY.launch()            // publish all write() values
```

Use a **hostname or IP** for the MQTT server (for example `192.168.1.10` or `broker.example.com`). Do **not** prefix with `mqtt://` — PubSubClient expects a host string.

---

## Quick start (ESP8266 / ESP32)

This matches the idea behind [`examples/ESP8266_Basic/ESP8266_Basic.ino`](examples/ESP8266_Basic/ESP8266_Basic.ino).

```cpp
#include <IOTFIY.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

const char* WIFI_SSID      = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER    = "YOUR_MQTT_HOST";   // IP or hostname only
const char* MQTT_CLIENT_ID = "esp_client_1";
const char* DEVICE_ID      = "device_1";
const int   MQTT_PORT      = 1883;

float sensorValue  = 0;
float controlValue = 0;

unsigned long lastPublishMs = 0;

void setup() {
  Serial.begin(115200);

  IOTFIY.config(
    WIFI_SSID,
    WIFI_PASSWORD,
    MQTT_SERVER,
    MQTT_CLIENT_ID,
    DEVICE_ID,
    MQTT_PORT
  );
  // Optional auth:
  // IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_CLIENT_ID,
  //               DEVICE_ID, MQTT_PORT, "mqtt_user", "mqtt_pass");

  IOTFIY.enableLWT(true);

  IOTFIY.checkAndReconnectWiFi();
  IOTFIY.begin();

  // Device → dashboard
  IOTFIY.write("SensorValue", "float", "A0", &sensorValue);

  // Dashboard → device (optional min/max)
  IOTFIY.read("ControlValue", "float", "Ctrl", &controlValue, 0, 100);

  IOTFIY.reconnectMQTT();
}

void loop() {
  IOTFIY.checkAndReconnectWiFi();

  if (!IOTFIY.isMQTTConnected()) {
    IOTFIY.reconnectMQTT();
  }

  IOTFIY.processMQTTMessages();

  if (millis() - lastPublishMs >= 3000) {
    lastPublishMs = millis();
#if defined(ESP8266)
    sensorValue = (analogRead(A0) / 1023.0f) * 100.0f;
#else
    sensorValue = fmod((millis() / 1000.0f) * 3.0f, 100.0f);
#endif
    IOTFIY.launch();
  }

  delay(10);
}
```

**What you should see on Serial**

- WiFi reconnect / IP when connected  
- `MQTT connection attempt N... connected!`  
- Capability lines like `SensorValue output published: SUCCESS`  
- On `launch()`: `Send data published: SUCCESS`  
- Incoming commands: `Message arrived [devices/.../commands] ...`

---

## Configuration

### `IOTFIY.config(...)`

```cpp
void config(
  const char* ssid,
  const char* password,
  const char* mqtt_server,
  const char* mqtt_client_id,
  const char* device_id,
  int mqtt_port = 1883,
  const char* mqtt_username = "",
  const char* mqtt_password = ""
);
```

| Parameter | Meaning |
|-----------|---------|
| `ssid` / `password` | WiFi credentials |
| `mqtt_server` | Broker host or IP (no `mqtt://`) |
| `mqtt_client_id` | MQTT client id (unique per device) |
| `device_id` | Used in all topic paths |
| `mqtt_port` | Default **1883** |
| `mqtt_username` / `mqtt_password` | Optional; empty string = no auth |

### Chainable config (advanced)

`IOTFIY.config()` with **no arguments** returns `DeviceConfig&`, so you can set fields one by one:

```cpp
IOTFIY.config()
  .ssid("MyWifi")
  .password("secret")
  .server("192.168.1.10")
  .port(1883)
  .clientId("client_1")
  .deviceId("device_1")
  .mqttUsername("user")
  .mqttPassword("pass");
```

`DeviceConfig::isValid()` is true when SSID, password, MQTT server, client id, and device id are all non-empty. The library does not call `isValid()` automatically inside `begin()`.

### Buffer sizes

Defined in `IOTFIY.h` if not already set:

| Macro | Default | Role |
|-------|---------|------|
| `MQTT_MAX_PACKET_SIZE` | 512 | PubSubClient buffer via `setBufferSize`; incoming payloads larger than this are truncated |
| `TCP_BUFFER_SIZE` | 1024 | Defined for alignment with common PubSubClient setups |

---

## API reference (`IOTFIY` global)

The global object is `IOTFIY` (`extern IOTFIYManager IOTFIY`).

### `void begin()`

Sets MQTT buffer size, broker address/port from config, and the message callback. Call after `config()` (and usually after WiFi is up). Does **not** connect MQTT by itself — call `reconnectMQTT()`.

### `void enableLWT(bool enabled = true)`

Enables or disables MQTT Last Will. When enabled, the next successful `reconnectMQTT()` registers a retained will on the status topic. Prints `MQTT LWT ENABLED...` or `disabled` to Serial.

### `bool checkAndReconnectWiFi()`

- If already connected → returns `true`  
- Otherwise disconnects, sets `WIFI_STA`, calls `WiFi.begin(ssid, password)`, waits up to **20 × 500 ms**  
- On success prints IP and returns `true`; otherwise returns `false`  
- ESP8266 waits 100 ms after disconnect; ESP32 waits 200 ms (same reconnect logic otherwise)

### `bool reconnectMQTT()`

Tries up to **5** times (5 s delay between failures):

1. Connects with or without auth / LWT (see LWT section)  
2. On success: subscribe to commands topic, publish retained **online** status, call `registerAllCapabilities()`, return `true`  
3. On failure: prints PubSubClient `state()` with a readable label  

Returns `false` if all attempts fail.

### `bool isMQTTConnected()`

Returns whether the internal PubSubClient is connected.

### `void processMQTTMessages()`

Calls `_mqttClient.loop()`. Call this frequently in `loop()` so keepalives and incoming commands work.

### `Send* write(name, type, pin, float* dataSource)`  
### `Send* write(name, type, pin, float* dataSource, float min, float max)`

Registers an **output** (device → backend). The pointer is read when you call `launch()` / `publishData()`.

- Max **10** writes; returns `nullptr` if full  
- Optional min/max: value is `constrain()`’d when publishing  
- `type` is sent in capability JSON as a string (examples use `"float"` or `"integer"`; storage is still `float*`)

### `Receive* read(name, type, pin, float* entryPoint)`  
### `Receive* read(name, type, pin, float* entryPoint, float min, float max)`  
### `Receive* read(name, type, pin, String* entryPoint)`

Registers a **control** (dashboard → device). Incoming command values are written into your variable.

- Max **10** reads; returns `nullptr` if full  
- For `"float"` / `"integer"`, optional min/max are included in capability registration  
- Handled command types in `Receive::handleCommand`: `"float"`, `"integer"`, `"boolean"`, `"string"` (see below)

### `void launch()`

Publishes **all** registered `write()` outputs (each calls `Send::publishData()`).

### `bool publishStringOutput(const char* name, const char* pin, const String& value)`

One-shot string output without a prior `write()`:

1. Retained capability on `devices/{deviceId}/capabilities_output/{name}` with `output.type = "string"`  
2. Data publish on the data topic  

Returns `false` if MQTT is not connected; otherwise returns the data publish result.

### `void registerAllCapabilities()`

Publishes registration start/complete on the status topic and registers every `read` / `write` capability. Called automatically from `reconnectMQTT()` on success; you normally do not need to call it yourself.

### `DeviceConfig& config()`

Returns the internal config object (chainable setters / getters).

---

## Send and Receive helpers

Objects returned by `write()` / `read()` are also usable directly.

### `Send`

| Method | Purpose |
|--------|---------|
| `getName()`, `getType()`, `getPin()` | Accessors |
| `hasRangeValues()`, `getMin()`, `getMax()` | Range info |
| `getCurrentValue()` | `*exitPoint` |
| `isCapabilityRegistered()` | Whether capability publish succeeded |
| `setDelay(int ms)` | Delay after capability register (default **50** ms) |
| `registerCapability()` | Publish retained output capability |
| `publishData()` | Publish one output value on the data topic |

### `Receive`

| Method | Purpose |
|--------|---------|
| `getName()`, `getType()`, `getPin()`, `getEntryPoint()` | Accessors |
| `registerCapability()` | Publish retained control capability |
| `handleCommand(value, result)` | Apply command; fill `result["newState"]` |

**`handleCommand` behaviour by `type` string**

| type | Effect |
|------|--------|
| `float` | Writes `float` into `float*` |
| `integer` | Writes `int` into `float*` (stored as float) |
| `boolean` | `"on"` / `"true"` / `"1"` → `1.0`, else `0.0` |
| `string` | Writes into `String*` if registered that way; if a `float*` exists it is set to `1.0` |

Unknown types return `false`.

---

## MQTT topics and payloads

Replace `{deviceId}` with your `device_id`.

| Topic | Direction | Role |
|-------|-----------|------|
| `devices/{deviceId}/data` | Device → broker | Sensor / output values |
| `devices/{deviceId}/status` | Device → broker | Online/offline, capability registration events |
| `devices/{deviceId}/commands` | Broker → device | Dashboard commands (subscribed on connect) |
| `devices/{deviceId}/command-response` | Device → broker | Success / error after handling a command |
| `devices/{deviceId}/capabilities_output/{name}` | Device → broker | Output capability (retained) |
| `devices/{deviceId}/capabilities_control/{name}` | Device → broker | Control capability (retained) |

### Output data (`launch` / `publishData`)

```json
{
  "type": "output",
  "deviceId": "device_1",
  "data": {
    "SensorValue": 42.5,
    "A0": 42.5
  }
}
```

Both the **name** and **pin** keys are set when the pin string is non-empty.

### Output capability

```json
{
  "deviceId": "device_1",
  "type": "capabilities_output",
  "outputName": "SensorValue",
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

`min` / `max` are only added for types `"float"` or `"integer"` when a range was given to `read()`.

### Incoming commands

Accepted on the commands topic in either shape:

**Flat**

```json
{ "value": 25, "control": "ControlValue" }
```

or with `"pin": "Ctrl"` instead of `"control"`.

**Nested** (when both `"type"` and `"data"` exist)

```json
{
  "type": "...",
  "data": {
    "value": 25,
    "control": "ControlValue"
  }
}
```

Matching order: by **control name** first, else by **pin**.

### Command response

Success:

```json
{
  "status": "success",
  "control": "ControlValue",
  "result": { "newState": 25 }
}
```

No match:

```json
{
  "status": "error",
  "message": "No matching control or pin found",
  "control": "...",
  "pin": "..."
}
```

(`control` / `pin` included when known.)

### Status / registration events

On successful MQTT connect (retained):

```json
{ "status": "online", "deviceId": "device_1" }
```

During `registerAllCapabilities()`:

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

## Last Will (LWT) and online / offline

1. Call `IOTFIY.enableLWT(true)` before `reconnectMQTT()`.  
2. On connect, PubSubClient is given a will:  
   - Topic: `devices/{deviceId}/status`  
   - Payload: `{"status":"offline","deviceId":"..."}`  
   - QoS **1**, **retained**  
3. After connect, the library also publishes retained **online** on the same topic.  
4. If the device drops without a clean disconnect, the broker publishes the will (**offline**).

Without `enableLWT(true)`, connect uses username/password or client id only — no will is set.

---

## Reconnection behaviour

| Layer | Behaviour |
|-------|-----------|
| WiFi | `checkAndReconnectWiFi()` — up to 20 tries, 500 ms apart |
| MQTT | `reconnectMQTT()` — up to 5 tries, 5 s between failures; on success re-subscribes and re-registers capabilities |

Always call `processMQTTMessages()` in `loop()`. If WiFi drops, reconnect WiFi first, then MQTT.

MQTT failure states printed to Serial include: timeout, connection lost, connect failed, disconnected, bad protocol, bad client id, unavailable, bad credentials, unauthorized.

---

## Examples in this repository

| Example | Path | Notes |
|---------|------|--------|
| ESP8266 basic | [`examples/ESP8266_Basic/ESP8266_Basic.ino`](examples/ESP8266_Basic/ESP8266_Basic.ino) | Sensor publish every 3 s + float control; works pattern also on ESP32 |
| M5Stack basic | [`examples/M5Stack_Basic/M5Stack_Basic.ino`](examples/M5Stack_Basic/M5Stack_Basic.ino) | ESP32 + M5Unified; buttons A/B/C via `write` + `launch`; needs **M5Unified** |

### Buttons (same pattern as M5 example)

```cpp
float btnA = 0, btnB = 0, btnC = 0;

IOTFIY.write("ButtonA", "integer", "BtnA", &btnA);
IOTFIY.write("ButtonB", "integer", "BtnB", &btnB);
IOTFIY.write("ButtonC", "integer", "BtnC", &btnC);

// on press:
btnB = 0; btnC = 0;
btnA = 1;
IOTFIY.launch();
```

### GPS-style floats

```cpp
float latitude = 24.8607;
float longitude = 67.0011;

IOTFIY.write("Latitude", "float", "Lat", &latitude);
IOTFIY.write("Longitude", "float", "Lng", &longitude);

// when values change:
IOTFIY.launch();
```

### String control (dashboard → device)

```cpp
String colorHex;

IOTFIY.read("Color", "string", "V4", &colorHex);
```

### String output without `write()`

```cpp
IOTFIY.publishStringOutput("StatusText", "Msg", "ready");
```

---

## ESP32 vs ESP8266

| Topic | Detail |
|-------|--------|
| WiFi header | Library includes `WiFi.h` (ESP32) or `ESP8266WiFi.h` (ESP8266) automatically |
| Your sketch | May still include the matching WiFi header if you use `WiFi.localIP()` etc. |
| Analog demo | ESP8266 example uses `analogRead(A0)`; ESP32 has no single standard A0 — use your own pin or a demo value |
| M5Stack example | ESP32 + M5 hardware only |
| Limits | Same API, same MQTT topics, max 10 sends / 10 receives |

---

## Troubleshooting

| Symptom | What to check |
|---------|----------------|
| Compile error about unsupported board | Use ESP32 or ESP8266 |
| WiFi fails | SSID/password; `checkAndReconnectWiFi()` Serial output |
| MQTT `CONNECT_BAD_CREDENTIALS` / `UNAUTHORIZED` | Username/password; broker ACL |
| MQTT timeout / connect failed | Host/IP (no `mqtt://`), port, firewall, broker running |
| No dashboard updates | Call `launch()` after changing the `float` variables |
| Commands ignored | Call `processMQTTMessages()`; confirm `read()` name/pin match the JSON |
| Payload truncated | Keep messages under `MQTT_MAX_PACKET_SIZE` (512 by default) |
| `write` / `read` returns `nullptr` | Already at 10 outputs or 10 controls |
| Missing PubSubClient / ArduinoJson | Install from Library Manager |

Open **Tools → Serial Monitor** at **115200** baud while testing.

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

## License / support

Maintainer: **iotfiy.solution@gmail.com** (see `library.properties`).

For issues and updates, use the [GitHub repository](https://github.com/iotfiy-solutions/Gateway_Library).
