#include "tim.h"

#include "signal.h"

#ifndef UTILITY_SUPPORT_MICROSECONDTIMER_HPP
#define UTILITY_SUPPORT_MICROSECONDTIMER_HPP

namespace utility::support {

    /**
     * @brief   the timer in microseconds
     * @note    The timeout may be better handled with interrupts and callbacks than polling in the 
     *          executive loop, but that may mean long ISRs. We may have to see if the polling
     *          adds too much latency. Any ideas are welcome.
     */
    class MicrosecondTimer {
    public:
        /**
         * @brief    Constructs the timer.
         * @param    htim the reference to the timer to be counted,
         */
        MicrosecondTimer(TIM_HandleTypeDef* htim = &htim4) :
            htim(htim)
        {
            threshold = 0;
            is_counting = false;
        };
        /**
         * @brief   Destructs the timer.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~MicrosecondTimer() {};

        /**
         * @brief   Begins the timer.
         * @param   timeout the timeout in microseconds, at most 65535,
         * @return  whether the timer is ready, i.e. is not currently counting,
         */
        bool begin(uint16_t timeout) {
            // If the timer is already counting, then return false before changing anything.
            if  (is_counting)
                return false;

            // Get the current tick and calculate the necessary threshold.
            uint16_t first_tick = __HAL_TIM_GET_COUNTER(htim);
            threshold = first_tick + timeout;
            // The 16-bit overflow should handle wrapping.

            is_counting = true;

            return true;
        }

        /**
         * @brief   Stops the timer.
         */
        void stop() {
            is_counting = false;
        }

        /**
         * @brief   Sees whether the timer has timed out.
         * @note    This should be kept very short.
         * @return  whether the timer has timed out,
         */
        inline bool has_timed_out() {
            // Copy the count-register of the timer so that it is stable for debugging.
            uint16_t count = __HAL_TIM_GET_COUNTER(htim);
            if ((is_counting) && (count > threshold)) {
                // This is a very hacky way to ignore overflowing on the sharp edge of the
                // saw-tooth wave.
                if (!((count - threshold) & 0x8000)) {
                    // Since the timer has timed out, it is no longer counting.
                    is_counting = false;
                    return true;
                } else
                    return false;
            } else
                return false;
        }

    private:
        /// @brief  the handler of the peripheral timer,
        TIM_HandleTypeDef* htim;
        /// @brief  the threshold to compare the count thereagainst,
        uint16_t threshold;
        /// @brief  whether the timer is in use,
        bool is_counting;

    };

} // namespace utility::support

#endif // UTILITY_SUPPORT_MICROSECONDTIMER_HPP
