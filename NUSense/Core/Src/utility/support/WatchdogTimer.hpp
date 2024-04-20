#include "signal.h"
#include "tim.h"

#ifndef UTILITY_SUPPORT_WATCHDOGTIMER_HPP
    #define UTILITY_SUPPORT_WATCHDOGTIMER_HPP


namespace utility::support {

    /**
     * @brief   A microsecond timer set up as a watchdog timer.
     * @note    This should really be re-written as a singleton class
     */
    class WatchdogTimer {
    public:
        /**
         * @brief    Constructs the timer.
         * @param    htim the reference to the timer to be counted,
         * @param    timeout the timeout in microseconds, at most 65535,
         */
        WatchdogTimer(TIM_HandleTypeDef* htim = &htim3, uin16_t timeout)
            : htim(htim), timeout(timeout), watchdog_active(false), watchdog_timeout(false) {
            // start the timer with the timeout
            HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_3);
        };

        /**
         * @brief Destructs the timer.
         */
        ~WatchdogTimer() {
            // stop the timer
            HAL_TIM_PWM_Stop(&htim, TIM_CHANNEL_3);
        }

    private:
        /// @brief  the handler of the peripheral timer,
        TIM_HandleTypeDef* htim;

        /// @brief  the watchdog timeout in microseconds,
        uint16_t timeout;

        /// @brief  whether the watchdog timer is in use,
        bool watchdog_active;

        /// @brief whether the watchdog timer has timed out,
        bool watchdog_timeout;
    };

}  // namespace utility::support
#endif  // UTILITY_SUPPORT_WATCHDOGTIMER_HPP