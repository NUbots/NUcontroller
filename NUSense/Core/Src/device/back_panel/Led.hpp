#include "../Pulser.hpp"
#include "stm32h753xx.h"

#ifndef DEVICE_BACK_PANEL_LED_HPP
    #define DEVICE_BACK_PANEL_LED_HPP

namespace device::back_panel {

    /**
     * @brief   The LED which is a pulser.
     * @note    This is an exercise for the author in bare-metal programming with the SFRs. Let me
     *          know if there are any better C++ paradigms for SFRs.
     */
    class Led : public Pulser {
    public:
        /**
         * @brief    Constructs the LED.
         * @param    port The reference to the port on which the buzzer is connected.
         * @param    pin The pin at which the buzzer is connected.
         */
        Led(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin) {
            // Set the pin as an output.
            port->MODER |= (0b01 << (pin * 2));
            port->MODER &= ~(0b10 << (pin * 2));
            // Set the output to be open-drain since the LED is electrically pulled high; see the
            // schematic:
            // https://github.com/ROBOTIS-GIT/ROBOTIS-OP-Series-Data/blob/master/ROBOTIS-OP%2C%20ROBOTIS-OP2/Hardware/Electronics/Boards/DARwIn-OP_Interface_rev3.pdf
            port->OTYPER |= (0b1 << pin);
            // Set the pin to high at first.
            port->BSRR = 1 << pin;
        }

        /**
         * @brief   Destructs the LED.
         * @note    Nothing needs to be freed as of yet.
         */
        virtual ~Led() {}

        /**
         * @brief   Turns the LED on.
         */
        inline void turn_on() override {
            // The pin is reset since the LED is electrically pulled high.
            port->BSRR = static_cast<uint32_t>(1 << (pin + 16));
            Pulser::turn_on();
        }

        /**
         * @brief   Turns the LED off.
         */
        inline void turn_off() override {
            // The pin is set since the LED is electrically pulled high.
            port->BSRR = 1 << pin;
            Pulser::turn_off();
        }

    private:
        /// @brief  The handler of the peripheral port.
        GPIO_TypeDef* port = NULL;
        /// @brief  The pin.
        uint16_t pin = 0;
    };

}  // namespace device::back_panel

#endif  // DEVICE_BACK_PANEL_LED_HPP
