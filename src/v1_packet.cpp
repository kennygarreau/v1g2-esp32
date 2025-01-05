#include "v1_packet.h"
#include "v1_config.h"
#include "v1_tft.h"
#include "TFT_eSPI.h"
#include <vector>

struct BandDirection {
    const char* band;
    const char* direction; };
struct alertByte {
    int count;
    int index; };
static std::string barValue, bogeyValue, directionValue, bandValue;
static std::string lastPayload = "";
int frontStrengthVal, rearStrengthVal;
int arrowColor = TFT_YELLOW;
bool priority, junkAlert;
static int alertCountValue, alertIndexValue;
float freqGhz;
alertsVector alertTable;
TFT_eSprite& sprite = displayController.getSprite();
Config globalConfig;

extern void requestMute();
bool muted = false;
uint8_t packet[10];

PacketDecoder::PacketDecoder(const std::string& packet) : packet(packet) {}

/* 
    hexToDecimal is an overloaded function which will convert a hex input to a decimal
    this is required for the bar mapping functions below
*/

int hexToDecimal(char hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    }
    return -1;
}

int hexToDecimal(const std::string& hexStr) {
    int decimalValue = 0;
    for (size_t i = 0; i < hexStr.length(); ++i) {
        int digit = hexToDecimal(hexStr[i]);
        if (digit == -1) {
            return -1;
        }
        decimalValue += digit * pow(16, hexStr.length() - 1 - i);
    }
    return decimalValue;
}

int mapXToBars(const std::string& hex) {
    int decimalValue = std::stoi(hex, nullptr, 16);

    if (decimalValue == 0x00) {return 0;}
    else if (decimalValue <= 0x95) {return 1;}
    else if (decimalValue <= 0x9F) {return 2;}
    else if (decimalValue <= 0xA9) {return 3;}
    else if (decimalValue <= 0xB3) {return 4;}
    else if (decimalValue <= 0xBC) {return 5;}
    else if (decimalValue <= 0xC4) {return 6;}
    else if (decimalValue <= 0xCF) {return 7;}
    else if (decimalValue <= 0xFF) {return 8;}
    else {return -1;}
}

int mapKToBars(const std::string& hex) {
    int decimalValue = std::stoi(hex, nullptr, 16);

    if (decimalValue == 0x00) {return 0;}
    else if (decimalValue <= 0x87) {return 1;}
    else if (decimalValue <= 0x8F) {return 2;}
    else if (decimalValue <= 0x99) {return 3;}
    else if (decimalValue <= 0xA3) {return 4;}
    else if (decimalValue <= 0xAD) {return 5;}
    else if (decimalValue <= 0xB7) {return 6;}
    else if (decimalValue <= 0xC1) {return 7;}
    else if (decimalValue <= 0xFF) {return 8;}
    else {return -1;}
}

int mapKaToBars(const std::string& hex) {
    int decimalValue = std::stoi(hex, nullptr, 16);

    if (decimalValue == 0x00) {return 0;}
    else if (decimalValue <= 0x8F) {return 1;}
    else if (decimalValue <= 0x96) {return 2;}
    else if (decimalValue <= 0x9D) {return 3;}
    else if (decimalValue <= 0xA4) {return 4;}
    else if (decimalValue <= 0xAB) {return 5;}
    else if (decimalValue <= 0xB2) {return 6;}
    else if (decimalValue <= 0xB9) {return 7;}
    else if (decimalValue <= 0xFF) {return 8;}
    else {return -1;}
}

int combineMSBLSB(const std::string& msb, const std::string& lsb) {
    int msbDecimal = hexToDecimal(msb);
    int lsbDecimal = hexToDecimal(lsb);
    if (msbDecimal == -1 || lsbDecimal == -1) {
        return -1;
    }
    return (msbDecimal * 256) + lsbDecimal;
}

void decodeByteZero(const std::string& userByte) {
    if (!userByte.empty()) {
        uint8_t byteValue = (uint8_t) strtol(userByte.c_str(), nullptr, 16);

        globalConfig.xBand = byteValue & 0b00000001;
        globalConfig.kBand = byteValue & 0b00000010;
        globalConfig.kaBand = byteValue & 0b00000100;
        globalConfig.laserBand = byteValue & 0b00001000;
        globalConfig.muteTo = (byteValue & 0b00010000) ? "Muted Volume" : "Zero";
        globalConfig.bogeyLockLoud = byteValue & 0b00100000;
        globalConfig.rearMute = byteValue & 0b01000000;
        globalConfig.kuBand = byteValue & 0b10000000;
    }
}

