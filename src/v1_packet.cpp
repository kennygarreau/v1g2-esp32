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
bool priority;
static int alertCountValue, alertIndexValue;
float freqGhz;
alertsVector alertTable;
TFT_eSprite& sprite = displayController.getSprite();

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
    /*
    update: does the v2 actually process six bars?
    */
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

std::string PacketDecoder::decodeDisplayData() {
    /* 
    if we have respAlertData, we should only need to process Laser (and empty) alert data here
    in v1_tft we should have a unique paint for laser alerts (similar to jbv1) - I don't think
    anything is needed from the aux packets
    */

    std::string payload = packet.substr(10, packet.size() - 12);

    if (payload.size() != 18) {
        return "Invalid displayData payload length!";
    }

    std::string bandPlusArrow = payload.substr(6, 2);
    if (bandPlusArrow == "21") {
        bandValue = "LASER!";
        directionValue = "FRONT";
    } else if (bandPlusArrow == "81") {
        bandValue = "LASER!";
        directionValue = "REAR";
    } else {
        bandValue = "";
        directionValue = "";
    }
   
    /*
    paint laser alert to displayController
    */
    
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
            // paint the directional arrow for the priority alert
            if (directionValue == "FRONT") 
            { displayController.drawUpArrow(selectedConstants.ARROW_FRONT_X, selectedConstants.ARROW_FRONT_Y, selectedConstants.ARROW_FRONT_WIDTH, UI_COLOR);}
            else if (directionValue == "SIDE")
            { displayController.drawSideArrows(selectedConstants.ARROW_SIDE_X, selectedConstants.ARROW_SIDE_Y, selectedConstants.ARROW_SIDE_WIDTH, selectedConstants.ARROW_SIDE_HEIGHT, UI_COLOR); }
            else if (directionValue == "REAR")
            { displayController.drawDownArrow(selectedConstants.ARROW_REAR_X, selectedConstants.ARROW_REAR_Y, selectedConstants.ARROW_REAR_WIDTH, UI_COLOR); }

            // paint the main signal bar with the strength of the priority alert
            if (rearStrengthVal > frontStrengthVal) { 
                displayController.drawSignalBars(rearStrengthVal, UI_COLOR); 
                displayController.drawHorizontalBars(selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i) - 10, rearStrengthVal, UI_COLOR); 
                }
            else { 
                displayController.drawSignalBars(frontStrengthVal, UI_COLOR); 
                displayController.drawHorizontalBars(selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i) - 10, frontStrengthVal, UI_COLOR);
                }
        }

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

        // paint the frequency of the alert
        displayController.displayFreq(freqGhz, selectedConstants.MHZ_DISP_X, selectedConstants.MHZ_DISP_Y + (selectedConstants.MHZ_DISP_Y_OFFSET * i), TFT_WHITE);

        // enable below for debugging
        std::string decodedPayload = "INDX:" + std::to_string(alertIndexValue) +
                                    " FREQ:" + std::to_string(freqMhz) +
                                    " FREQ_GHZ:" + std::to_string(freqGhz) +
                                    " FSTR:" + std::to_string(frontStrengthVal) +
                                    " RSTR:" + std::to_string(rearStrengthVal) +
                                    " BAND:" + bandValue +
                                    " BDIR:" + directionValue +
                                    " PRIO:" + std::to_string(priority) +
                                    " decode(ms): ";
        Serial.print(("respAlertData: " + decodedPayload).c_str());
    }
}

std::string PacketDecoder::decode() {
    unsigned long startTimeMillis = millis();

    std::string sof = packet.substr(0, 2);
    if (sof != "AA") {
        return "Invalid start of frame!";
    }

    std::string endOfFrame = packet.substr(packet.length() - 2);
    if (endOfFrame != "AB") {
        return "Invalid end of frame!";
    }

    std::string packetID = packet.substr(6, 2);
    
    if (packetID == "31") {
        // we should add some logic here around not processing identical packets
        // if (payload == lastPayload) {
        //     alertTableUpdated = false;
        // }
        // else {
        //     lastPayload = payload;
        //     alertTableUpdated = true;
            return decodeDisplayData();
        // }
    }
    else if (packetID == "43") {
        /* we should do a light decode here */
        //bool alertTableUpdated;
        std::string payload = packet.substr(10, packet.size() - 12);

        // 0418: need to test if this is what is causing the blackscreens and if it's even worth filtering here
        // if (payload == lastPayload) {
        //     alertTableUpdated = false;
        // } 
        // else {
            // lastPayload = payload;
            // alertTableUpdated = true;
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
            
            // 0418: this is unused, remove after testing
            std::string auxByte = payload.substr(12, 2);
            if (auxByte == "80") {priority = true;}
            else {priority = false;}
            
            // if the packet is a priority for the alert table, insert at the beginning of the alertTable
            // if (priority) {
            //     alertTable.insert(alertTable.begin(), payload);
            // }
            // else {
            //     alertTable.push_back(payload);
            // }

            alertTable.push_back(payload);
            // check if the alertTable vector size is more than or equal to the tableSize (alerts.count) extracted from alertByte
            if (alertTable.size() >= alertCountValue) {
                decodeAlertData(alertTable);
                // clear the alertTable if we have no more elements to process
                alertTable.clear();
            } else {
                if (alertCountValue == 0) {
                    // this should blank the screen after the alertTable is cleared (no alerts present)
                    sprite.fillScreen(TFT_BLACK);
                }
                // is there anything to be done here?
                Serial.println("untrapped alertCountValue and alertTable.size()");
            }
        }
        unsigned long elapsedTimeMillis = millis() - startTimeMillis;
        Serial.println(elapsedTimeMillis);
        return "";
    }
//     return "";
// }

/*
the functions below are responsible for generating and sending packets to the v1.
calculating checksums for inbound packets incurs unnecessary overhead therefore
is not done in any of the functions above.
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
        for (int i = 0; i < payloadLength - 2; i++) {
            packet[5 + i] = payloadData[i];
        }
        packet[5 + payloadLength - 2] = calculateChecksum(packet, 5 + payloadLength - 2);
        packet[5 + payloadLength - 1] = PACKETEND;
    } else {
        packet[4] = 0x01;
        packet[5] = calculateChecksum(packet, 5);
        packet[6] = PACKETEND;
    }
    Serial.print("reqStartAlertData packet sent: ");
    Serial.print(packet[0], HEX);
    Serial.print(packet[1], HEX);
    Serial.print(packet[2], HEX);
    Serial.print(packet[3], HEX);
    Serial.print(packet[4], HEX);
    Serial.print(packet[5], HEX);
    Serial.println(packet[6], HEX);
    return packet;
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

uint8_t* Packet::reqTurnOffMainDisplay() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqTurnOffMainDisplay packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQTURNOFFMAINDISPLAY, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}

uint8_t* Packet::reqBatteryVoltage() {
    uint8_t payloadData[] = {0x01};
    uint8_t payloadLength = sizeof(payloadData) / sizeof(payloadData[0]);
    Serial.println("Sending reqBatteryVoltage packet");
    return constructPacket(DEST_V1, REMOTE_SENDER, PACKET_ID_REQBATTERYVOLTAGE, const_cast<uint8_t*>(payloadData), payloadLength, packet);
}
