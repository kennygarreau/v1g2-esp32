#include "v1_fs.h"

bool SPIFFSFileManager::init() {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS filesystem");
        return false;
    }
    return true;
}

File SPIFFSFileManager::openFile(const char* filePath, const char* mode) {
    return SPIFFS.open(filePath, mode);
}

void SPIFFSFileManager::closeFile(File file) {
    file.close();
}
