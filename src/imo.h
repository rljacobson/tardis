#pragma once

/*
This code was provided by iMo:
	https://www.eevblog.com/forum/microcontrollers/stm32-trying-to-get-spi-working-with-arduino-framework-for-fast-digitalwrite/
*/

#define BITBAND_SRAM_BASE        0x22000000
#define SRAM_START               0x20000000
#define BITBAND_PERIPHERAL_BASE  0x42000000
#define PERIPHERAL_START         0x40000000

#define BB_ALIAS(address, bit) ( *(volatile uint32*)( BITBAND_SRAM_BASE + ((uint32)(address)-SRAM_START) * 32 + (uint32)(bit)*4) )
#define BITBAND_PERIPHERAL(address, bit) *( (volatile uint32*)( BITBAND_PERIPHERAL_BASE + ((uint32)(address)-PERIPHERAL_START) * 32 + (uint32)(bit)*4) )
#define GPIO_ADDRESS(gpio) (GPIO_START + 0x400 * (uint32)((gpio)-'A'))
#define GPIO_OFFSET(gpioLetter) (0x400 * (uint32)((gpioLetter)-'A'))
#define BITBAND_GPIO_INPUT(gpioLetter, bit) BITBAND_PERIPHERAL((uint32)&(GPIOA_BASE->IDR)+GPIO_OFFSET(gpioLetter), (bit))
#define BITBAND_GPIO_OUTPUT(gpioLetter, bit) BITBAND_PERIPHERAL((uint32)&(GPIOA_BASE->ODR)+GPIO_OFFSET(gpioLetter), (bit))

#define ATOI2(s) ((s)[1] ? 10*(uint32)((s)[0]-'0') + (s)[1]-'0' : (uint32)((s)[0]-'0'))
#define XPIN_TO_GPIO(pin) (#pin[1])
#define XPARSE_PIN_BIT(pin) (ATOI2(#pin+2))
#define XCHECK_PIN_ID(pin) static_assert(#pin[0] == 'P' && #pin[1] >= 'A' && #pin[1] <= 'I' && #pin[2] >= '0' && #pin[2] <= '9' && \
    (#pin[3] == 0 || (#pin[3] >= '0' && #pin[3] <= '9' && #pin[4] == 0)) && ATOI2(#pin+2) < 16, "Invalid pin ")

#define DIGITAL_READ(pin) (BITBAND_GPIO_INPUT(XPIN_TO_GPIO(pin), XPARSE_PIN_BIT(pin)))
#define DIGITAL_WRITE(pin,value) do { XCHECK_PIN_ID(pin); (BITBAND_GPIO_OUTPUT(XPIN_TO_GPIO(pin), XPARSE_PIN_BIT(pin)))=(value); } while(0)

//port/gpio oriented macros (pins == bitmask of a bit set to 1)
#define IO_SET(port, pins)          port->regs->ODR |= (pins)       //set bits on port
#define IO_CLR(port, pins)          port->regs->ODR &=~(pins)       //clear bits on port

//fast routines through BRR/BSRR registers (pins == bitmask of a bit set to 1)
#define FIO_SET(port, pins)         port->regs->BSRR = (pins)
#define FIO_CLR(port, pins)         port->regs->BRR = (pins)