void decodeByteOne(const std::string& userByte) {
    if (!userByte.empty()) {
        uint8_t byteValue = (uint8_t) strtol(userByte.c_str(), nullptr, 16);
        
        globalConfig.euro = byteValue & 0b00000001;
        globalConfig.kVerifier = byteValue & 0b00000010;
        globalConfig.rearLaser = byteValue & 0b00000100;
        globalConfig.customFreqDisabled = byteValue & 0b00001000;
        globalConfig.kaAlwaysPrio = byteValue & 0b00010000;
        globalConfig.fastLaserDetection = byteValue & 0b00100000;
        globalConfig.kaSensitivityBit0 = (byteValue & 0b01000000) ? 1 : 0;
        globalConfig.kaSensitivityBit1 = (byteValue & 0b10000000) ? 2 : 0;

        int kaSensitivity = globalConfig.kaSensitivityBit0 + globalConfig.kaSensitivityBit1;
        switch (kaSensitivity) {
            case 0:
                globalConfig.kaSensitivity = "Max Range*";
                break;
            case 1:
                globalConfig.kaSensitivity = "Relaxed";
                break;
            case 2:
                globalConfig.kaSensitivity = "2020 Original";
                break;
            case 3:
                globalConfig.kaSensitivity = "Max Range";
                break;
        }
    }
}

void decodeByteTwo(const std::string& userByte) {
    if (!userByte.empty()) {
        uint8_t byteValue = (uint8_t) strtol(userByte.c_str(), nullptr, 16);
        
        globalConfig.startupSequence = byteValue & 0b00000001;
        globalConfig.restingDisplay = byteValue & 0b00000010;
        globalConfig.bsmPlus = byteValue & 0b00000100;
        globalConfig.autoMuteBit0 = (byteValue & 0b00001000) ? 1 : 0;
        globalConfig.autoMuteBit1 = (byteValue & 0b00010000) ? 2 : 0;

        int autoMute = globalConfig.autoMuteBit0 + globalConfig.autoMuteBit1;
        switch (autoMute) {
            case 0:
                globalConfig.autoMute = "Off*";
                break;
            case 1:
                globalConfig.autoMute = "On";
                break;
            case 2:
                globalConfig.autoMute = "On with Unmute 5+";
                break;
            case 3:
                globalConfig.autoMute = "Off";
                break;
        }
    }
}

std::string PacketDecoder::decodeDisplayData() {
    /* 
    if we have respAlertData, we should only need to process Laser (and empty) alert data here
    in v1_tft we should have a unique paint for laser alerts (similar to jbv1) - I don't think
    anything is needed from the aux packets
    */

    std::string payload = packet.substr(10, packet.size() - 12);

    if (payload.size() != 18) {
        return "err pkt";
    }

    std::string bandPlusArrow = payload.substr(6, 2);
    if (bandPlusArrow == "21" || bandPlusArrow == "81") {
        bandValue = "LASER";
        directionValue = (bandPlusArrow == "21") ? "FRONT" : "REAR";
        sprite.fillScreen(TFT_RED);
        displayController.displayText(bandValue.c_str(), selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y, TFT_WHITE);
    } else {
        bandValue = "";
        directionValue = "";
    }
   
    std::string decodedPayload = "BND:" + bandValue +
                                " DIR:" + directionValue +
                                " ";

    //Serial.println(("infDisplayData: " + decodedPayload).c_str());

    return decodedPayload;
}

