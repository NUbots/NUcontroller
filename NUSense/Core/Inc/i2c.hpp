/*
 * I2C.hpp
 *
 *  Created on: Jan 19, 2026
 */

#ifndef _I2C_HPP_
#define _I2C_HPP_

#include "stm32h7xx.h"

/*
sda is on port c pin 9
scl is on port a pin 8
i2c initialisation:
- I2C_CR1: clear PE bit
- I2C_CR1: configure anfoff and dnf[3:0]
- I2C_TIMINGR: configure PRESC[3:0], SDADEL[3:0], SCLDEL[3:0], SCLH[7:0], SCLL[7:0]
- I2C_CR1: configure NOSTRETCH
- I2C_CR1: set PE bit
*/

namespace nusense {
    class I2C {
    public:
        I2C() {
            // configure csi
            RCC->CR |= RCC_CR_CSION;  // enable csi
            while (!(RCC->CR & RCC_CR_CSIRDY)) {
            }  // wait for CSI to be ready

            RCC->D2CCIP2R |= (0x3 << RCC_D2CCIP2R_I2C123SEL_Pos);  // set i2c1 clock to CSI
            RCC->APB1LENR |= RCC_APB1LENR_I2C1EN;                  // enable i2c peripheral clock

            // configure i2c
            I2C1->CR1 &= ~I2C_CR1_PE;  // clear PE bit to allow configuring

            // i2c timing
            I2C1->TIMINGR |= (0x1 << I2C_TIMINGR_PRESC_Pos);   // set prescale to 2 (period: 0.5 us/500 ns)
            I2C1->TIMINGR |= (0x0 << I2C_TIMINGR_SCLDEL_Pos);  // data setup time: 0.5 us - (0+1) x 0.5us
            I2C1->TIMINGR |= (0x1 << I2C_TIMINGR_SDADEL_Pos);  // data hold time: 0.5 us - 1 x 0.5us
            I2C1->TIMINGR |= (0x8 << I2C_TIMINGR_SCLH_Pos);    // set scl high period to 4.5 us - (8+1) x 0.5us
            I2C1->TIMINGR |= (0x9 << I2C_TIMINGR_SCLL_Pos);    // set sda high period to 5.0 us - (9+1) x 0.5us

            // enable i2c
            I2C1->CR1 |= I2C_CR1_PE;  // enable i2c1
        }
    };
}  // namespace nusense


#endif /* _I2C_HPP_ */
