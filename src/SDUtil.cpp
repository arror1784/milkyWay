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

String SDUtil::getSerial() {
    if (_serial.length() == 0) {
        _serial = SDUtil::readFile(SDUtil::serialPath_);
        Serial.println("SDUtil::serialPath_ : " +SDUtil::serialPath_);
    }
    return _serial;
}

bool SDUtil::downloadFile(const String &api, long userId, const String &filename) {
    String parsedFileName = String(filename);
    parsedFileName.replace(" ", "%20");

    String url = api + parsedFileName;

    Serial.println(url);

    HTTPClient httpClient;
    httpClient.begin(url);
    httpClient.addHeader("Authorization", String("Bearer ") + authenticationToken_);

    int httpCode = httpClient.GET();
    if (httpCode == HTTP_CODE_OK) {
        File file = SD.open(("/" + String(userId) + "_" + filename).c_str(), FILE_WRITE);

        httpClient.writeToStream(&file);

        file.close();

        Serial.println("download file finish");
        return true;
    }
    Serial.println("http get fail : " + String(httpCode));
    return false;
}

bool SDUtil::writeFile(const String &path, const String &data) {
    File file = SD.open(path, FILE_WRITE);

    return file.write((uint8_t *) data.c_str(), data.length()) != 0;
}

String SDUtil::readFile(const String &path) {
    File file = SD.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("− failed to open file for reading");
        return "";
    }
    String res = file.readString();
    res.replace("\n", "");
    return res;
}

bool SDUtil::exists(const String &path) {
    File file = SD.open(path);
    return !!file;
}