void PacketDecoder::decodeAlertData(const alertsVector& alerts) {
    /* 
    execute if we successfully write reqStartAlertData to clientWriteUUID
    */
    frontStrengthVal = 0;
    rearStrengthVal = 0;

    sprite.fillScreen(TFT_BLACK);

    for (int i = 0; i < alerts.size(); i++) {
        std::string freqMSB = alerts[i].substr(2, 2);
        std::string freqLSB = alerts[i].substr(4, 2);

        int freqMhz = combineMSBLSB(freqMSB, freqLSB);
        freqGhz = static_cast<float>(freqMhz) / 1000.0f;

        std::string frontStrength = alerts[i].substr(6, 2);
        std::string rearStrength = alerts[i].substr(8, 2);
        std::string bandArrowDef = alerts[i].substr(10, 2);

        std::map<std::string, BandDirection> bandArrowMap;
            bandArrowMap["21"] = {"LASER", "FRONT"};
            bandArrowMap["81"] = {"LASER", "REAR"};
            bandArrowMap["24"] = {"K", "FRONT"};
            bandArrowMap["44"] = {"K", "SIDE"};
            bandArrowMap["84"] = {"K", "REAR"};
            bandArrowMap["22"] = {"KA", "FRONT"};
            bandArrowMap["42"] = {"KA", "SIDE"};
            bandArrowMap["82"] = {"KA", "REAR"};
            bandArrowMap["28"] = {"X", "FRONT"};
            bandArrowMap["48"] = {"X", "SIDE"};
            bandArrowMap["88"] = {"X", "REAR"};
            bandArrowMap["00"] = {"", ""};

        auto iBand = bandArrowMap.find(bandArrowDef);
        if (iBand != bandArrowMap.end()) {
            BandDirection direction = iBand->second;
            directionValue = direction.direction;
            bandValue = direction.band;
        } else {
            Serial.println("error in bandArrowMap key");
        }
        std::string alertIndexStr = alerts[i].substr(0, 2);

        std::string auxByte = alerts[i].substr(12, 2);
        if (auxByte == "80") {priority = true;}
        else if (auxByte == "40") {junkAlert = true;}
        else {priority = false; junkAlert = false;}

        std::map<std::string, alertByte> alertByteMap;
        // {alertCount, alertIndex}
        // Count: B0-B3, Index: B4-B7
        // bits 0-3 are for the # of alerts on the table. max of 15. bits 4-7 are for the alert index, again max of 15.
        // they are calculated individually rather than the bytesum
        alertByteMap["00"] = {0, 0};
        alertByteMap["11"] = {1, 1};
        alertByteMap["12"] = {2, 1};
        alertByteMap["13"] = {3, 1};
        alertByteMap["22"] = {2, 2};
        alertByteMap["23"] = {3, 2};
        alertByteMap["33"] = {3, 3};
        
        auto iCount = alertByteMap.find(alertIndexStr);
        if (iCount != alertByteMap.end()) {
            alertByte alerts = iCount->second;
            alertCountValue = alerts.count;
            alertIndexValue = alerts.index;
        } else {
            Serial.print("error in alertCountMap key: ");
            Serial.println(alertIndexStr.c_str());
        }

        /* after this there should be no substring processing; we should only focus on painting the display */
        // paint the main arrows and bars based on priority alerts
        if (priority) {
            arrowColor = TFT_RED;
        }
        
        // paint the directional arrow(s) - this should work for the prio alert now, so we may want to move this block
        if (directionValue == "FRONT") 
        { displayController.drawUpArrow(selectedConstants.ARROW_FRONT_X, selectedConstants.ARROW_FRONT_Y, selectedConstants.ARROW_FRONT_WIDTH, arrowColor);}
        else if (directionValue == "SIDE")
        { displayController.drawSideArrows(selectedConstants.ARROW_SIDE_X, selectedConstants.ARROW_SIDE_Y, selectedConstants.ARROW_SIDE_WIDTH, selectedConstants.ARROW_SIDE_HEIGHT, arrowColor); }
        else if (directionValue == "REAR")
        { displayController.drawDownArrow(selectedConstants.ARROW_REAR_X, selectedConstants.ARROW_REAR_Y, selectedConstants.ARROW_REAR_WIDTH, arrowColor); }

        // paint the small directional arrow of the alert
        if (bandValue == "X") {
            frontStrengthVal = mapXToBars(frontStrength);
            rearStrengthVal = mapXToBars(rearStrength);
            if (directionValue == "FRONT") {
                displayController.drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
            } else if (directionValue == "SIDE") {
                displayController.drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
            } else if (directionValue == "REAR") {
                displayController.drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
            }
        } 
        else if (bandValue == "K") {
            frontStrengthVal = mapKToBars(frontStrength);
            rearStrengthVal = mapKToBars(rearStrength);
            if (directionValue == "FRONT") {
                displayController.drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
            } else if (directionValue == "SIDE") {
                displayController.drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
            } else if (directionValue == "REAR") {
                displayController.drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
            }
        }
        else if (bandValue == "KA") {
            frontStrengthVal = mapKaToBars(frontStrength);
            rearStrengthVal = mapKaToBars(rearStrength);        
            if (directionValue == "FRONT") {
                displayController.drawUpArrow(selectedConstants.SMALL_ARROW_FRONT_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_FRONT_WIDTH, UI_COLOR);
            } else if (directionValue == "SIDE") {
                displayController.drawSideArrows(selectedConstants.SMALL_ARROW_SIDE_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_SIDE_WIDTH, selectedConstants.SMALL_ARROW_SIDE_HEIGHT, UI_COLOR);
            } else if (directionValue == "REAR") {
                displayController.drawDownArrow(selectedConstants.SMALL_ARROW_REAR_X, selectedConstants.SMALL_ARROW_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), selectedConstants.SMALL_ARROW_REAR_WIDTH, UI_COLOR);
            }
        }
        else if (bandValue == "LASER") {
            sprite.fillScreen(TFT_RED);
            displayController.displayText(bandValue.c_str(), selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y, TFT_WHITE);
            arrowColor = TFT_YELLOW;
            sprite.pushSprite(0,0);
            delay(2500);
        }
        
        if (bandValue != "LASER") {
            // paint the main signal bar
            if (rearStrengthVal > frontStrengthVal) { 
                displayController.drawSignalBars(rearStrengthVal, UI_COLOR); 
                displayController.drawHorizontalBars(selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i) - 10, rearStrengthVal, UI_COLOR); 
            }
            else { 
                displayController.drawSignalBars(frontStrengthVal, UI_COLOR); 
                displayController.drawHorizontalBars(selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i) - 10, frontStrengthVal, UI_COLOR);
            }

            // paint the frequency of the alert
            if (freqGhz > 0) {
                displayController.displayFreq(freqGhz, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), TFT_WHITE);
            }
        }
        // enable below for debugging
        std::string decodedPayload = "INDX:" + std::to_string(alertIndexValue) +
                                    //" FREQ:" + std::to_string(freqMhz) +
                                    " FREQ_GHZ:" + std::to_string(freqGhz) +
                                    " FSTR:" + std::to_string(frontStrengthVal) +
                                    " RSTR:" + std::to_string(rearStrengthVal) +
                                    " BAND:" + bandValue +
                                    " BDIR:" + directionValue +
                                    " PRIO:" + std::to_string(priority) +
                                    " JUNK:" + std::to_string(junkAlert);
                                    //+ " decode(ms): ";
        Serial.println(("respAlertData: " + decodedPayload).c_str());
    }
}

