#include "stm32h753xx.h"

#ifndef UTILITY_SUPPORT_BUTTON_HPP
    #define UTILITY_SUPPORT_BUTTON_HPP

namespace utility::support {

    /**
     * @brief   the buzzer
     * @note    This is an exercise for the author in bare-metal programming with the SFRs. Let me
     *          know if there are any better C++ paradigms for SFRs.
     */
    class Button {
    public:
        /**
         * @brief    Constructs the button.
         * @param    port the reference to the port on which the button is connected,
         * @param    pin at which the button is connected,
         */
        Button(GPIO_TypeDef* port, uint16_t pin)
            : port(port), pin(pin), is_pressed(false), n_highs(0), n_lows(0), threshold(10) {
            // Set the resistors to pull up.
            port->PUPDR &= ~(0b11 << (pin * 2));
            port->PUPDR |= (0b01 << (pin * 2));
            // Set the pin as an input.
            port->MODER &= ~(0b11 << (pin * 2));
        }

        /**
         * @brief   Destructs the button.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~Button() {}

        /**
         * @brief   Polls the pin.
         * @return  whether the pin is high or low,
         */
        inline bool read() {
            return ((port->IDR & (0x0001 << pin)) != 0);
        }

        /**
         * @brief   Handles the debouncing filter.
         * @note    To be honest, I am not fully sure on how this works, some magic with which Trent
         *          came up. Better suggestions for debouncing are welcome.
         * @return  whether the button has just been pressed, i.e. a single shot of the rising
         *          active edge.
         */
        bool filter() {
            // The pin's read state has been inverted given that the button pulls down.
            n_highs = std::max(0, std::min(threshold * 2, n_highs + (!read() ? 1 : -1)));
            n_lows  = std::max(0, std::min(threshold * 2, n_lows + (!read() ? -1 : 1)));

            bool was_pressed = is_pressed;

            // I changed the threshold to have hysteresis since the two if-conditions were both
            // true at the same time, and the is_pressed was thus always false.
            if (n_highs == 4 * threshold / 3) {
                is_pressed = true;
            }
            else if (n_lows == 4 * threshold / 3) {
                is_pressed = false;
            }

            // This is to make the press one-shot, i.e. returns true only once for each distinct
            // press.
            return is_pressed && !(was_pressed);
        }

    private:
        /// @brief  the handler of the peripheral port,
        GPIO_TypeDef* port;
        /// @brief  the pin,
        uint16_t pin;
        /// @brief  the debounced state, i.e. whether it has been fully pressed,
        bool is_pressed;
        /// @brief  the number of highs counted,
        uint16_t n_highs;
        /// @brief  the number of lows counted,
        uint16_t n_lows;
        /// @brief  th threshold for debouncing,
        uint16_t threshold;
    };

}  // namespace utility::support

#endif  // UTILITY_SUPPORT_BUTTON_HPP
