#include "Pulser.hpp"
#include "stm32h753xx.h"

#ifndef DEVICE_BUZZER_HPP
    #define DEVICE_BUZZER_HPP

namespace device {

    /**
     * @brief   The buzzer which is a pulser.
     * @note    This is an exercise for the author in bare-metal programming with the SFRs. Let me
     *          know if there are any better C++ paradigms for SFRs.
     */
    class Buzzer : public Pulser {
    public:
        /**
         * @brief    Constructs the buzzer.
         * @param    port The reference to the port on which the buzzer is connected.
         * @param    pin The pin at which the buzzer is connected.
         */
        Buzzer(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin) {
            // Set the pin as an output.
            port->MODER |= (0b01 << (pin * 2));
            port->MODER &= ~(0b10 << (pin * 2));
        }

        /**
         * @brief   Destructs the buzzer.
         * @note    Nothing needs to be freed as of yet.
         */
        virtual ~Buzzer() {}

        /**
         * @brief   Turns the buzzer on.
         * @note    May introduce modulated frequencies later with a timer.
         */
        inline void turn_on() override {
            port->BSRR = 1 << pin;
            Pulser::turn_on();
        }

        /**
         * @brief   Turns the buzzer off.
         */
        inline void turn_off() override {
            port->BSRR = static_cast<uint32_t>(1 << (pin + 16));
            Pulser::turn_off();
        }

    private:
        /// @brief  The handler of the peripheral port.
        GPIO_TypeDef* port = NULL;
        /// @brief  The pin.
        uint16_t pin = 0;
    };

}  // namespace device

#endif  // DEVICE_BUZZER_HPP
