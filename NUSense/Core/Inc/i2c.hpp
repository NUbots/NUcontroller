/*
 * I2C.hpp
 *
 *  Created on: Jan 19, 2026
 */

#ifndef _I2C_HPP_
#define _I2C_HPP_

#include "main.h"


namespace nusense {
    class I2C {
        I2C() {
            // enable i2c1
            RCC->APB1LENR |= RCC_APB1LENR_I2C1EN; // enable i2c peripheral clock
            I2C1->CR1 |= I2C_CR1_PE; // enable i2c1

            // setup timing
            I2C1->TIMINGR |= I2C_TIMINGR_PRESC;
        }
    };
}


#endif /* _I2C_HPP_ */
