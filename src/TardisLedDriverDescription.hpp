//
// Created by Robert Jacobson on 10/22/19.
//

#pragma once

#ifdef ARDUINO_ARM_TARDIS

#include <Arduino.h>

#include "kaleidoscope/driver/BaseLedDriverDescription.h"

namespace kaleidoscope::hardware::empros {

struct TardisLedDriverDescription : public kaleidoscope::driver::BaseLedDriverDescription {
  static constexpr LedCountType led_count = 148;
};

}

#endif
