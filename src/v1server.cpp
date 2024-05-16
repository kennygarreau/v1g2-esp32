/*
 * Valentine V1 Gen2 Remote Display
 * version: v0.1
 * Author: Kenny G
 * Date: 2024.April 8
 * License: GPL 3.0
 */

#include <Arduino.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include "TFT_eSPI.h"
#include "WebServer.h"
#include "BLEDevice.h"
#include "pin_config.h"
#include "v1_config.h"
#include "v1_packet.h"
#include "v1_tft.h"
#include "v1_fs.h"
#include "nunitoFont.h"

WebServer server(80);
IPAddress local_ip(192, 168, 242, 1);
IPAddress gateway(192, 168, 242, 1);
IPAddress subnet(255, 255, 255, 0);

BLERemoteService* dataRemoteService;
BLERemoteService* infoRemoteService;
BLERemoteCharacteristic* infDisplayDataCharacteristic = nullptr; // characteristic for obtaining v1 display data
BLERemoteCharacteristic* clientWriteCharacteristic = nullptr; // characteristic for client writes
BLEClient* pClient = nullptr;
BLEScan* pBLEScan;
bool connected = false;

static bool laserAlert = false;
String hexData = "";
static std::string bogeyValue, barValue, bandValue, directionValue;

TFT_eSPI tft = TFT_eSPI();
DisplayController displayController(tft);
TFT_eSprite& dispSprite = displayController.getSprite();
TFT_eSprite signalStrengthDisplay = TFT_eSprite(&tft);

Preferences preferences;

struct v1Settings {
  int displayOrientation;
  uint16_t textColor;
  String ssid;
  String password;
  String wifiMode;
  bool isPortraitMode;
  bool disableBLE;
  bool storeMode;
  };
v1Settings settings;
bool isPortraitMode;
int loopCounter = 0;
unsigned long lastMillis = 0;

DisplayConstants portraitConstants = {170, 320, 10, 160, 54, 10, 96, 110, 32, 80, 144, 172, 32, 78, 70, 64, 10, 134, 280, 20, 5,
  110, 108, 74, 144, 230, 32, 35, 10, 6, 20, 4};
DisplayConstants landscapeConstants = {320, 170, 50, 20, 48, 50, 144, 244, 39, 128, 20, 32, 32, 188, 96, 112, 14, 10, 128, 20, 5,
  244, 148, 128, 20, 84, 32, 35, 12, 6, 18, 3};

//DisplayConstants selectedConstants = settings.isPortraitMode ? portraitConstants : landscapeConstants;
DisplayConstants selectedConstants;

const uint8_t notificationOn[] = {0x1, 0x0};

SPIFFSFileManager fileManager;

void scanAndConnect() {
  BLEScanResults foundDevices = pBLEScan->start(5);
  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(bmeServiceUUID))) {
      Serial.println("V1 found. Attempting to connect...");
      Serial.println(device.getAddress().toString().c_str());
      if (pClient->connect(&device)) {
        // Connection successful, stop scanning
        pBLEScan->stop();
        break;
      }
    }
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
  }
};

void connectToServer() {
    if (!connected) {
      scanAndConnect();
    }
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
      connected = true;
    }

    void onDisconnect(BLEClient* pClient) {
      connected = false;
    }
};

static void notifyDisplayCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  hexData = "";
  if (pData) {
    for (size_t i = 0; i < length; i++) {
      char hexBuffer[3];
      sprintf(hexBuffer, "%02X", pData[i]);
      hexData += hexBuffer;
    }
    // uncomment below for debug before entry into the payload calls
    //Serial.print("HEX rcvd: ");
    //Serial.println(hexData);
  }
}

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_DISCONNECTED:
      delay(1000);
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("WiFi begin succeeded ");
      Serial.println(WiFi.localIP());
      break;
    default:
       // Display additional events???
      break;    
  }
}

void wifiSetup() {
  WiFi.onEvent(onWiFiEvent);
}

void wifiConnect() {
  WiFi.mode(WIFI_MODE);
  if (WIFI_MODE == WIFI_MODE_AP) {
    WiFi.softAP(settings.ssid, settings.password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
  }
  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

}

void startupInfo() {
  if (pClient && pClient->isConnected()) {
    if (!dataRemoteService) {
      dataRemoteService = pClient->getService(bmeServiceUUID);
      if (dataRemoteService == nullptr) {
        Serial.println("Failed to find infDisplayData service.");
        return;
      }
    }
  clientWriteCharacteristic = dataRemoteService->getCharacteristic(clientWriteUUID);
    if (clientWriteCharacteristic != nullptr) {
      clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqVersion(), 1, false);
      delay(250);
      infDisplayDataCharacteristic = dataRemoteService->getCharacteristic(infDisplayDataUUID);
      infDisplayDataCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    }
  }
}

