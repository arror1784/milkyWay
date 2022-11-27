#include "SDUtil.h"

#include <WString.h>
#include <FS.h>
#include <SD.h>
#include <HTTPClient.h>

String SDUtil::authenticationToken_;

const String SDUtil::wifiInfoPath_ = "/WIFI";
const String SDUtil::serialPath_ = "/SERIAL";

void SDUtil::init() {
    bool sdStatus = SD.begin(5);
    if (!sdStatus) {
        while (1);
    }
}

String SDUtil::getSerial(){
  if(_serial.length() == 0){
    _serial = SDUtil::readFile(SDUtil::serialPath_);
    Serial.println("Asdasd");
    Serial.println(SDUtil::serialPath_);
  }
  return _serial;
}


bool SDUtil::downloadFile(const String &downloadUrl, long id, const String &filename) {
    // FILE file = fopen(("/" + String(id) + "_" + filename).c_str(), "ab");
    File file = SD.open(("/" + String(id) + "_" + filename).c_str(), FILE_WRITE);

    HTTPClient httpClient;
    httpClient.addHeader("Authorization", String("Bearer ") + authenticationToken_);
    httpClient.begin(downloadUrl);
    WiFiClient *stream = httpClient.getStreamPtr();

    // Download data and write into SD card
    size_t downloadedDataSize = 0;
    const size_t audioSize = httpClient.getSize();
    while (downloadedDataSize < audioSize) {
        size_t availableDataSize = stream->available();
        if (availableDataSize > 0) {
            auto *audioData = (uint8_t *) malloc(availableDataSize);
            stream->readBytes(audioData, availableDataSize);
            // write(fileno(file), audioData, availableDataSize);
            file.write(audioData, availableDataSize);
            downloadedDataSize += availableDataSize;
            free(audioData);
        }
    }

   file.close();
    
    return true;
}
bool SDUtil::writeFile(const String &path,const String &data){
    File file = SD.open(path,FILE_WRITE);

    if(file.write((uint8_t*)data.c_str(),data.length()) == 0)
      return false;
    else return true;

}

String SDUtil::readFile(const String &path) {
    
    File file = SD.open(path);
    if(!file || file.isDirectory()){
        Serial.println("− failed to open file for reading");
        return "";
    }
    return file.readString();
}

bool SDUtil::exists(const String &path) {
    return exists(path);
}

