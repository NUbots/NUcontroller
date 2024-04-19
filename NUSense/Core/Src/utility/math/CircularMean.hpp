#ifndef UTILITY_MATH_CIRCULARMEAN_HPP
#define UTILITY_MATH_CIRCULARMEAN_HPP

#include <cmath>

namespace utility::math {

    /**
     * @brief   a handler that calculates the circular mean of a frame of samples.
     */
    class CircularMean {
    public:
        /**
         * @brief    Constructs the handler.
         */
        CircularMean() : sine_sum(0.0f), cosine_sum(0.0f){};
        /**
         * @brief   Destructs the handler.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~CircularMean(){};

        /**
         * @brief   Adds the angle to the sums.
         * @param   angle the angle to be added in radians,
         */
        void add(const float angle) {
            sine_sum += std::sin(angle);
            cosine_sum += std::cos(angle);
        }

        /**
         * @brief Calculates the mean.
         * @return the mean
         */
        const float get_mean() const {
            return std::atan2(sine_sum, cosine_sum);
        }

        /**
         * @brief   Resets the sums to nought.
         */
        void reset() {
            sine_sum   = 0.0f;
            cosine_sum = 0.0f;
        }

    private:
        /// @brief  the sum of sines,
        float sine_sum;
        /// @brief  the sum of cosines,
        float cosine_sum;
    };

}  // namespace utility::math

#endif  // UTILITY_MATH_CIRCULARMEAN_HPP