// void getHardwareInfo() {
//   if (pClient && pClient->isConnected()) {
//     if (!infoRemoteService) {
//       infoRemoteService = pClient->getService(deviceInfoUUID);
//       if (infoRemoteService == nullptr) {
//         Serial.println("Failed to find infoRemoteService");
//         return;
//       }
//     }
  
//   std::map<std::string, BLERemoteCharacteristic*>* pCharacteristics = infoRemoteService->getCharacteristics();
//     if (pCharacteristics) {
//       if (pCharacteristics->count("00002A29-0000-1000-8000-00805F9B34FB")) {
//           BLERemoteCharacteristic* manufacturerIDChar = (*pCharacteristics)["00002A29-0000-1000-8000-00805F9B34FB"];
//           std::string manufacturerIDValue = manufacturerIDChar->readValue();
//           Serial.println(manufacturerIDValue.c_str());
//       }

//       if (pCharacteristics->count("00002A27-0000-1000-8000-00805F9B34FB")) {
//           BLERemoteCharacteristic* hardwareRevisionChar = (*pCharacteristics)["00002A27-0000-1000-8000-00805F9B34FB"];
//           std::string hardwareRevisionValue = hardwareRevisionChar->readValue();
//           Serial.println(hardwareRevisionValue.c_str());
//       }

//       if (pCharacteristics->count("00002A24-0000-1000-8000-00805F9B34FB")) {
//           BLERemoteCharacteristic* modelNumberChar = (*pCharacteristics)["00002A24-0000-1000-8000-00805F9B34FB"];
//           std::string modelNumberValue = modelNumberChar->readValue();
//           Serial.println(modelNumberValue.c_str());
//       }
//     }
//   }
// }

void displayReader() {
  if (pClient && pClient->isConnected()) {
  if (!dataRemoteService) {
    dataRemoteService = pClient->getService(bmeServiceUUID);
    if (dataRemoteService == nullptr) {
      Serial.println("Failed to find infDisplayData service.");
      return;
    }
  }

  std::map<std::string, BLERemoteCharacteristic*>* pCharacteristics = dataRemoteService->getCharacteristics();
  if (pCharacteristics) {
    for (auto& characteristic : *pCharacteristics) {
      if (characteristic.first == clientWriteUUID.toString()) {
        Serial.println("Client write characteristic found");
        clientWriteCharacteristic = dataRemoteService->getCharacteristic(clientWriteUUID);
        if (clientWriteCharacteristic != nullptr) {
          clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqStartAlertData(), 7, false);
          delay(250);
          infDisplayDataCharacteristic = dataRemoteService->getCharacteristic(infDisplayDataUUID);
          infDisplayDataCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
        }
      }
      else if (characteristic.first == infDisplayDataUUID.toString()) {
        Serial.println("Data display characteristic found");
        infDisplayDataCharacteristic = dataRemoteService->getCharacteristic(infDisplayDataUUID);
        infDisplayDataCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
        }
      }
    }
  }
}

