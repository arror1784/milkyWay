#ifndef MILKYWAY_SDCARD_H
#define MILKYWAY_SDCARD_H

#include "header.h"

class SDCard : public Singleton<SDCard> {
public:
  void init();
  bool writeFile(const String& path, const String& filename, uint8_t *data);
};


#endif //MILKYWAY_SDCARD_H
