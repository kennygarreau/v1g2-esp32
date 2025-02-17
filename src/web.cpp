#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Update.h>
#include "v1_config.h"
#include "v1_packet.h"
#include "web.h"

extern AsyncWebServer server;
extern GPSData gpsData;
extern Config globalConfig;
extern v1Settings settings;
//Preferences preferences;

unsigned long rebootTime = 0;
bool isRebootPending = false;

std::string manufacturerName, modelNumber, serialNumber, hardwareRevision, firmwareRevision, softwareRevision;

String readFileFromSPIFFS(const char *path)
{
    File file = SPIFFS.open(path, "r");
    if (!file || file.isDirectory())
    {
        return "<h1>404 - Page not found</h1>";
    }
    String content;
    while (file.available())
    {
        content += String((char)file.read());
    }
    file.close();
    return content;
}

void serveStaticFile(AsyncWebServer &server, const char *path, const char *mimeType)
{
    server.on(path, HTTP_GET, [path, mimeType](AsyncWebServerRequest *request) { 
        request->send(SPIFFS, path, mimeType); 
    });
}

void checkReboot() {
    if (isRebootPending && millis() - rebootTime >= 5000) {
        ESP.restart();
    }
}

uint32_t hexToUint32(const String &hex) {
    String cleanHex = hex.startsWith("#") ? hex.substring(1) : hex;

    if (cleanHex.length() != 6) {
        Serial.println("Error: Invalid hex length");
        return 0;
    }

    for (char c : cleanHex) {
        if (!isxdigit(c)) {
            Serial.println("Error: Invalid hex character");
            return 0;
        }
    }

    uint32_t colorValue = strtoul(cleanHex.c_str(), nullptr, 16);

    // Debugging output to verify the conversion
    // Serial.print("Converted value: ");
    // Serial.println(colorValue, HEX);

    return colorValue;
}


void setupWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
        request->send(200, "text/html", readFileFromSPIFFS("/index.html")); 
    });

    server.on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"version\":\"" + String(FIRMWARE_VERSION) + "\"}");
    });

    serveStaticFile(server, "/index.js", "application/javascript");
    serveStaticFile(server, "/settings.js", "application/javascript");
    serveStaticFile(server, "/update.js", "application/javascript");
    serveStaticFile(server, "/index.html", "text/html");
    serveStaticFile(server, "/update.html", "text/html");
    serveStaticFile(server, "/settings.html", "text/html");
    serveStaticFile(server, "/style.css", "text/css");
    serveStaticFile(server, "/favicon.ico", "image/x-icon");

    server.on("/gps-info", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<200> jsonDoc;

        jsonDoc["latitude"] = gpsData.latitude;
        jsonDoc["longitude"] = gpsData.longitude;
        jsonDoc["speed"] = gpsData.speed;
        jsonDoc["altitude"] = gpsData.altitude;
        jsonDoc["time"] = gpsData.time;
        jsonDoc["satelliteCount"] = gpsData.satelliteCount;

        String jsonResponse;
        serializeJson(jsonDoc, jsonResponse);
        request->send(200, "application/json", jsonResponse); });

    server.on("/board-info", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<200> jsonDoc;

        jsonDoc["manufacturer"] = manufacturerName;
        jsonDoc["model"] = modelNumber;
        jsonDoc["serial"] = serialNumber;
        jsonDoc["hardwareVersion"] = hardwareRevision;
        jsonDoc["firmwareVersion"] = firmwareRevision;
        jsonDoc["softwareVersion"] = softwareRevision;

        // User Byte 0
        JsonObject configJson = jsonDoc.createNestedObject("config");
        configJson["X Band"] = globalConfig.xBand ? "On" : "Off";
        configJson["K Band"] = globalConfig.kBand ? "On" : "Off";
        configJson["Ka Band"] = globalConfig.kaBand ? "On" : "Off";
        configJson["Laser"] = globalConfig.laserBand ? "On" : "Off";
        configJson["Mute to"] = globalConfig.muteTo;
        configJson["Bogey-Lock Loud after muting"] = globalConfig.bogeyLockLoud ? "On" : "Off";
        configJson["X and K Rear Mute in Logic Modes"] = globalConfig.rearMute ? "Off" : "On";
        configJson["Ku Band"] = globalConfig.kuBand ? "Off" : "On";

        // User Byte 1
        configJson["Euro Mode"] = globalConfig.euro ? "Off" : "On";
        configJson["K-Verifier"] = globalConfig.kVerifier ? "On" : "Off";
        configJson["Rear Laser"] = globalConfig.rearLaser ? "On" : "Off";
        configJson["Custom Frequencies"] = globalConfig.customFreqDisabled ? "Disabled" : "Enabled";
        configJson["Ka Always Priority Alert"] = globalConfig.kaAlwaysPrio ? "Off" : "On";
        configJson["Fast Laser Detection"] = globalConfig.fastLaserDetection ? "On" : "Off";
        configJson["Ka Sensitivity"] = globalConfig.kaSensitivity;

        // User Byte 2
        configJson["Startup Sequence"] = globalConfig.startupSequence ? "On" : "Off";
        configJson["Resting Display"] = globalConfig.restingDisplay ? "On" : "Off";
        configJson["BSM Plus"] = globalConfig.bsmPlus ? "Off" : "On";
        configJson["Auto Mute"] = globalConfig.autoMute;

        JsonObject displaySettingsJson = jsonDoc.createNestedObject("displaySettings");
        displaySettingsJson["wifiMode"] = settings.wifiMode;
        displaySettingsJson["ssid"] = settings.ssid;
        displaySettingsJson["password"] = settings.password;
        displaySettingsJson["disableBLE"] = settings.disableBLE;
        displaySettingsJson["enableGPS"] = settings.enableGPS;
        displaySettingsJson["lowSpeedThreshold"] = settings.lowSpeedThreshold;
        displaySettingsJson["displayOrientation"] = settings.displayOrientation;
        displaySettingsJson["isPortraitMode"] = settings.isPortraitMode;
        displaySettingsJson["turnOffDisplay"] = settings.turnOffDisplay;
        displaySettingsJson["onlyDisplayBTIcon"] = settings.onlyDisplayBTIcon;
        displaySettingsJson["displayTest"] = settings.displayTest;
        displaySettingsJson["unitSystem"] = settings.unitSystem;

        char hexColor[8];
        sprintf(hexColor, "#%06X", settings.textColor & 0xFFFFFF);
        //Serial.printf("Stored color value: 0x%06X\n", settings.textColor & 0xFFFFFF);
        displaySettingsJson["textColor"] = hexColor;

        String jsonResponse;
        serializeJson(jsonDoc, jsonResponse);
        request->send(200, "application/json", jsonResponse); 
    });

    server.on("/updateFirmware", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool reboot = !Update.hasError();
        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain",
            reboot ? "Update Success! Rebooting..." : "Update Failed!");
        response->addHeader("Connection", "close");
        request->send(response);
        if (reboot) {
            isRebootPending = true;
            rebootTime = millis();
            } }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("Update Start: %s\n", filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            }
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
            if (final) {
                if (Update.end(true)) {
                    Serial.printf("Update Success: %u bytes\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            } 
    });

    server.on("/updateFilesystem", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Filesystem updated. Rebooting...");
        ESP.restart();
        }, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("Starting filesystem update: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
                Update.printError(Serial);
            }
        }
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("Filesystem update successful: %u bytes\n", index + len);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.on("/updateSettings", HTTP_POST, 
        [](AsyncWebServerRequest *request){},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
            //Serial.println("DEBUG: Body received in handler");
            String body = String((char*)data).substring(0, len);
            //Serial.println("DEBUG: Body content: " + body);

            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, body);

            if (error) {
                Serial.println("Failed to parse JSON");
                request->send(400, "application/json", "{\"error\": \"Failed to parse JSON\"}");
                return;
            }
            preferences.begin("settings", false);

            Serial.println("Settings updated:");
            if (doc.containsKey("wifiMode")) {
                settings.wifiMode = doc["wifiMode"].as<String>();
                Serial.println("wifiMode: " + settings.wifiMode);
                preferences.putString("wifiMode", settings.wifiMode);
            }  
            if (doc.containsKey("ssid")) {
                settings.ssid = doc["ssid"].as<String>();
                Serial.println("ssid: " + settings.ssid);
                preferences.putString("ssid", settings.ssid);
            }
            if (doc.containsKey("password")) {
                settings.password = doc["password"].as<String>();
                Serial.println("password: " + settings.password);
                preferences.putString("password", settings.password);
            }
            if (doc.containsKey("disableBLE")) {
                settings.disableBLE = doc["disableBLE"].as<bool>();
                Serial.println("disableBLE: " + String(settings.disableBLE));
                preferences.putBool("disableBLE", settings.disableBLE);
            }
            if (doc.containsKey("enableGPS")) {
                settings.enableGPS = doc["enableGPS"].as<bool>();
                Serial.println("enableGPS: " + String(settings.enableGPS));
                preferences.putBool("enableGPS", settings.enableGPS);
            }
            if (doc.containsKey("lowSpeedThreshold")) {
                settings.lowSpeedThreshold = doc["lowSpeedThreshold"].as<int>();
                Serial.println("lowSpeedThreshold: " + String(settings.lowSpeedThreshold));
                preferences.putInt("lowSpeedThres", settings.lowSpeedThreshold);
            }
            if (doc.containsKey("displayOrientation")) {
                settings.displayOrientation = doc["displayOrientation"].as<int>();
                Serial.println("displayOrientation: " + String(settings.displayOrientation));
                preferences.putInt("displayOrient", settings.displayOrientation);
            }
            if (doc.containsKey("textColor")) {
                //Serial.print("raw textColor from esp32: ");
                //Serial.println(doc["textColor"].as<String>());
                settings.textColor = hexToUint32(doc["textColor"].as<String>());
                Serial.println("textColor: " + String(settings.textColor));
                preferences.putUInt("textColor", settings.textColor);
            }
            if (doc.containsKey("displayTest")) {
                settings.displayTest = doc["displayTest"].as<bool>();
                Serial.println("displayTest: " + String(settings.displayTest));
                preferences.putBool("displayTest", settings.displayTest);
                delay(500);
                ESP.restart();
            }
            if (doc.containsKey("onlyDisplayBTIcon")) {
                settings.onlyDisplayBTIcon = doc["onlyDisplayBTIcon"].as<bool>();
                Serial.println("onlyDisplayBTIcon: " + String(settings.onlyDisplayBTIcon));
                preferences.putBool("onlyDispBTIcon", settings.onlyDisplayBTIcon);
            }
            if (doc.containsKey("turnOffDisplay")) {
                settings.turnOffDisplay = doc["turnOffDisplay"].as<bool>();
                Serial.println("turnOffDisplay: " + String(settings.turnOffDisplay));
                preferences.putBool("turnOffDisplay", settings.turnOffDisplay);
            }
            if (doc.containsKey("unitSystem")) {
                settings.unitSystem = doc["unitSystem"].as<String>();
                Serial.println("unitSystem: " + settings.unitSystem);
                preferences.putString("unitSystem", settings.unitSystem);
            }
            isRebootPending = true;
            preferences.end();


            request->send(200, "application/json", "{\"message\": \"Settings updated successfully - rebooting in 5 seconds\"}");
    });

    server.begin();
}