/* -*- mode: c++ -*-
 * Empros Systems International's Operation Keyboard support for Kaleidoscope.
 * Copyright (C) 2019 Robert Jacobson
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */


//
// Created by Robert Jacobson on 10/18/19.
//

// #include "spi_constants.hpp"
#include "Tardis.hpp"
#include "imo.h"

// Which bits need to be set for setting/resetting a pin using BSSR.
#define setBSSR(n) (1UL << n)
#define resetBSSR(n) (1UL << (n + 16))
// Setting/resetting a pin on GPIO Port B using BSSR.
#define setBB(n) BITBAND_GPIO_OUTPUT('B', n)
#define getBB(n) BITBAND_GPIO_INPUT('B', n)
#define setB(n)                             \
    do {                                    \
        BITBAND_GPIO_OUTPUT('B', n) = 1U;   \
        /*GPIOB->regs->BSRR = setBSSR(n);*/ \
    } while(false)
#define resetB(n)                      \
    do {                               \
        GPIOB->regs->BRR = setBSSR(n); \
    } while(false)
#define writeBSSR(bitmask)           \
    do {                             \
        GPIOB->regs->BSRR = bitmask; \
    } while(false)

namespace kaleidoscope::hardware::empros{

void Tardis::setup(){
    TardisSuper::setup();
    pinMode(PB6, PWM);              // SCK
    pinMode(PB12, OUTPUT);          // NSS
    pinMode(PB14, INPUT_PULLDOWN);  // MISO
    pinMode(PB15, OUTPUT);          // MOSI

    // Enable the APB2 bus clock.
    // ToDo: Check if this is necessary. (Probably not.)
    RCC_BASE->APB2ENR |= 1U;
    // Reroute the EVENT signal (SEV instruction) to pin PB6.
    AFIO_BASE->EVCR = AFIO_EVCR_EVOE | AFIO_EVCR_PORT_PB | AFIO_EVCR_PIN_6;

    setB(NSS);
    
    in_buffer = TardisKeyScanner::keyState_;
}  // end Tardis::setup()

/**
 * @brief Performs the necessary hardware diddling to read the status of each key button.
 */
void Tardis::readMatrix() __attribute__((optimize("-O0"))) {
    // The complete details of this function are described in [Notes.md](Notes.md). This code
    // will be mysterious without an understanding of the signal timing requirements and bit
    // pattern computation.

    uint32_t key_index = 0;  // Index into current bit of `in_buffer`, which is also key address.
    uint32_t prev_key_index =
        127;       // We read the key button for the previous key index after we need the next key index.
    uint32_t val;  // Register storage for each 16-bit value to write out to shift register.


    for(key_index = 0; key_index < 128; key_index++){

        resetB(NSS);  // Bring NSS (the latch) low. (Has no effect on the output of the shift register.)

        // Compute the bit pattern for `key_index` for columns 0-15.
        val = (key_index << 4U) | 0b0100U;
        asm volatile("rbit %[aVal], %[aVal]" : [aVal] "+r"(val) : "[aVal]"(val));
        val >>= 16U;

        // Alternate MOSI and strobing clock for the remaining bits. The `nop`'s are needed to
        // get the signal timing right.
        setBB(MOSI) = val;
        asm volatile("nop");  // TOK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("nop");  // TOK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("sev");  // CLK
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("sev");  // CLK
        // ToDo: Measure benefit of filling these no-ops with some of the computation below.

        // Read MISO
        BB_ALIAS(in_buffer, prev_key_index | 128) = getBB(MISO);  // Read MISO into `in_buffer`

        setB(NSS);  // Bring NSS back up to set the latch.

        // Compute the bit pattern for `key_index` for columns 16-31. (Magic.)
        val = (key_index << 4) & 0x00ffU | 0b0010U | (key_index << 9);
        asm volatile("rbit %[aVal], %[aVal]" : [aVal] "+r"(val) : "[aVal]"(val));
        val >>= 16;

        resetB(NSS);  // Bring NSS (the latch) low. (Has no effect on the shift register output.)

        // Alternate MOSI and strobing clock for the remaining bits.
        setBB(MOSI) = val;
        asm volatile("nop");
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("nop");
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        val >>= 1;
        setBB(MOSI) = val;
        asm volatile("sev");  // CLK
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("sev");  // CLK
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("nop");  // TOk
        asm volatile("sev");  // CLK

        // Read MISO
        BB_ALIAS(in_buffer, prev_key_index) = getBB(MISO);  // Read MISO into `in_buffer`

        setB(NSS);  // Bring NSS back up to set latch.

        // Finally increment `key_index`
        prev_key_index = key_index;              // Save old index
    }

}

} // end namespace
