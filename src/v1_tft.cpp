#include "v1_tft.h"
#include "v1_config.h"
#include "TFT_eSPI.h"
#include "nunitoFont.h"

DisplayController::DisplayController(TFT_eSPI& display) : tft(display), sprite(&display) {}

TFT_eSprite& DisplayController::getSprite() {
    return sprite;
}

void DisplayController::drawUpArrow(int x, int y, int size, uint16_t color) {
    sprite.fillTriangle(x, y - size / 4, x + size / 2, y + size / 4, x - size / 2, y + size / 4, color);
    sprite.fillRect(x - size / 4, y + size / 4, size / 2, size / 10, color);
}

void DisplayController::drawDownArrow(int x, int y, int size, uint16_t color) {
    sprite.fillTriangle(x, y + size / 8, x + size / 2, y - size / 8, x - size / 2, y - size / 8, color);
    sprite.fillRect(x - size / 4, y - size / 5, size / 2, size / 10, color);
}

void DisplayController::drawSideArrows(int x, int y, int width, int height, uint16_t color) {
    sprite.fillRect(x, y, width, height, color);
    sprite.fillTriangle(x, y + height * 2, x - height, y + height / 2, x, y - height, color);
    sprite.fillTriangle(x + width, y + height * 2, x + width + height, y + height / 2, x + width, y - height, color);
}

/* Currently unused, leave for future customization? */
// void DisplayController::displayBogeyCount(int x, int y, String bogeys, uint16_t color) {
//     tft.setTextColor(color);
//     tft.setTextSize(6);
//     tft.setTextFont(1);
//     tft.setCursor(x, y);
//     tft.print(bogeys);
// }

void DisplayController::displayFreq(float freqVal, int x, int y, uint16_t color) {
    sprite.loadFont(nunitoFont);
    sprite.setTextColor(color);
    sprite.setCursor(x, y);
    sprite.print(freqVal, 3);
    sprite.unloadFont();
}

void DisplayController::displayText(const char* text, int x, int y, uint16_t color) {
    sprite.setTextColor(color);
    sprite.setTextSize(1);
    sprite.setTextFont(1);
    sprite.setCursor(x, y);
    sprite.print(text);
}

// void DisplayController::displayLaser(int x, uint16_t color) {
//     displayText("L", LASER_DISP_X, LASER_DISP_Y, color);
// }

// void DisplayController::displayKa(int x, uint16_t color) {
//     displayText("Ka", KA_DISP_X, KA_DISP_Y, color);
// }

// void DisplayController::displayK(int x, uint16_t color) {
//     displayText("K", KBAND_DISP_X, KBAND_DISP_Y, color);
// }

// void DisplayController::displayX(int x, uint16_t color) {
//     displayText("X", XBAND_DISP_X, XBAND_DISP_Y, color);
// }

void DisplayController::drawHorizontalBars(int y, int strength, uint16_t color) {
    int startX = selectedConstants.BAR_START_X;
    int startY = y;

    if (strength) {
        for (int i = 0; i < strength; i++) {
            int barX = startX + i * (selectedConstants.H_BAR_WIDTH + selectedConstants.SPACING);
            int barY = startY;
            switch (i) {
                case 0:
                    color = TFT_GREEN;
                    break;
                case 1:
                    color = TFT_GREENYELLOW;
                    break;
                case 2:
                    color = TFT_YELLOW;
                    break;
                case 3:
                    color = TFT_ORANGE;
                    break;
                case 4:
                    color = TFT_RED;
                    break;
                case 5:
                    color = TFT_RED;
                    break;    
                case 6:
                    color = TFT_RED;
                    break;
                case 7:
                    color = TFT_RED;
                    break;      
            }

            sprite.fillRect(barX, barY, selectedConstants.H_BAR_WIDTH, selectedConstants.H_BAR_HEIGHT, color);
        }
    }
}

void DisplayController::horizGradStr(int x, int y, int barHeight, int strength) {
  int offset = 19;

  switch (strength) {
    case 1:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_GREEN);
        break;
    case 2:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_GREENYELLOW);
        break;
    case 3:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_YELLOW);
        break;
    case 4:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_ORANGE);
        break;
    case 5:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_ORANGE);
        break;
    case 6:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_BROWN);
        break;
    case 7:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_RED);
        break;
    case 8:
        sprite.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_RED);
        break;
    }
}

void DisplayController::drawSignalBars(int strength, uint16_t color) {
    
    int startX = selectedConstants.BAR_START_X;
    int startY = selectedConstants.BAR_START_Y;

    if (strength) {
        for (int i = 0; i < strength; i++) {
            int barX = startX;
            int barY = startY - i * (selectedConstants.BAR_HEIGHT + selectedConstants.SPACING);
            if (!color == TFT_BLACK) {
                switch (i) {
                    case 0:
                        color = TFT_GREEN;
                        break;
                    case 1:
                        color = TFT_GREENYELLOW;
                        break;
                    case 2:
                        color = TFT_YELLOW;
                        break;
                    case 3:
                        color = TFT_ORANGE;
                        break;
                    case 4:
                        color = TFT_RED;
                        break;
                    case 5:
                        color = TFT_RED;
                        break;  
                    case 6:
                        color = TFT_RED;
                        break;
                    case 7:
                        color = TFT_RED;
                        break;             
                }
            }
            sprite.fillRect(barX, barY, selectedConstants.BAR_WIDTH, selectedConstants.BAR_HEIGHT, color);
        }
    }
}