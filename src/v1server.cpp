/*
 * Valentine V1 Gen2 Remote Display
 * version: v0.9.1.4
 * Author: Kenny G
 * Date: 2025.Jan.04
 * License: GPL 3.0
 */

#include <Arduino.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include "TFT_eSPI.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "BLEDevice.h"
#include "pin_config.h"
#include "v1_config.h"
#include "v1_packet.h"
#include "v1_tft.h"
#include "v1_fs.h"
#include "nunitoFont.h"
#include <TinyGPS++.h>
#include "web.h"

AsyncWebServer server(80);
IPAddress local_ip(192, 168, 242, 1);
IPAddress gateway(192, 168, 242, 1);
IPAddress subnet(255, 255, 255, 0);

BLERemoteService* dataRemoteService;
BLERemoteCharacteristic* infDisplayDataCharacteristic = nullptr; // characteristic for obtaining v1 display data
BLERemoteCharacteristic* clientWriteCharacteristic = nullptr; // characteristic for client writes
BLEClient* pClient = nullptr;
BLEScan* pBLEScan;
bool connected = false;

static bool laserAlert = false;
String hexData = "";
static String previousHexData = "";
static std::string bogeyValue, barValue, bandValue, directionValue;

TFT_eSPI tft = TFT_eSPI();
DisplayController displayController(tft);
TFT_eSprite& dispSprite = displayController.getSprite();
TFT_eSprite signalStrengthSprite = TFT_eSprite(&tft);
int rssiSpriteWidth = 24;
int rssiSpriteHeight = 12;

GPSData gpsData;
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
int currentSpeed = 0;

v1Settings settings;
DisplayConstants selectedConstants;
Preferences preferences;

int loopCounter = 0;
unsigned long lastMillis = 0;

DisplayConstants portraitConstants = {170, 320, 10, 160, 54, 10, 96, 110, 32, 80, 144, 172, 32, 78, 70, 64, 10, 134, 280, 20, 5,
  110, 108, 74, 144, 230, 32, 35, 10, 6, 20, 4};
DisplayConstants landscapeConstants = {320, 170, 50, 20, 48, 50, 144, 244, 39, 128, 20, 32, 32, 188, 96, 112, 14, 10, 128, 20, 5,
  244, 148, 128, 20, 84, 32, 35, 12, 6, 18, 3};

const uint8_t notificationOn[] = {0x1, 0x0};

SPIFFSFileManager fileManager;

void requestMute() {
  clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqMuteOn(), 7, false);
}

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
    // Serial.print("BLE Advertised Device found: ");
    // Serial.println(advertisedDevice.toString().c_str());
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
    if (hexData != previousHexData) {
      // Serial.print("HEX rcvd: ");
      // Serial.println(hexData);
      
      previousHexData = hexData;
    }
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
  } else if (WIFI_MODE == WIFI_STA) {
    WiFi.begin(settings.ssid, settings.password);
  }
}

void queryDeviceInfo(BLEClient* pClient) {
  BLERemoteService* pService = pClient->getService(deviceInfoUUID);

  if (pService == nullptr) {
    Serial.println("Device Information Service not found.");
    return;
  }

  struct CharacteristicMapping {
    const char* name;
    const char* uuid;
    std::string* storage;
  } characteristics[] = {
    {"Manufacturer Name", "2A29", &manufacturerName},
    {"Model Number", "2A24", &modelNumber},
    {"Serial Number", "2A25", &serialNumber},
    {"Hardware Revision", "2A27", &hardwareRevision},
    {"Firmware Revision", "2A26", &firmwareRevision},
    {"Software Revision", "2A28", &softwareRevision}
  };

  for (const auto& charInfo : characteristics) {
    BLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(BLEUUID(charInfo.uuid));
    if (pCharacteristic != nullptr) {
      *charInfo.storage = pCharacteristic->readValue();
      Serial.printf("%s: %s\n", charInfo.name, charInfo.storage->c_str());
    } else {
      Serial.printf("%s not found.\n", charInfo.name);
    }
  }
}

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
        //Serial.println("Client write characteristic found");
        clientWriteCharacteristic = dataRemoteService->getCharacteristic(clientWriteUUID);
        if (clientWriteCharacteristic != nullptr) {
          infDisplayDataCharacteristic = dataRemoteService->getCharacteristic(infDisplayDataUUID);
          infDisplayDataCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
          delay(100);
          
          // If we allow the user to select full blank vs BT illuminated, we may need to modify the buffer size
          if (settings.turnOffDisplay) {
            clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqTurnOffMainDisplay(), 8, false);
            delay(50);
          }

          clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqStartAlertData(), 7, false);
          }
      }
      else if (characteristic.first == infDisplayDataUUID.toString()) {
        //Serial.println("Data display characteristic found");
        infDisplayDataCharacteristic = dataRemoteService->getCharacteristic(infDisplayDataUUID);
        infDisplayDataCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
        }
      }
    }
  }
}

