#include "signal.h"
#include "tim.h"

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
        MicrosecondTimer(TIM_HandleTypeDef* htim = &htim4) : htim(htim), start(0), timeout(0), is_counting(false) {}

        /**
         * @brief   Destructs the timer.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~MicrosecondTimer() {}

        /**
         * @brief   Begins the timer.
         * @param   timeout the timeout in microseconds, at most 65535,
         * @return  whether the timer is ready, i.e. is not currently counting,
         */
        bool begin(uint16_t input_timeout) {
            // If the timer is already counting, then return false before changing anything.
            if (is_counting)
                return false;

            // Get the current tick.
            start       = __HAL_TIM_GET_COUNTER(htim);
            timeout     = input_timeout;
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
         * @brief   Restarts the timer.
         * @param   timeout the timeout in microseconds, at most 65535.
         */
        void restart(uint16_t input_timeout) {
            stop();
            begin(input_timeout);
        }

        /**
         * @brief   Sees whether the timer has timed out.
         * @note    This should be kept very short.
         * @return  whether the timer has timed out,
         */
        inline bool has_timed_out() {
            // Copy the count-register of the timer so that it is stable for debugging.
            uint16_t count = __HAL_TIM_GET_COUNTER(htim);
            // If the time elapsed is greater than the timeout, then stop and return true.
            // Only check this if the timer is counting so that the processor is not needlessly checking the inequality
            // in a loop.
            // The 16-bit arithmetic should handle overwrapping.
            if ((is_counting) && (static_cast<uint16_t>(count - start) > timeout)) {
                stop();
                return true;
            }
            else {
                return false;
            }
        }

    private:
        /// @brief  The handler of the peripheral timer.
        TIM_HandleTypeDef* htim;
        /// @brief  The time at which the count began.
        uint16_t start;
        /// @brief  The timeout to count towards.
        uint16_t timeout;
        /// @brief  Whether the timer is in use.
        bool is_counting;
    };

}  // namespace utility::support

#endif  // UTILITY_SUPPORT_MICROSECONDTIMER_HPP
