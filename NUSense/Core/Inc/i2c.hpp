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

            RCC->D2CCIP2R |= (0x3 << RCC_D2CCIP2R_I2C123SEL_Pos);  // set I2C3 clock to CSI
            RCC->APB1LENR |= RCC_APB1LENR_I2C3EN;                  // enable i2c peripheral clock
            RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;                   // enable GPIOA clock
            RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;                   // enable GPIOC clock

            // configure scl pin as open-drain pull up
            GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;              // clear pin 8 mode bits
            GPIOA->MODER |= GPIO_MODER_MODE8_1;                 // alternate function (input+output)
            GPIOA->OTYPER |= GPIO_OTYPER_OT8;                   // open-drain
            GPIOA->PUPDR |= GPIO_PUPDR_PUPD8_0;                 // pull up
            GPIOA->AFR[1] |= (0b0100 << GPIO_AFRH_AFSEL8_Pos);  // set alternate function to i2c (I2C3_SCL)

            // configure sda pin as open-drain pull up
            GPIOC->MODER &= ~GPIO_MODER_MODE9_Msk;              // clear pin 9 mode bits
            GPIOC->MODER |= GPIO_MODER_MODE9_1;                 // alternate function (input+output)
            GPIOC->OTYPER |= GPIO_OTYPER_OT9;                   // open-drain
            GPIOC->PUPDR |= GPIO_PUPDR_PUPD9_0;                 // pull up
            GPIOC->AFR[1] |= (0b0100 << GPIO_AFRH_AFSEL9_Pos);  // set alternate function to i2c (I2C3_SCL)

            // configure i2c
            I2C3->CR1 &= ~I2C_CR1_PE;  // clear PE bit to allow configuring

            // i2c timing
            I2C3->TIMINGR |= 0x1 << I2C_TIMINGR_PRESC_Pos;   // set prescale to 2 (period: 0.5 us/500 ns)
            I2C3->TIMINGR |= 0x0 << I2C_TIMINGR_SCLDEL_Pos;  // data setup time: 0.5 us - (0+1) x 0.5us
            I2C3->TIMINGR |= 0x1 << I2C_TIMINGR_SDADEL_Pos;  // data hold time: 0.5 us - 1 x 0.5us
            I2C3->TIMINGR |= 0x8 << I2C_TIMINGR_SCLH_Pos;    // set scl high period to 4.5 us - (8+1) x 0.5us
            I2C3->TIMINGR |= 0x9 << I2C_TIMINGR_SCLL_Pos;    // set sda high period to 5.0 us - (9+1) x 0.5us

            // enable i2c
            I2C3->CR1 |= I2C_CR1_PE;  // enable I2C3
        }

        uint32_t testSend() {
            I2C3->CR2 = 0;                               // clear CR2 to reset configuration
            I2C3->CR2 |= I2C_CR2_AUTOEND;                // automatically send stop after data transmit finished
            I2C3->CR2 |= 0x8 << (I2C_CR2_SADD_Pos + 1);  // set slave address to 0x8 (battery controller)
            I2C3->CR2 |= 1 << I2C_CR2_NBYTES_Pos;        // set number of bytes to 1
            I2C3->CR2 |= I2C_CR2_START;                  // generate start condition

            while (!(I2C3->ISR & I2C_ISR_TXE)) {
            } // 

            I2C3->TXDR = 0x07;  // send register address of SYS_STAT + bit 7 (CC_READY bit)

            uint32_t ret = I2C3->ISR;

            return ret;
        }
    };
}  // namespace nusense


#endif /* _I2C_HPP_ */
