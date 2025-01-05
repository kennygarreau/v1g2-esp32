#include "TFT_eSPI.h"
#include <Arduino.h>

#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

class DisplayController {
private:
    TFT_eSPI& tft;
    TFT_eSprite sprite;

public:
    DisplayController(TFT_eSPI& display);
    TFT_eSprite& getSprite();

    void drawUpArrow(int x, int y, int size, uint16_t color);
    void drawDownArrow(int x, int y, int size, uint16_t color);
    void drawSideArrows(int x, int y, int width, int height, uint16_t color);
    void displayText(const char* text, int x, int y, uint16_t color);
    void drawSignalBars(int strength, uint16_t color);
    void drawHorizontalBars(int y, int strength, uint16_t color);
    void horizGradStr(int x, int y, int barHeight, int strength);
    void displayFreq(float freqVal, int x, int y, uint16_t color);
    //void displayFrequency(float frequency, int index, uint16_t color);
    // void displayLaser(int x, uint16_t color);
    // void displayKa(int x, uint16_t color);
    // void displayK(int x, uint16_t color);
    // void displayX(int x, uint16_t color);
    // void displayBogeyCount(int x, int y, String bogeys, uint16_t color);
};

#endif