std::string PacketDecoder::decode(int lowSpeedThreshold, int currentSpeed) {
    unsigned long startTimeMillis = millis();

    std::string sof = packet.substr(0, 2);
    if (sof != "AA") {
        return "err SOF";
    }

    std::string endOfFrame = packet.substr(packet.length() - 2);
    if (endOfFrame != "AB") {
        return "err EOF";
    }

    std::string packetID = packet.substr(6, 2);
    
    if (packetID == "31") {
        // does this need logic here around not processing identical packets?
        return decodeDisplayData();

    }
    else if (packetID == "43") {
        /* we should do a light decode here */
        bool alertTableUpdated;
        std::string payload = packet.substr(10, packet.length() - 12);
        
        if (payload == lastPayload) {
            alertTableUpdated = false;
        } 
        else {
            lastPayload = payload;
            alertTableUpdated = true;
            std::string alertIndexStr = packet.substr(10, 2);

            std::map<std::string, alertByte> alertByteMap;
            // {alertCount, alertIndex}
            // Count: B0-B3, Index: B4-B7; binary for the four bits for each var
            alertByteMap["00"] = {0, 0};
            alertByteMap["11"] = {1, 1};
            alertByteMap["12"] = {2, 1};
            alertByteMap["13"] = {3, 1};
            alertByteMap["22"] = {2, 2};
            alertByteMap["23"] = {3, 2};
            alertByteMap["33"] = {3, 3};
            
            auto iCount = alertByteMap.find(alertIndexStr);
            if (iCount != alertByteMap.end()) {
                alertByte alerts = iCount->second;
                alertCountValue = alerts.count;
                alertIndexValue = alerts.index;
            } else {
                Serial.print("error in alertCountMap key: ");
                Serial.println(alertIndexStr.c_str());
            }

            alertTable.push_back(payload);
            // check if the alertTable vector size is more than or equal to the tableSize (alerts.count) extracted from alertByte
            if (alertTable.size() >= alertCountValue) {
                decodeAlertData(alertTable);
                // clear the alertTable if we have no more elements to process
                alertTable.clear();
            } else {
                if (alertCountValue == 0) {
                    // this should blank the screen after the alertTable is cleared (no alerts present)
                    muted = false;
                    sprite.fillScreen(TFT_BLACK);
                }
                // is there anything to be done here?
            }
            sprite.pushSprite(0,0);
        unsigned long elapsedTimeMillis = millis() - startTimeMillis;
        //Serial.println(elapsedTimeMillis);
        }
        return "";
    }
    else if (packetID == "12") {
        std::string userByteZero = packet.substr(10, 2);
        std::string userByteOne = packet.substr(12, 2);
        std::string userByteTwo = packet.substr(14, 2);

        decodeByteZero(userByteZero);
        decodeByteOne(userByteOne);
        decodeByteTwo(userByteTwo);
    }
    else if (packetID == "66") {
        Serial.println("infV1Busy");
    }
    else if (packetID == "67") {
        Serial.println("respDataError encountered");
    }
    return "";
}

