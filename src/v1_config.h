#pragma once
#include "BLEDevice.h"
#include "v1_tft.h"
#include "TFT_eSPI.h"

extern TFT_eSPI tft;
extern DisplayController displayController;

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

extern bool isPortraitMode;

// WIFI_STA: ESP32 is a wireless client (connecting to home/mobile network)
// WIFI_AP: ESP32 becomes an access point
#define WIFI_MODE WIFI_AP
#define DISABLE_BLE

// Custom UI color; should be modified to an acceptable TFT_COLOR
// See https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h for default enumerations
#define UI_COLOR TFT_RED

// Set the layout for the display
// unset below to debug display (store mode)
//#define DISPLAY_TEST

//#define LANDSCAPE_L
//#define LANDSCAPE_R
//#define PORTRAIT_1 // original prototype
//#define PORTRAIT_2 // L-R bars at top, arrows in middle, band(MHz) listings by color at bottom
//#define PORTRAIT_3 // vertical bar + arrows at top, band(MHz) listings by color at bottom

// set ORIENTATION to 0 for portrait, 1 for landscape (USB on right) or 3 for landscape (USB on left)
#ifdef LANDSCAPE_L
    #define ORIENTATION 3
#endif

#ifdef LANDSCAPE_R
    #define ORIENTATION 1
#endif

#if defined(LANDSCAPE_L) || (LANDSCAPE_R)
    #define MAX_X 320
    #define MAX_Y 170
    #define MHZ_DISP_X 50
    #define MHZ_DISP_Y 20
    #define MHZ_DISP_Y_OFFSET 48

    #define BAR_START_X 50
    #define BAR_START_Y 144

    #define ARROW_FRONT_X 244
    #define ARROW_FRONT_Y 39
    #define ARROW_FRONT_WIDTH 128
    
    #define SMALL_ARROW_FRONT_X 20
    #define SMALL_ARROW_FRONT_Y 32
    #define SMALL_ARROW_FRONT_WIDTH 32 // modify to change the scale
    
    #define ARROW_SIDE_X 188
    #define ARROW_SIDE_Y 96
    #define ARROW_SIDE_WIDTH 112
    #define ARROW_SIDE_HEIGHT 14
    
    #define SMALL_ARROW_SIDE_X 10
    #define SMALL_ARROW_SIDE_Y 128
    #define SMALL_ARROW_SIDE_WIDTH 20
    #define SMALL_ARROW_SIDE_HEIGHT 5

    #define ARROW_REAR_X 244
    #define ARROW_REAR_Y 148
    #define ARROW_REAR_WIDTH 128
    
    #define SMALL_ARROW_REAR_X 20
    #define SMALL_ARROW_REAR_Y 84
    #define SMALL_ARROW_REAR_WIDTH 32 // modify to change the scale

    #define BAR_WIDTH  35
    #define BAR_HEIGHT 12
    #define H_BAR_HEIGHT 6
    #define H_BAR_WIDTH 18
    #define SPACING 3
#endif

#if defined(PORTRAIT_1) || defined(PORTRAIT_2) || defined(PORTRAIT_3)
    #define ORIENTATION 0
#endif

#ifdef PORTRAIT_3
    #define MAX_X 170
    #define MAX_Y 320    
    #define MHZ_DISP_X 10
    #define MHZ_DISP_Y 160
    #define MHZ_DISP_Y_OFFSET 54

    #define BAR_START_X 10
    #define BAR_START_Y 96

    #define ARROW_FRONT_X 110
    #define ARROW_FRONT_Y 32
    #define ARROW_FRONT_WIDTH 80 // previously 96

    #define SMALL_ARROW_FRONT_X 144
    #define SMALL_ARROW_FRONT_Y 172
    #define SMALL_ARROW_FRONT_WIDTH 32 // modify to change the scale

    #define ARROW_SIDE_X 78
    #define ARROW_SIDE_Y 70
    #define ARROW_SIDE_WIDTH 64
    #define ARROW_SIDE_HEIGHT 10

    #define SMALL_ARROW_SIDE_X 134
    #define SMALL_ARROW_SIDE_Y 280
    #define SMALL_ARROW_SIDE_WIDTH 20
    #define SMALL_ARROW_SIDE_HEIGHT 5

    #define ARROW_REAR_X 110
    #define ARROW_REAR_Y 108
    #define ARROW_REAR_WIDTH 74 // previously 96

    #define SMALL_ARROW_REAR_X 144
    #define SMALL_ARROW_REAR_Y 230
    #define SMALL_ARROW_REAR_WIDTH 32 // modify to change the scale

    #define BAR_WIDTH  35
    #define BAR_HEIGHT 10
    #define H_BAR_HEIGHT 6
    #define H_BAR_WIDTH 20
    #define SPACING    4
#endif

// uncomment to scan/connect to V1 LE Bluetooth module
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