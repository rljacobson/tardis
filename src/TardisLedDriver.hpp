//
// Created by Robert Jacobson on 10/22/19.
//

#pragma once

#ifdef ARDUINO_ARM_TARDIS

#ifndef CRGB
struct cRGB{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
#define CRGB(r, g, b) (cRGB){r, g, b}
#endif

#include "TardisKeyScannerDescription.hpp"      // KeyAddr typedef.
#include "kaleidoscope/driver/BaseLedDriver.h"
#include "TardisLedDriverDescription.hpp"
#include "AtomicBitfield.hpp"

namespace kaleidoscope::hardware::empros{

class Tardis; // Forward declaration.

typedef AtomicBitfield<148> LEDdata_t;
typedef kaleidoscope::driver::BaseLedDriver<TardisLedDriverDescription> TardisLedDriverSuper;

class TardisLedDriver: public TardisLedDriverSuper{
    friend class Tardis;
public:
    
    void setup();
    // For the Tardis, `syncLEDs` is a no-op.
    static void syncLeds(){ };
    static void setCrgbAt(int8_t i, cRGB crgb);
    static cRGB getCrgbAt(int8_t i);
    
    static int8_t getLedIndex(KeyAddr key_addr);
    
    // ToDo(rljacobson): Implement variable LED brightness.

private:
    
    // `duty_cycle_` is a number from 0 to 1000 and corresponds to the brightness of the LEDs.
    // TODO(rljacobson): Add support for different duty cycles for each LED.
    static unsigned int duty_cycle_;
    static bool LEDsChanged_;
    
    // These are initialized by the Tardis class.
    static LEDdata_t LEDState_;
    static uint8_t *LED_regs_;
    static int LEDFrequency_;
};

}

#endif
