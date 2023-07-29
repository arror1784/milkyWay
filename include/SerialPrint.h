#ifndef MILKYWAY_SERIALPRINT_H
#define MILKYWAY_SERIALPRINT_H

#include <WString.h>

#ifdef DEBUG
    #define SERIAL_PRINTLN(x) Serial.println(x)
    #define SERIAL_PRINT(x) Serial.print(x)
#else
    #define SERIAL_PRINTLN(x)
    #define SERIAL_PRINT(x)
#endif

#endif //MILKYWAY_SERIALPRINT_H
