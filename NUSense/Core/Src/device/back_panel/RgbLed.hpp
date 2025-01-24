#include "../Pulser.hpp"
#include "Led.hpp"
#include "stm32h753xx.h"

#ifndef DEVICE_BACK_PANEL_RGBLED_HPP
    #define DEVICE_BACK_PANEL_RGBLED_HPP

namespace device::back_panel {

    /**
     * @brief   The RGB-LED which is a pulser.
     * @note    This is an exercise for the author in bare-metal programming with the SFRs. Let me
     *          know if there are any better C++ paradigms for SFRs.
     * @note    For now, I have settled on basic 3-bit RGB. This will be a placeholder for either
     *          bitbanged PWM in a loop or a new PCB revision with the RGB LEDs rerouted to PWM
     *          timers.
     */
    class RgbLed : public Pulser {
    public:
        /**
         * @brief    Constructs the RGB-LED.
         * @param    leds an array of the three LEDs, red, green, and blue.
         */
        RgbLed(std::array<Led, 3> leds) : leds(leds) {}

        /**
         * @brief   Destructs the RGB-LED.
         * @note    Nothing needs to be freed as of yet.
         */
        virtual ~RgbLed() {}

        /**
         * @brief   Sets the RGB value.
         * @param   value The RGB value.
         */
        void set_value(const uint32_t value) {
            values[0] = static_cast<uint8_t>(value >> 16);
            values[1] = static_cast<uint8_t>(value >> 8);
            values[2] = static_cast<uint8_t>(value >> 0);
        }
        void set_value(std::array<uint8_t, 3>& value) {
            values = value;
        }

        /**
         * @brief   Turns the RGB-LED on.
         */
        inline void turn_on() override {
            // For each LED, either turn it on or off depending on its value.
            for (size_t i = 0; i < 3; i++) {
                // For now, we will have to live (and suffer) with dumb 3-bit RGB until either the
                // RGB LEDs are rerouted to PWM timers or someone can be bothered to bitbang PWM in
                // a loop (which might be a topic for another PR later).

                // Mom, can we have RGB?
                // No, there is RGB at home.
                // At home:

                // If the value is greater than 127, then turn it on.
                values[i] > 127 ? leds[i].turn_on() : leds[i].turn_off();
            }

            Pulser::turn_on();
        }

        /**
         * @brief   Turns the RGB-LED off.
         */
        inline void turn_off() override {
            // Turn each led off.
            for (auto& led : leds) {
                led.turn_off();
            }

            Pulser::turn_off();
        }

    private:
        /// @brief  The red, green, and blue values.
        std::array<uint8_t, 3> values;
        /// @brief  The red, green, and blue leds.
        std::array<Led, 3> leds;
    };

}  // namespace device::back_panel

#endif  // DEVICE_BACK_PANEL_RGBLED_HPP
