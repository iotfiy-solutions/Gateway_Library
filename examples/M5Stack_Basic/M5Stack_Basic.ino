#include <M5Unified.h>
#include <IOTFIY.h>
#include <WiFi.h>

// WiFi
const char* WIFI_SSID     = "IOTF2";
const char* WIFI_PASSWORD = "Gromore1";

// MQTT / device — fill these in
const char* MQTT_SERVER    = "mqtt://localhost";
const char* MQTT_CLIENT_ID = "";
const char* DEVICE_ID      = "device_C";
const int   MQTT_PORT      = 1883;

// Values sent to the IOTFIY frontend when buttons are pressed
float btnAValue = 0;
float btnBValue = 0;
float btnCValue = 0;

// Value received from the IOTFIY dashboard
float receivedNumber = 0;

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
  M5.Display.setTextSize(1);
  M5.Display.drawString("Dashboard number", M5.Display.width() / 2, 80);

  M5.Display.setTextSize(4);
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.drawFloat(receivedNumber, 1, M5.Display.width() / 2, 100);

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(String("Last button: ") + lastButton, M5.Display.width() / 2, M5.Display.height() - 40);

  M5.Display.drawString("A          B          C", M5.Display.width() / 2, M5.Display.height() - 18);
}

void sendButton(const char* name, float* value) {
  *value = 1;
  strncpy(lastButton, name, sizeof(lastButton));
  lastButton[sizeof(lastButton) - 1] = '\0';

  IOTFIY.launch();
  drawScreen();

  Serial.print("Sent button ");
  Serial.println(name);
}

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  if (M5.Display.width() < M5.Display.height()) {
    M5.Display.setRotation(M5.Display.getRotation() ^ 1);
  }

  drawScreen();

  IOTFIY.config(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_CLIENT_ID, DEVICE_ID, MQTT_PORT);

  IOTFIY.checkAndReconnectWiFi();
  IOTFIY.begin();

  // Buttons A / B / C -> frontend (IOTFIY.write)
  IOTFIY.write("ButtonA", "integer", "BtnA", &btnAValue);
  IOTFIY.write("ButtonB", "integer", "BtnB", &btnBValue);
  IOTFIY.write("ButtonC", "integer", "BtnC", &btnCValue);

  // Dashboard command / number -> M5 screen (IOTFIY.read)
  IOTFIY.read("DashboardNumber", "float", "Display", &receivedNumber);

  IOTFIY.reconnectMQTT();

  lastWifiOk = (WiFi.status() == WL_CONNECTED);
  lastMqttOk = IOTFIY.isMQTTConnected();
  lastShownNumber = receivedNumber;
  drawScreen();
}

void loop() {
  M5.update();

  IOTFIY.checkAndReconnectWiFi();
  if (!IOTFIY.isMQTTConnected()) {
    IOTFIY.reconnectMQTT();
  }
  IOTFIY.processMQTTMessages();

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
