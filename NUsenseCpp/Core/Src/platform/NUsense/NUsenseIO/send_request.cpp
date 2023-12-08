//#include <fmt/format.h>

#include "../Convert.hpp"
#include "../NUsenseIO.hpp"

namespace platform::NUsense {

    void NUsenseIO::send_servo_read_request(const NUgus::ID id, const uint8_t port_i) {
        packet_handlers[port_i].reset();
        ports[port_i].write(
            dynamixel::ReadCommand(
                (uint8_t)id,
                (uint16_t)AddressBook::SERVO_READ,
                (uint16_t)sizeof(DynamixelServoReadData)
            )
        );
    }

    void NUsenseIO::send_servo_write_1_request(const NUgus::ID id, const uint8_t port_i) {

        DynamixelServoWriteDataPart1 data;

        uint8_t i = (uint8_t)id;

        // If our torque should be disabled then we disable our torque
        data.torque_enable =
            uint8_t(    servo_states[i].torque != 0 
                    &&  !std::isnan(servo_states[i].goal_position)
            );

        data.velocity_i_gain = convert::i_gain(servo_states[i].velocity_i_gain);
        data.velocity_p_gain = convert::p_gain(servo_states[i].velocity_p_gain);
        data.position_d_gain = convert::d_gain(servo_states[i].position_d_gain);
        data.position_i_gain = convert::i_gain(servo_states[i].position_i_gain);
        data.position_p_gain = convert::p_gain(servo_states[i].position_p_gain);

        // Send a write-instruction for the current servo.
        packet_handlers[port_i].reset();
        ports[port_i].write(
            dynamixel::WriteCommand<DynamixelServoWriteDataPart1>(
                (uint8_t)id,
                (uint16_t)AddressBook::SERVO_WRITE_1,
                data
            )
        );
        
    }

    void NUsenseIO::send_servo_write_2_request(const NUgus::ID id, const uint8_t port_i) {

        DynamixelServoWriteDataPart2 data;

        uint8_t i = (uint8_t)id;

        data.feedforward_1st_gain = convert::ff_gain(servo_states[i].feedforward_1st_gain);
        data.feedforward_2nd_gain = convert::ff_gain(servo_states[i].feedforward_2nd_gain);
        data.goal_pwm             = convert::PWM(servo_states[i].goal_pwm);
        data.goal_current         = convert::current(servo_states[i].goal_current);
        data.goal_velocity        = convert::velocity(servo_states[i].goal_velocity);
        data.profile_acceleration = convert::ff_gain(servo_states[i].profile_acceleration);
        data.profile_velocity     = convert::profile_velocity(servo_states[i].profile_velocity);
        data.goal_position =
            convert::position(i, servo_states[i].goal_position, {1}, {0});

        // Send a write-instruction for the current servo.
        packet_handlers[port_i].reset();
        ports[port_i].write(
            dynamixel::WriteCommand<DynamixelServoWriteDataPart2>(
                (uint8_t)id,
                (uint16_t)AddressBook::SERVO_WRITE_2,
                data
            )
        );
            
    }

}  // namespace platform::NUsense
