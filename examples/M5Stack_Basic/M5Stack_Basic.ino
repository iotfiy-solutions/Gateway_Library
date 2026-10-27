/*
  IOTFIY — M5Stack Basic example (ESP32 + M5Unified)

  Board:     Tools → Board → ESP32 Arduino → your M5 module
  Libraries: M5Unified, PubSubClient, ArduinoJson, IOTFIY

  write() / read():
    - Type is inferred from the variable
    - Name is optional (defaults to pin)
*/

#include <M5Unified.h>
#include <IOTFIY.h>

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_ID     = "device_m5_1";

int btnAValue = 0;   // → type "integer"
int btnBValue = 0;
int btnCValue = 0;
float receivedNumber = 0;  // → type "float"

float lastShownNumber = -9999;
char lastButton[2] = "-";
bool lastWifiOk = false;
bool lastMqttOk = false;

void drawScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);

  M5.Display.setTextSize(2);
  M5.Display.drawString("IOTFIY Basic", M5.Display.width() / 2, 8);

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(lastWifiOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
  M5.Display.drawString(lastWifiOk ? "WiFi: OK" : "WiFi: ...", M5.Display.width() / 2, 36);

  M5.Display.setTextColor(lastMqttOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
  M5.Display.drawString(lastMqttOk ? "MQTT: OK" : "MQTT: ...", M5.Display.width() / 2, 52);

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.drawString("Dashboard number", M5.Display.width() / 2, 80);

  M5.Display.setTextSize(4);
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.drawFloat(receivedNumber, 1, M5.Display.width() / 2, 100);

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(String("Last button: ") + lastButton, M5.Display.width() / 2, M5.Display.height() - 40);
  M5.Display.drawString("A          B          C", M5.Display.width() / 2, M5.Display.height() - 18);
}

void sendButton(const char* label, int* value) {
  *value = 1;
  strncpy(lastButton, label, sizeof(lastButton));
  lastButton[sizeof(lastButton) - 1] = '\0';

  IOTFIY.launch();
  drawScreen();

  Serial.print(F("Sent button "));
  Serial.println(label);
}

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  if (M5.Display.width() < M5.Display.height()) {
    M5.Display.setRotation(M5.Display.getRotation() ^ 1);
  }

  drawScreen();

  IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);
  IOTFIY.begin();

  IOTFIY.write("BtnA", &btnAValue);
  IOTFIY.write("BtnB", &btnBValue);
  IOTFIY.write("BtnC", &btnCValue);
  IOTFIY.read("Display", &receivedNumber);

  IOTFIY.launch();

  lastWifiOk = (WiFi.status() == WL_CONNECTED);
  lastMqttOk = IOTFIY.isMQTTConnected();
  lastShownNumber = receivedNumber;
  drawScreen();
}

void loop() {
  M5.update();
  IOTFIY.launch();

  if (M5.BtnA.wasPressed()) {
    btnBValue = 0;
    btnCValue = 0;
    sendButton("A", &btnAValue);
  }
  if (M5.BtnB.wasPressed()) {
    btnAValue = 0;
    btnCValue = 0;
    sendButton("B", &btnBValue);
  }
  if (M5.BtnC.wasPressed()) {
    btnAValue = 0;
    btnBValue = 0;
    sendButton("C", &btnCValue);
  }

  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  bool mqttOk = IOTFIY.isMQTTConnected();
  if (wifiOk != lastWifiOk || mqttOk != lastMqttOk || receivedNumber != lastShownNumber) {
    lastWifiOk = wifiOk;
    lastMqttOk = mqttOk;
    lastShownNumber = receivedNumber;
    drawScreen();
  }

  delay(10);
}
