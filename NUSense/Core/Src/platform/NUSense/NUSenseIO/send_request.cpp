
#include "../Convert.hpp"
#include "../NUSenseIO.hpp"

namespace platform::NUSense {

    void NUSenseIO::send_servo_read_request(dynamixel::Chain& chain) {
        NUgus::ID id = chain.current();
        chain.write(dynamixel::ReadCommand(static_cast<uint8_t>(id),
                                           static_cast<uint16_t>(AddressBook::SERVO_READ),
                                           static_cast<uint16_t>(sizeof(DynamixelServoReadData))));
    }

    void NUSenseIO::send_servo_write_1_request(dynamixel::Chain& chain) {

        DynamixelServoWriteDataPart1 data{};

        NUgus::ID id = chain.current();
        uint8_t i    = static_cast<uint8_t>(id) - 1;

        // If our torque should be disabled then we disable our torque
        data.torque_enable = uint8_t(servo_states[i].torque != 0 && !std::isnan(servo_states[i].goal_position));

        data.velocity_i_gain = convert::i_gain(servo_states[i].velocity_i_gain);
        data.velocity_p_gain = convert::p_gain(servo_states[i].velocity_p_gain);
        data.position_d_gain = convert::d_gain(servo_states[i].position_d_gain);
        data.position_i_gain = convert::i_gain(servo_states[i].position_i_gain);
        data.position_p_gain = convert::p_gain(servo_states[i].position_p_gain);

        // Send a write-instruction for the current servo.
        // Chain.write readys the packet handler for the response packet and starts the timeout timer.
        chain.write(
            dynamixel::WriteCommand<DynamixelServoWriteDataPart1>(static_cast<uint8_t>(id),
                                                                  static_cast<uint16_t>(AddressBook::SERVO_WRITE_1),
                                                                  data));
    }

    void NUSenseIO::send_servo_write_2_request(dynamixel::Chain& chain) {

        DynamixelServoWriteDataPart2 data{};

        NUgus::ID id = chain.current();
        uint8_t i    = static_cast<uint8_t>(id) - 1;

        data.feedforward_1st_gain = convert::ff_gain(servo_states[i].feedforward_1st_gain);
        data.feedforward_2nd_gain = convert::ff_gain(servo_states[i].feedforward_2nd_gain);
        data.goal_pwm             = convert::PWM(servo_states[i].goal_pwm);
        data.goal_current         = convert::current(servo_states[i].goal_current);
        data.goal_velocity        = convert::velocity(servo_states[i].goal_velocity);
        data.profile_acceleration = convert::ff_gain(servo_states[i].profile_acceleration);
        data.profile_velocity     = convert::profile_velocity(servo_states[i].profile_velocity);
        data.goal_position        = convert::position(i, servo_states[i].goal_position, {1}, {0});

        // Send a write-instruction for the current servo.
        // Chain.write readys the packet handler for the response packet and starts the timeout timer.
        chain.write(
            dynamixel::WriteCommand<DynamixelServoWriteDataPart2>(static_cast<uint8_t>(id),
                                                                  static_cast<uint16_t>(AddressBook::SERVO_WRITE_2),
                                                                  data));
    }
}  // namespace platform::NUSense
