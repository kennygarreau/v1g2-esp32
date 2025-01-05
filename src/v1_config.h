#pragma once
#include "BLEDevice.h"
#include "v1_tft.h"
#include "TFT_eSPI.h"
#include <Preferences.h>

#define FIRMWARE_VERSION "0.9.1.5"

extern TFT_eSPI tft;
extern DisplayController displayController;

struct v1Settings {
  int displayOrientation;
  uint32_t textColor;
  String ssid;
  String password;
  String wifiMode;
  String unitSystem;
  bool isPortraitMode;
  bool disableBLE;
  bool enableGPS;
  bool displayTest;
  bool turnOffDisplay;
  bool onlyDisplayBTIcon;
  int lowSpeedThreshold;
};

struct GPSData {
  double latitude;
  double longitude;
  double speed;
  double altitude;
  int satelliteCount;
  String time;
};

struct DisplayConstants {
    int MAX_X;
    int MAX_Y;
    int MHZ_DISP_X;
    int MHZ_DISP_Y;
    int MHZ_DISP_Y_OFFSET;

    int BAR_START_X;
    int BAR_START_Y;

    int ARROW_FRONT_X;
    int ARROW_FRONT_Y;
    int ARROW_FRONT_WIDTH;

    int SMALL_ARROW_FRONT_X;
    int SMALL_ARROW_Y;
    int SMALL_ARROW_FRONT_WIDTH;

    int ARROW_SIDE_X;
    int ARROW_SIDE_Y;
    int ARROW_SIDE_WIDTH;
    int ARROW_SIDE_HEIGHT;

    int SMALL_ARROW_SIDE_X;
    int SMALL_ARROW_SIDE_Y;
    int SMALL_ARROW_SIDE_WIDTH;
    int SMALL_ARROW_SIDE_HEIGHT;

    int ARROW_REAR_X;
    int ARROW_REAR_Y;
    int ARROW_REAR_WIDTH;

    int SMALL_ARROW_REAR_X;
    int SMALL_ARROW_REAR_Y;
    int SMALL_ARROW_REAR_WIDTH;

    int BAR_WIDTH;
    int BAR_HEIGHT;
    int H_BAR_HEIGHT;
    int H_BAR_WIDTH;
    int SPACING;
};

extern DisplayConstants selectedConstants;
extern std::string manufacturerName, modelNumber, serialNumber, hardwareRevision, firmwareRevision, softwareRevision;

extern Preferences preferences;

extern bool isPortraitMode;

// for GPS connectivity
#define BAUD_RATE 9600

#define WIFI_MODE WIFI_STA

// Custom UI color; should be modified to an acceptable TFT_COLOR
// See https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h for default enumerations
#define UI_COLOR TFT_RED

//#define V1LE
#ifdef V1LE
    // to-do: discover Bluetooth LE dongle Service UUID
    static BLEUUID v1leServiceUUID("92A0AFF4-9E05-11E2-AA59-F23C91AEC05E");
#else
    static BLEUUID bmeServiceUUID("92A0AFF4-9E05-11E2-AA59-F23C91AEC05E");
#endif

// Bluetooth service configurations
static BLEUUID deviceInfoUUID("180A");

// Device Information Service characteristics
static BLEUUID manufacturerUUID("2A29");
static BLEUUID modelUUID("2A24");
static BLEUUID serialUUID("2A25");
static BLEUUID hardwareUUID("2A27");
static BLEUUID firmwareUUID("2A26");
static BLEUUID softwareUUID("2A28");

// V1 Service characteristics
static BLEUUID infDisplayDataUUID("92A0B2CE-9E05-11E2-AA59-F23C91AEC05E"); // V1 out client in SHORT - notify for alerts
//static BLEUUID char2UUID("92A0B4E0-9E05-11E2-AA59-F23C91AEC05E"); // V1 out client in LONG
//static BLEUUID char5UUID("92A0BCE0-9E05-11E2-AA59-F23C91AEC05E"); // notify
static BLEUUID clientWriteUUID("92A0B6D4-9E05-11E2-AA59-F23C91AEC05E"); // client out V1 in SHORT
//static BLEUUID char4UUID("92A0B8D2-9E05-11E2-AA59-F23C91AEC05E"); // client out V1 in LONG
//static BLEUUID char6UUID("92A0BAD4-9E05-11E2-AA59-F23C91AEC05E"); // write and write without response