#ifndef PLATFORM_SERVOSTATE_HPP
#define PLATFORM_SERVOSTATE_HPP

#include "stdint.h" // needed for explicit type-defines

#include <ostream>  // needed for outputting the servo-state

namespace platform {
/// @see servo_states
struct ServoState {
    /// @brief True if we need to write new values to the hardware
    bool dirty = false;

    /// @brief Current error state of the servo
    /// @note different to the dynamixel packet error status
    uint8_t hardware_error = 0;

    /// @brief Most recent packet error received, to cache before RawSensors is emitted
    uint8_t packet_error = 0;

    /// @brief True if we simulate where we think the servos should be
    /// @note that we still write the commands to hardware
    bool simulated = false;

    /// @brief Our internal system torque target, this is never sent to the servo
    int torque = 0;
    /// @brief Whether the servo's torque is enabled, allowing it to move
    bool torque_enabled = true;

    // Cached values that are never read and are sent to the servos
    /// @brief The integral gain of the velocity
    float velocity_i_gain = 1920.0f / 65536.0f;  // ROBOTIS default
    /// @brief The proportional gain of the velocity
    float velocity_p_gain = 100.0f / 128.0f;  // ROBOTIS default
    /// @brief The derivative gain of the position
    float position_d_gain = 0.0f;
    /// @brief The integral gain of the position
    float position_i_gain = 0.0f;
    /// @brief The proportional gain of the position
    float position_p_gain = 850.0f / 128.0f;  // ROBOTIS default
    /// @brief The first feedforward torque coefficient
    float feedforward_1st_gain = 0.0f;
    /// @brief The second feedforward torque coefficient
    float feedforward_2nd_gain = 0.0f;
    /// @brief The target pulse width modulation of the servo
    float goal_pwm = 885.0f;  // ROBOTIS default
    /// @brief The target current of the servo
    float goal_current = 6.52176f / 0.00336f;  // ROBOTIS default
    /// @brief The target velocity of the servo, not used by the servos
    float goal_velocity = 1.08775f;  // ROBOTIS default
    /// @brief The target position of the servo
    float goal_position = 0.0f;
    /// @brief The target acceleration of the servo
    float profile_acceleration = 0.0f;
    /// @brief The target velocity of the servo, replacing moving speed in v1 protocol
    float profile_velocity = 0.0f;

    // Values that are either simulated or read from the servos
    /// @brief The last read pulse width modulation of the servo
    float present_pwm = 0.0f;
    /// @brief The last read current of the servo
    float present_current = 0.0f;
    /// @brief The last read velocity of the servo
    float present_velocity = 0.0f;
    /// @brief The last read position of the servo
    float present_position = 0.0f;
    /// @brief The last read voltage of the servo
    float voltage = 0.0f;
    /// @brief The last read temperature of the servo
    float temperature = 0.0f;

    /// @brief Whether we have initialised this servo yet
    bool initialised = false;
};

/**
 * @brief   Outputs the read values to a row.
 * @note    this is mainly used for debugging,
 * @param   out the output stream,
 * @param   servo_state the servo-state,
 * @return  the output stream,
 */
std::ostream & operator << (std::ostream& out, const ServoState& servo_state);

} // namespace paltform

#endif  // PLATFORM_SERVOSTATE_HPP
