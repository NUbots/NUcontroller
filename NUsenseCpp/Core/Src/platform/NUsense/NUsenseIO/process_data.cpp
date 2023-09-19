#include "../NUsenseIO.hpp"

#include "../Convert.hpp"
#include "../../../utility/math/comparison.hpp"

namespace platform::NUsense {

    void NUsenseIO::process_servo_data(
        std::array<platform::ServoState,NUMBER_OF_DEVICES>& servo_states,
        const dynamixel::StatusReturnCommand<sizeof(DynamixelServoReadData)> packet
    ) {
        const DynamixelServoReadData data = *(reinterpret_cast<const DynamixelServoReadData*>(packet.data.data()));

        // IDs are 1..20 so need to be converted for the servo_states index
        uint8_t servo_index = packet.id - 1;

        servo_states[servo_index].torque_enabled = (data.torque_enable == 1);

        // Although they're stored in the servo state here, packet errors are combined and processed all at once as
        // subcontroller errors in the RawSensors message
        servo_states[servo_index].packet_error = (uint8_t)packet.error;

        // Servo error status from control table, NOT dynamixel status packet error.
        servo_states[servo_index].hardware_error = data.hardware_error_status;

        servo_states[servo_index].present_pwm      = convert::PWM(data.present_pwm);
        servo_states[servo_index].present_current  = convert::current(data.present_current);
        servo_states[servo_index].present_velocity = convert::velocity(data.present_velocity);  // todo: check
        // TODO: Add the proper direction and offset somehow.
        servo_states[servo_index].present_position =
            convert::position(servo_index, data.present_position, {1}, {0});
        servo_states[servo_index].voltage     = convert::voltage(data.present_voltage);
        servo_states[servo_index].temperature = convert::temperature(data.present_temperature);

        // Buzz if any servo is hot, use the boolean flag to turn the buzzer off once the servo is no longer hot
        // A servo is defined to be hot if the detected temperature exceeds the maximum tolerance in the configuration
        /*bool any_servo_hot = false;
        for (const auto& servo : servo_states) {
            if (servo.temperature > cfg.alarms.temperature.level) {
                any_servo_hot = true;
                HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
                break;
            }
        }

        if (!any_servo_hot) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }*/

        // If this servo has not been initialised yet, set the goal states to the current states
        if (!servo_states[servo_index].initialised) {
            servo_states[servo_index].goal_position = servo_states[servo_index].present_position;
            servo_states[servo_index].torque        = servo_states[servo_index].torque_enabled ? 1.0f : 0.0f;
            servo_states[servo_index].initialised   = true;
        }
    }

}  // namespace platform::NUsense
