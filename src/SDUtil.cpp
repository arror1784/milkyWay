#include "SDUtil.h"

#include <WString.h>
#include <FS.h>
#include <SD.h>
#include <HTTPClient.h>

String SDUtil::authenticationToken_;

const String SDUtil::wifiInfoPath_ = "/WIFI.json";
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
        Serial.println(SDUtil::serialPath_);
    }
    return _serial;
}

bool SDUtil::downloadFile(const String &api, int id, const String &filename) {
    String parsedFileName = String(filename);
    parsedFileName.replace(" ", "%20");

    String url = api + parsedFileName;

    Serial.println(url);

    SDUtil::authenticationToken_ = "eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiI4YmRiYWY1My1iZTUyLTRkYzUtYTVjNS0yMzVlMjBkMmQ0NDIiLCJ1c2VyUGsiOiI4YmRiYWY1My1iZTUyLTRkYzUtYTVjNS0yMzVlMjBkMmQ0NDIiLCJpYXQiOjE2Njk5ODU5ODAsImV4cCI6MTY3MjU3Nzk4MH0.FLhPj5wjBicNupGlv_BmUMaDYFcuY6kh_zuXzOrrxZk";

    HTTPClient httpClient;
    httpClient.begin(url);
    httpClient.setReuse(true);
    httpClient.addHeader("Authorization", String("Bearer ") + SDUtil::authenticationToken_);

    int httpCode = httpClient.GET();
    if (httpCode == HTTP_CODE_OK) {
        File file = SD.open(("/" + filename).c_str(), FILE_WRITE);

//        if(file.size() > 0) {
//            file.flush();
//        }

        int status = httpClient.writeToStream(&file);

        file.close();
        httpClient.end();
        if (status < 0) {
            Serial.println("http writeToStream fail : " + String(status));
            return false;
        }

        Serial.println("download file finish : " + String(status));

        return true;
    }
    Serial.println("http get fail : " + String(httpCode));
    httpClient.end();

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

