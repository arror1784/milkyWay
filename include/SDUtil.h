#ifndef MILKYWAY_SDUTIL_H
#define MILKYWAY_SDUTIL_H

#include <WString.h>
#include <SD.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <HTTPClient.h>

#include "Singleton.h"

class SDUtil {
public:
  static void init();

  static bool writeFile(const String &downloadUrl, long id, const String &filename);
};

#endif //MILKYWAY_SDUTIL_H
