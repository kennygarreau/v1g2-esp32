#include "v1_tft.h"
#include "v1_config.h"
#include "TFT_eSPI.h"
#include "nunitoFont.h"

DisplayController::DisplayController(TFT_eSPI& display) : tft(display), sprite(&display) {}

TFT_eSprite& DisplayController::getSprite() {
    return sprite;
}

void DisplayController::drawUpArrow(int x, int y, int size, uint16_t color) {
    //size = 80;
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
void DisplayController::displayBogeyCount(int x, int y, String bogeys, uint16_t color) {
    tft.setTextColor(color);
    tft.setTextSize(6);
    tft.setTextFont(1);
    tft.setCursor(x, y);
    tft.print(bogeys);
}

void DisplayController::displayFreq(float freqVal, int x, int y, uint16_t color) {
    sprite.loadFont(nunitoFont);
    sprite.setCursor(x, y);
    sprite.print(freqVal, 3);
}

void DisplayController::displayText(const char* text, int x, int y, uint16_t color) {
    sprite.setTextColor(color);
    sprite.setTextSize(1);
    sprite.setTextFont(3);
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
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_GREEN);
        break;
    case 2:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_GREENYELLOW);
        break;
    case 3:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_YELLOW);
        break;
    case 4:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_ORANGE);
        break;
    case 5:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_ORANGE);
        break;
    case 6:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_BROWN);
        break;
    case 7:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_RED);
        break;
    case 8:
        tft.fillRectHGradient(x, y, offset * strength, barHeight, TFT_GREEN, TFT_RED);
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

/* this was just a test condition before, can probably be eliminated as it was moved to displayFreq() */
// void DisplayController::displayFrequency(float frequency, int index, uint16_t color) {
//     char buffer[6];
//     snprintf(buffer, sizeof(buffer), "%f", frequency);
//     if (index = 1) { displayText(buffer, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y, TFT_RED); }
//     else if (index = 2) { displayText(buffer, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + 15, TFT_RED); }
//     else if (index = 3) { displayText(buffer, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + 30, TFT_RED); }
// }

void DisplayController::displayTestHorizontal() {
    drawUpArrow(selectedConstants.ARROW_FRONT_X, selectedConstants.ARROW_FRONT_Y, selectedConstants.ARROW_FRONT_WIDTH, UI_COLOR);
    drawSideArrows(selectedConstants.ARROW_SIDE_X, selectedConstants.ARROW_SIDE_Y, selectedConstants.ARROW_SIDE_WIDTH, selectedConstants.ARROW_SIDE_HEIGHT, UI_COLOR);
    drawDownArrow(selectedConstants.ARROW_REAR_X, selectedConstants.ARROW_REAR_Y, selectedConstants.ARROW_REAR_WIDTH, UI_COLOR);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y - 10, 1, UI_COLOR);
    displayFreq(24.297, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 0), TFT_WHITE);
    drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_FRONT_Y, selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET - 10, 6, UI_COLOR);
    displayFreq(10.837, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 1), TFT_WHITE);
    drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_REAR_Y, selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET * 2 - 10, 5, UI_COLOR);
    displayFreq(34.532, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 2), TFT_WHITE);
    drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_SIDE_Y, selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
}

void DisplayController::displayTestPortrait_2() {
    drawUpArrow(selectedConstants.ARROW_FRONT_X, selectedConstants.ARROW_FRONT_Y, selectedConstants.ARROW_FRONT_WIDTH, UI_COLOR);
    //drawSideArrows(ARROW_SIDE_X, ARROW_SIDE_Y, ARROW_SIDE_WIDTH, ARROW_SIDE_HEIGHT, UI_COLOR);
    //drawDownArrow(ARROW_REAR_X, ARROW_REAR_Y, ARROW_REAR_WIDTH, UI_COLOR);
    
    // first respAlertData grouping
    //horizGradStr(10, MHZ_DISP_Y - 10, 6, 8);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y - 10, 6, UI_COLOR);
    displayFreq(24.150, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 0), TFT_WHITE);
    //displayText("24.150", MHZ_DISP_X, MHZ_DISP_Y, TFT_WHITE);
    drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_FRONT_Y, selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
    
    // second respAlertData grouping
    //horizGradStr(10, MHZ_DISP_Y + MHZ_DISP_Y_OFFSET - 10, 6, 5);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET - 10, 4, UI_COLOR);
    displayFreq(10.526, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 1), TFT_WHITE);
    drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_REAR_Y, selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
    
    // third respAlertData grouping
    //horizGradStr(10, MHZ_DISP_Y + MHZ_DISP_Y_OFFSET * 2 - 10, 6, 2);
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET * 2 - 10, 2, UI_COLOR);
    displayFreq(34.709, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 2), TFT_WHITE);
    drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_SIDE_Y, selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
    drawSignalBars(6, TFT_RED);
}

/* probably redundant after sprite testing is complete */
void DisplayController::displayTestPortrait_3() {
    //drawUpArrow(ARROW_FRONT_X, ARROW_FRONT_Y, ARROW_FRONT_WIDTH, UI_COLOR);
    drawSideArrows(selectedConstants.ARROW_SIDE_X, selectedConstants.ARROW_SIDE_Y, selectedConstants.ARROW_SIDE_WIDTH, selectedConstants.ARROW_SIDE_HEIGHT, UI_COLOR);
    drawDownArrow(selectedConstants.ARROW_REAR_X, selectedConstants.ARROW_REAR_Y, selectedConstants.ARROW_REAR_WIDTH, UI_COLOR);
    // first respAlertData grouping
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y - 10, 1, UI_COLOR);
    displayFreq(24.297, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 0), TFT_WHITE);
    drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_FRONT_Y, selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
    // second respAlertData grouping
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET - 10, 6, UI_COLOR);
    displayFreq(10.837, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 1), TFT_WHITE);
    drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_REAR_Y, selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
    // third respAlertData grouping
    drawHorizontalBars(selectedConstants.MHZ_DISP_Y + selectedConstants.MHZ_DISP_Y_OFFSET * 2 - 10, 5, UI_COLOR);
    displayFreq(34.536, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * 2), TFT_WHITE);
    drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_SIDE_Y, selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
    drawSignalBars(2, TFT_RED);
}