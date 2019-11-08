//
// Created by Robert Jacobson on 10/22/19.
//


#ifdef ARDUINO_ARM_TARDIS

#include "Arduino.h"

#include "kaleidoscope/driver/BaseKeyScannerDescription.h"

namespace kaleidoscope::hardware::empros {

struct TardisKeyScannerDescription : kaleidoscope::driver::BaseKeyScannerDescription {
  KEYSCANNER_DESCRIPTION(8, 32)
};

}

typedef kaleidoscope::hardware::empros::TardisKeyScannerDescription::KeyAddr KeyAddr;

#endif
