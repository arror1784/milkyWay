#include "SDUtil.h"

String SDUtil::authenticationToken_;

void SDUtil::init() {
  bool sdStatus = SD.begin(5);
  if (!sdStatus) {
    while (1);
  }
}

bool SDUtil::writeFile(const String &downloadUrl, long id, const String &filename) {
  SD_MMC.begin();
  FILE *file = fopen(("/" + String(id) + "_" + filename).c_str(), "ab");

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
      write(fileno(file), audioData, availableDataSize);
      downloadedDataSize += availableDataSize;
      free(audioData);
    }
  }

  fclose(file);
  SD_MMC.end();

  return true;
}
