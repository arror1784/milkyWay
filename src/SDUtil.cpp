#include "SDUtil.h"

String SDUtil::authenticationToken_;

const String SDUtil::defaultColorSetsPath_ = "/lightEffect.json";

void SDUtil::init() {
    bool sdStatus = SD.begin(5);

    if (!sdStatus) {
        while (1);
    }
}

bool SDUtil::downloadFile(const String &api, int id, const String &filename) {
    String parsedFileName = String(filename);
    parsedFileName.replace(" ", "%20");

    String url = api + Util::decodeUrl(parsedFileName);

    Serial.println(url);

    HTTPClient httpClient;
    httpClient.begin(url);
    httpClient.setReuse(true);
    httpClient.addHeader("Authorization", String("Bearer ") + SDUtil::authenticationToken_);

    int httpCode = httpClient.GET();
    if (httpCode == HTTP_CODE_OK) {
        File file = SD.open(("/" + filename).c_str(), FILE_WRITE);

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

void SDUtil::listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

bool SDUtil::exists(const String &path) {
    File file = SD.open(path);
    return !!file;
}

