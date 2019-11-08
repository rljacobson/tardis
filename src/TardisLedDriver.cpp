//
// Created by Robert Jacobson on 10/22/19.
//

#include "TardisLedDriver.hpp"


namespace kaleidoscope::hardware::empros{

void TardisLedDriver::setup(){
    TardisLedDriverSuper::setup();
    LEDsChanged_ = false;
    duty_cycle_ = 1000;
}
void TardisLedDriver::setCrgbAt(int8_t i, cRGB crgb){

}
cRGB TardisLedDriver::getCrgbAt(int8_t i){
    return cRGB();
}
int8_t TardisLedDriver::getLedIndex(KeyAddr key_addr){
    return 0;
}

}