void handleFile(const char* filename, const char* contentType) {
    File file = fileManager.openFile(filename, "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, contentType);
    file.close();
}

void handleNotFound() {
  server.send(404, "text/html", "File not found");
}

void handleRoot() {
  handleFile("/index.html", "text/html");
}

void handleCSS() {
  handleFile("/style.css", "text/css");
}

void handleSettings() {
  handleFile("/settings.html", "text/html");
}

void handleUpdate() {
  handleFile("/update.html", "text/html");
}

void loadSettings() {
  // 0 = portrait, 3 = landscape
  //settings.isPortraitMode = preferences.getBool("isPortraitMode", true);
  settings.displayOrientation = preferences.getInt("displayOrientation", 0);
  settings.textColor = preferences.getUShort("textColor", TFT_WHITE);
  settings.ssid = preferences.getString("ssid", "v1display");
  settings.password = preferences.getString("password", "password123");
  settings.disableBLE = preferences.getBool("disableBLE", false);
  settings.storeMode = preferences.getBool("storeMode", false);
}

void saveSelectedConstants(const DisplayConstants& constants) {
    preferences.putBytes("selectedConstants", &constants, sizeof(DisplayConstants));
}

void loadSelectedConstants(DisplayConstants& constants) {
    preferences.getBytes("selectedConstants", &constants, sizeof(DisplayConstants));
}

int getBluetoothSignalStrength() {
  int rssi = pClient->getRssi();
  return rssi;
}

void updateSignalStrengthDisplay() {
    // Clear signal strength display sprite
    signalStrengthDisplay.fillSprite(TFT_BLACK);
    // Retrieve and display Bluetooth signal strength
    int rssi = getBluetoothSignalStrength(); // Function to get Bluetooth signal strength
    signalStrengthDisplay.setTextColor(TFT_BLUE);
    signalStrengthDisplay.drawString(String(rssi), 0, 0);
}

void setup()
{
  delay(1500);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  pinMode(PIN_BUTTON_2, INPUT);
  Serial.begin(115200);

  Serial.println("Reading initial settings...");
  preferences.begin("settings", false);
  settings.isPortraitMode = preferences.getBool("isPortraitMode", true);
  Serial.print("settings.isPortraitMode is set to: ");
  Serial.println(settings.isPortraitMode ? "true" : "false");
  // Serial.print("MAX_X: ");
  // Serial.println(selectedConstants.MAX_X);
  // Serial.print("MAX_Y: ");
  // Serial.println(selectedConstants.MAX_Y);
  
  loadSelectedConstants(selectedConstants);
  // if not initialized yet (first boot) - initialize settings
  if (selectedConstants.MAX_X == 0 && selectedConstants.MAX_Y == 0) {
    selectedConstants = settings.isPortraitMode ? portraitConstants : landscapeConstants;

    // Save selectedConstants to Preferences
    Serial.println("CONFIG: this appears to be the first boot, saving defaults");
    saveSelectedConstants(selectedConstants);
  }

  loadSettings();
  Serial.print("settings.displayOrientation is set to: ");
  Serial.println(String(settings.displayOrientation));

  Serial.print("MAX_X: ");
  Serial.println(selectedConstants.MAX_X);

  Serial.print("MAX_Y: ");
  Serial.println(selectedConstants.MAX_Y);

  tft.begin();
  
  tft.setRotation(settings.displayOrientation);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(settings.textColor);
  //tft.loadFont(nunitoFont);
  
  int rssiSpriteWidth = 30;
  int rssiSpriteHeight = 12;

  dispSprite.createSprite(selectedConstants.MAX_X, selectedConstants.MAX_Y - rssiSpriteHeight);
  signalStrengthDisplay.setColorDepth(8);
  signalStrengthDisplay.createSprite(rssiSpriteWidth, rssiSpriteHeight);

  Serial.println("Connecting to WiFi");
  wifiSetup();
  wifiConnect();

  if (!fileManager.init()) {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted succesfully");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/settings.html", HTTP_GET, handleSettings);
  server.on("/update.html", HTTP_GET, handleUpdate);
  server.onNotFound(handleNotFound);
  server.begin();

 if (settings.disableBLE == false) {
  Serial.println("Searching for V1 bluetooth..");
  BLEDevice::init("ESP32 Client");
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

  connectToServer();
  
  if (connected) {
    displayReader();
    infDisplayDataCharacteristic->registerForNotify(notifyDisplayCallback);
  } else {
    Serial.println("Could not find a V1 connection");
  }
 }
  preferences.end();

}

void loop() {  
  // web server handler
  server.handleClient();

  int buttonState = digitalRead(PIN_BUTTON_2);
  if (buttonState == LOW) {
    //do something if button is pressed
  }

  if (settings.disableBLE == false) {
    if (pClient->isConnected()) {
      connected = true;
    } else {
      Serial.println("disconnected - attempting reconnect");
      pBLEScan = BLEDevice::getScan();
      pBLEScan->setActiveScan(true);
      connectToServer();
      
      if (connected) {
        displayReader();
        infDisplayDataCharacteristic->registerForNotify(notifyDisplayCallback);
      } else {
      Serial.println("Could not find a V1 connection");
      }
    }
 }

  // decode loop takes 6-7ms
  std::string packet = hexData.c_str();
  PacketDecoder decoder(packet);
  std::string decoded = decoder.decode();

  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMillis >= 1000) {
    Serial.print("Loops executed: ");
    Serial.println(loopCounter);
    
    updateSignalStrengthDisplay();
    signalStrengthDisplay.pushSprite(tft.width() - signalStrengthDisplay.width(), tft.height() - signalStrengthDisplay.height());

    lastMillis = currentMillis;
    loopCounter = 0;
  }

  loopCounter++;

}