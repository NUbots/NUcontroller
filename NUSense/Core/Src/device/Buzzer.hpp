#include "stm32h753xx.h"

#ifndef DEVICE_BUZZER_HPP
    #define DEVICE_BUZZER_HPP

namespace device {

    /**
     * @brief   the buzzer
     * @note    This is an exercise for the author in bare-metal programming with the SFRs. Let me
     *          know if there are any better C++ paradigms for SFRs.
     */
    class Buzzer {
    public:
        /**
         * @brief    Constructs the buzzer.
         * @param    port the reference to the port on which the buzzer is connected,
         * @param    pin at which the buzzer is connected,
         */
        Buzzer(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin) {
            // Set the resistors to pull up.
            port->PUPDR &= ~(0b11 << (pin * 2));
            port->PUPDR |= (0b01 << (pin * 2));
            // Set the pin as an output.
            port->MODER |= (0b01 << (pin * 2));
            port->MODER &= ~(0b10 << (pin * 2));
        }

        /**
         * @brief   Destructs the buzzer.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~Buzzer() {}

        /**
         * @brief   Turns the buzzer on.
         * @note    May introduce beaps and modulated frequencies later.
         */
        inline void turn_on() {
            port->BSRR = 1 << pin;
        }

        /**
         * @brief   Turns the buzzer off.
         */
        inline void turn_off() {
            port->BSRR = static_cast<uint32_t>(1 << (pin * 2));
        }

    private:
        /// @brief  the handler of the peripheral port,
        GPIO_TypeDef* port;
        /// @brief  the pin,
        uint16_t pin;
    };

}  // namespace device

#endif  // DEVICE_BUZZER_HPP