void loadSettings() {
  settings.wifiMode = preferences.getString("wifiMode", "WIFI_AP");

  settings.ssid = preferences.getString("ssid", "v1display");
  settings.password = preferences.getString("password", "password123");
  settings.disableBLE = preferences.getBool("disableBLE", false);
  settings.displayTest = preferences.getBool("displayTest", false);
  settings.enableGPS = preferences.getBool("enableGPS", false);
  settings.lowSpeedThreshold = preferences.getInt("lowSpeedThres", 35);
  settings.unitSystem = preferences.getString("unitSystem", "Imperial");
  settings.displayOrientation = preferences.getInt("displayOrient", 0);

  if (settings.displayOrientation == 0 || settings.displayOrientation == 2) {
    settings.isPortraitMode = true;
  } else {
    settings.isPortraitMode = false;
  }

  settings.textColor = preferences.getUInt("textColor", 0xFF0000);
  settings.turnOffDisplay = preferences.getBool("turnOffDisplay", true);
  settings.onlyDisplayBTIcon = preferences.getBool("onlyDispBTIcon", true);

  selectedConstants = settings.isPortraitMode ? portraitConstants : landscapeConstants;
  Serial.println("Panel Width: " + String(selectedConstants.MAX_X));
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
  // Retrieve and display Bluetooth signal strength
  int rssi = getBluetoothSignalStrength(); // Function to get Bluetooth signal strength

  // Clear signal strength display sprite
  signalStrengthSprite.fillSprite(TFT_BLACK);
  signalStrengthSprite.setTextColor(TFT_BLUE);
  signalStrengthSprite.drawString(String(rssi), 0, 0);
}

String formatTime(TinyGPSPlus &gps) {
  char timeBuffer[10];
  if (gps.time.isValid()) {
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
  } else {
    snprintf(timeBuffer, sizeof(timeBuffer), "00:00:00");
  }
  return String(timeBuffer);
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  pinMode(PIN_BUTTON_2, INPUT);

  Serial.println("Reading initial settings...");
  preferences.begin("settings", false);
  loadSettings();

  Serial.print("settings.isPortraitMode is set to: ");
  Serial.println(settings.isPortraitMode ? "true" : "false");
  
  loadSelectedConstants(selectedConstants);
  // if not initialized yet (first boot) - initialize settings
  if (selectedConstants.MAX_X == 0 && selectedConstants.MAX_Y == 0) {
    selectedConstants = settings.isPortraitMode ? portraitConstants : landscapeConstants;

    // Save selectedConstants to Preferences
    Serial.println("CONFIG: this appears to be the first boot, saving defaults");
    saveSelectedConstants(selectedConstants);
  }

  preferences.end();

  if (settings.enableGPS) {
    Serial.println("Initializing GPS...");
    gpsSerial.begin(BAUD_RATE, SERIAL_8N1, RXD, TXD);
  }

  Serial.println("DEBUG: Initializing display...");
  tft.begin();
  
  tft.setRotation(settings.displayOrientation);
  tft.fillScreen(TFT_BLACK);
  
  int rssiSpriteWidth = 30;
  int rssiSpriteHeight = 12;

  dispSprite.createSprite(selectedConstants.MAX_X, selectedConstants.MAX_Y - rssiSpriteHeight);
  signalStrengthSprite.setColorDepth(8);
  signalStrengthSprite.createSprite(rssiSpriteWidth, rssiSpriteHeight);

  // TODO: add a failsafe handler for invalid wifi credentials
  Serial.println("Connecting to WiFi");
  wifiSetup();
  wifiConnect();

  if (!fileManager.init()) {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }

 setupWebServer();

  if (settings.displayTest == true) {
    
    Serial.println("========================");
    Serial.println("====  DISPLAY TEST  ====");
    Serial.println("========================");

    std::string packets[] = {"AAD6EA430713291D21858800E8AB", "AAD6EA4307235E569283240000AB", "AAD6EA430733878CB681228030AB"};
    
    for (const std::string& packet : packets) {
      PacketDecoder decoder(packet); 
      std::string decoded = decoder.decode(settings.lowSpeedThreshold, currentSpeed);
    }

    sleep(30);
  }

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


  queryDeviceInfo(pClient);
  }
 Serial.println("v1g2 firmware version: " + String(FIRMWARE_VERSION));

}

void loop() {  
  static bool configHasRun = false;

  int buttonState = digitalRead(PIN_BUTTON_2);
  if (buttonState == LOW) {
    //TODO: something if button is pressed
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
  std::string decoded = decoder.decode(settings.lowSpeedThreshold, currentSpeed);

  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMillis >= 2000) {
    
    // uncomment below for debugging
    //Serial.print("Loops executed: ");
    //Serial.println(loopCounter);
    
    updateSignalStrengthDisplay();
    signalStrengthSprite.pushSprite(tft.width() - signalStrengthSprite.width(), tft.height() - signalStrengthSprite.height());

    if (!configHasRun) {
      if (globalConfig.muteTo.empty()) {
        Serial.print("User settings not obtained. ");
        clientWriteCharacteristic->writeValue((uint8_t*)Packet::reqUserBytes(), 7, false);
      } else {
        Serial.println("User settings obtained!");
        configHasRun = true;
      }
    }
    lastMillis = currentMillis;
    loopCounter = 0;
  }

  if (settings.enableGPS) {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
      }
      if (gps.location.isUpdated()) {
        gpsData.latitude = gps.location.lat();
        gpsData.longitude = gps.location.lng();
        gpsData.satelliteCount = gps.satellites.value();
        gpsData.time = formatTime(gps);

        if (settings.unitSystem == "Metric") {
          gpsData.speed = gps.speed.kmph();
          gpsData.altitude = gps.altitude.meters();
          currentSpeed = static_cast<int>(gps.speed.kmph());
        } else {
          gpsData.speed = gps.speed.mph();
          gpsData.altitude = gps.altitude.feet();
          currentSpeed = static_cast<int>(gps.speed.mph());
        }
      }
  }

  loopCounter++;

  checkReboot();

  yield();  // Prevent watchdog timeout
  delay(1);
}