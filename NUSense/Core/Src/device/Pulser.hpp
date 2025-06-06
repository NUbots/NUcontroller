#ifndef DEVICE_PULSER_HPP
#define DEVICE_PULSER_HPP

#include "../utility/support/MillisecondTimer.hpp"

namespace device {

    /// @brief  The half period of a pulse in milliseconds.
    static constexpr uint16_t HALFPULSE_PERIOD = 500;
    /// @brief  The time between burst of pulses in milliseconds if the pulser is repeating.
    static constexpr uint16_t DEAD_PERIOD = 2000;
    /*

             |<---->|                                   |<------- . . . ------->|
             Halfpulse                                         Dead period


    XX        ------        ------                ------                         ----
    XX\ off  /  on  \      /      \  . . .       /      \         . . .         /     . . .
    XX ------        ------        -       ------        --------       --------

       |<---------->|<----------->|       |<----------->|
          1st pulse    2nd pulse              N-th pulse

       |<--------------------------- . . . ------------>|                       |<--- . . .
                                Burst                                           Repeated burst
    */

    /**
     * @brief   The pulser.
     */
    class Pulser {
    public:
        /// @brief  The levels priority of the pulsing routine if any.
        enum Priority { NONE = 0, LOW = 1, MID = 2, HIGH = 3 };

        /**
         * @brief    Constructs the pulser.
         */
        Pulser() {}

        /**
         * @brief   Destructs the pulser.
         * @note    Nothing needs to be freed as of yet.
         */
        virtual ~Pulser() {}

        /**
         * @brief   Gets the current priority.
         */
        inline Priority get_current_priority() const {
            return current_priority;
        }

        /**
         * @brief   Pulses the device.
         * @note    This should ideally not be overridden.
         * @note    The priority is a basic way to stop calls overwriting important pulses like
         *          the one for overheating. It does not queue or shelved tasks.
         * @param   num_pulses The number of pulses to show.
         * @param   is_repeating Whether the burst of pulses is repeating with some dead time in
         *          between each.
         * @return  Whether the new pulsing task has been taken in, i.e. the priority is high
         *          enough.
         */
        inline bool pulse(const uint8_t num_pulses    = 1,
                          const bool is_repeating     = false,
                          const Priority new_priority = LOW) {
            // If the wanted priority is less than the current priority, then return early.
            if (new_priority < current_priority) {
                return false;
            }
            current_priority = new_priority;

            // Begin the timer for the first half-pause, i.e. the off-period.
            halfpulse_timer.begin(HALFPULSE_PERIOD);
            turn_off();

            // Store the number of pulses in a single burst. This will behave as a counter for how
            // ever many pulses are left.
            num_pulses_left = num_pulses;

            // If repeating, then keep the total number of pulses for the next burst.
            if (is_repeating) {
                num_pulses_in_burst = num_pulses;
            }
            else {
                num_pulses_in_burst = 0;
            }

            return true;
        }

        /**
         * @brief   Turns the pulser on which just stays on continuously without pulses.
         * @note    This function should be expanded further in the derived class with more
         *          hardware-specific code.
         */
        virtual inline void turn_on() {
            is_on = true;
        }

        /**
         * @brief   Turns the pulser off.
         * @note    This function should be expanded further in the derived class with more
         *          hardware-specific code.
         */
        virtual inline void turn_off() {
            is_on = false;
        }

        /**
         * @brief   Handles the current pulsing task if any.
         * @note    This should ideally not be overridden.
         */
        void handle() {
            // If there are still pulses left to do, and the half-pause has timed out, then toggle
            // the output.
            if ((num_pulses_left != 0) && (halfpulse_timer.has_timed_out())) {
                // Toggle the output.
                if (is_on) {
                    // If this is a falling edge, then a full pulse has been done, and decrease the
                    // number of pulses left.
                    num_pulses_left--;
                    turn_off();
                }
                else {
                    turn_on();
                }

                // If there are still pulses left, then time the next half-pulse.
                if (num_pulses_left != 0) {
                    halfpulse_timer.begin(HALFPULSE_PERIOD);
                }
                // Else, if it is repeating, then time both the dead period and the first 
                // half-pulse and repeat the task.
                else if (num_pulses_in_burst != 0) {
                    num_pulses_left = num_pulses_in_burst;
                    halfpulse_timer.begin(DEAD_PERIOD + HALFPULSE_PERIOD);
                }
                // Else, then there is no longer a current pulsing task. Reset the priority and do
                // nothing.
                else {
                    current_priority = NONE;
                }
            }
        }

        /**
         * @brief   Stops the current pulsing task if any.
         */
        void stop() {
            halfpulse_timer.stop();
            num_pulses_left = 0;
            num_pulses_in_burst = 0;
            turn_off();
            current_priority = NONE;
        }

    private:
        /// @brief  The state of the device, i.e. the output.
        bool is_on = false;
        /// @brief  Whether the current pulsing task has repeated bursts.
        bool is_repeating = false;
        /// @brief  The priority of the current pulsing task lest an important signal, e.g.
        ///         the over-temperature alarm, is overridden.
        /// @note   This is also used as a simple flag to know whether there is a current pulsing
        ///         task.
        Priority current_priority = NONE;
        /// @brief  The number of pulses left to show on the device.
        /// @note   Behaves as a counter from which to decrease.
        /// @note   A pulse is both a on-period and an off-period.
        uint8_t num_pulses_left = 0;
        /// @brief  The total number of pulses in a burst.
        /// @note   Unlike num_pulses_left, stores a fixed value to which to refer back.
        /// @note   If this is set to zero, then the pulser does not repeat, and there is only one
        ///         burst.
        uint8_t num_pulses_in_burst = 0;
        /// @brief  The timer to time the half-pulses.
        /// @note   A half-pulse is either a single on-period or a single off-period.
        utility::support::MillisecondTimer halfpulse_timer{};
    };

}  // namespace device

#endif  // DEVICE_PULSER_HPP
