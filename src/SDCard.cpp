#include "SDCard.h"

void SDCard::init() {
  bool sdStatus = SD.begin(5);
  if (!sdStatus) {
    while (1);
  }
}

bool SDCard::writeFile(const String& path, const String& filename, uint8_t *data) {
  File file = SD_MMC.open(path + filename, FILE_WRITE);
  if (!file) {
    return false;
  }
  file.print(*data);
  file.close();
  return true;
}