/*
the functions below are responsible for generating and sending packets to the v1. calculating checksums for 
inbound packets incurs unnecessary overhead therefore is not done in any of the functions above.
*/

uint8_t Packet::calculateChecksum(const uint8_t *data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

uint8_t* Packet::constructPacket(uint8_t destID, uint8_t sendID, uint8_t packetID, uint8_t *payloadData, uint8_t payloadLength, uint8_t *packet) {
    packet[0] = PACKETSTART;
    packet[1] = 0xD0 + destID;
    packet[2] = 0xE0 + sendID;
    packet[3] = packetID;
    packet[4] = payloadLength;

    if (payloadLength > 1) {
        for (int i = 0; i < payloadLength - 1; i++) {
            packet[5 + i] = payloadData[i];
        }
        packet[5 + payloadLength - 1] = calculateChecksum(packet, 5 + payloadLength - 1);
        packet[5 + payloadLength] = PACKETEND;
    } else {
        packet[4] = 0x01;
        packet[5] = calculateChecksum(packet, 5);
        packet[6] = PACKETEND;
    }

    // this is broken and needs fixing, but maybe not?
    /* Serial.print("Packet sent: ");
    for (int i = 0; i <= sizeof(packet + 2); i++) {
        Serial.print(packet[i], HEX);
    }
    Serial.println(); */
    return packet;
}

uint8_t* Packet::reqTurnOffMainDisplay() {
    uint8_t payloadData[] = {0x01, 0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqTurnOffMainDisplay packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQTURNOFFMAINDISPLAY, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqStartAlertData() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqStartAlertData packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQSTARTALERTDATA, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqVersion() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]); 
    Serial.println("Sending reqVersion packet");   
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQVERSION, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqSweepSections() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqSweepSection packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQSWEEPSECTIONS, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqSerialNumber() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqSerialNumber packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQSERIALNUMBER, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqTurnOnMainDisplay() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqTurnOnMainDisplay packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQTURNONMAINDISPLAY, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqMuteOn() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQMUTEON, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqMuteOff() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqMuteOff packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQMUTEOFF, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqBatteryVoltage() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqBatteryVoltage packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQBATTERYVOLTAGE, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqCurrentVolume() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqCurrentVolume packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQCURRENTVOLUME, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqUserBytes() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqUserBytes packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQUSERBYTES, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}