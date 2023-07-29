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
    String url = api + Util::encodeUrl(parsedFileName);

    SERIAL_PRINTLN(url);

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
            SERIAL_PRINTLN("http writeToStream fail : " + String(status));
            return false;
        }

        SERIAL_PRINTLN("download file finish : " + String(status));

        return true;
    }
    SERIAL_PRINTLN("http get fail : " + String(httpCode));
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
        SERIAL_PRINTLN("− failed to open file for reading");
        return "";
    }
    String res = file.readString();
    res.replace("\n", "");
    return res;
}

void SDUtil::listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    SERIAL_PRINT(String("Listing directory: ") + dirname);

    File root = fs.open(dirname);
    if (!root) {
        SERIAL_PRINTLN("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        SERIAL_PRINTLN("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            SERIAL_PRINT("  DIR : ");
            SERIAL_PRINTLN(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else {
            SERIAL_PRINT("  FILE: ");
            SERIAL_PRINT(file.name());
            SERIAL_PRINT("  SIZE: ");
            SERIAL_PRINTLN(file.size());
        }
        file = root.openNextFile();
    }
}

bool SDUtil::exists(const String &path) {
    File file = SD.open(path);
    return !!file;
}

bool SDUtil::deleteFile(const String &path) {
    return SD.remove("/" + path);
}

