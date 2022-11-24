#ifndef ESP32_LIB_NEOPIXEL_H
#define ESP32_LIB_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <map>

#include "Util.h"
#include "SimpleTimer.h"

enum class EDimmingStatus {
  UP, DOWN
};

class ColorSet {
public:
  long id;
  std::vector<uint32_t> colors;
};

class LightEffect {
public:
  long id;
  std::map<long, ColorSet *> colorSets;
  ELightMode mode = ELightMode::Blinking;
  bool isRandomColor;
  long speed;
  bool isRandomSpeed;
};

class NeoPixel {
public:
  // 최대 밝기로 킨다.
  void on();

  // 끈다.
  void off();

  // 발기를 낮춘다, 0에 도달하면 디밍 상태를 UP으로 바꾼다.
  void lowerBrightness();

  // 밝기를 높힌다. 최대 밝기에 도달하면 디빙 상태를 DOWN으로 바꾼다.
  void increaseBrightness();

  // 현재 최대 밝기인지 확인한다.
  bool isOn() const;

  // 사용할 빛 효과가 유효한지 검증한다.
  bool isCurrentLightEffectValid();

  // 사용할 빛 효과를 반환한다.
  const LightEffect &getCurrentLightEffect();

  // 디밍 상태를 반환한다.
  EDimmingStatus getDimmingStatus() const;

  void setLightEffectId(int lightEffectId);

  void setColorSetId(int colorSetId);

  void setLightEffects(const JsonArray &jsonArray);

  // 싱글톤 객체 레퍼런스를 반환한다.
  static NeoPixel &getInstance() {
    static NeoPixel instance(ledCount_, pin_);

    return instance;
  }

  static void setPin(int16_t pin);

  static void setLedCount(int ledCount);

private:
  NeoPixel(uint16_t nLed, int16_t pin);

  void updatePixelColor();

  int _lightEffectId = 0;
  int _colorSetId = 0;
  EDimmingStatus _dimmingStatus = EDimmingStatus::UP;

  Adafruit_NeoPixel _strip;
  std::map<long, LightEffect *> _lightEffects;

  static int16_t pin_;
  static int ledCount_;
  static const int maxBright_;
};

#endif //ESP32_LIB_NEOPIXEL_H