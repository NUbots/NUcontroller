#include "../NUsenseIO.hpp"

namespace platform::NUsense {

    void NUsenseIO::startup() {
        // Begin the receiving. This should be done only once if we are using the DMA
        // as a buffer.
        for (auto& port : ports) {
            port.begin_rx();
            port.flush_rx();
        }

        /*
            ~~~ ~~~ ~~~ Basic Set-up ~~~ ~~~ ~~~
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Here, the servos are set up to have no return-delay, to return always
            to a write-instruction (unlike the OpenCR set-up), and to have time-
            based profile-velocity control.
        */

        // For each port, write for all servos the return-delay-time to be 0 μs.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<uint8_t>(
                            (uint8_t)id,
                            (uint16_t)dynamixel::DynamixelServo::Address::RETURN_DELAY_TIME,
                            0x00
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        // For each port, write for all servos the status-return-level to allow 
        // all statuses to be returned.
        // Arguably, this is not needed since it is 0x02 by default.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<uint8_t>(
                            (uint8_t)id,
                            (uint16_t)dynamixel::DynamixelServo::Address::STATUS_RETURN_LEVEL,
                            0x02
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        // For each port, write for all servos the drive-mode to have a 
        // velocity-based profile.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<uint8_t>(
                            (uint8_t)id,
                            (uint16_t)dynamixel::DynamixelServo::Address::DRIVE_MODE,
                            0x04
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        /*
        * ~~~ ~~~ ~~~ Set-up of the Indirect Registers ~~~ ~~~ ~~~
        * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        * Here, the indirect registers are set up by a sequence of 
        * write-instructions on each port. These indirect registers are used for 
        * the contiguous read-bank which is read constantly in a loop and the two 
        * write-banks.
        */

        // For each port, write for all servos the addresses of the read-bank to 
        // the indirect registers.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<std::array<uint16_t,17>>(
                            (uint8_t)id,
                            (uint16_t)platform::NUsense::AddressBook::SERVO_READ_ADDRESS,
                            {
                                uint16_t(dynamixel::DynamixelServo::Address::TORQUE_ENABLE),
                                uint16_t(dynamixel::DynamixelServo::Address::HARDWARE_ERROR_STATUS),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_PWM_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_PWM_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_CURRENT_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_CURRENT_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_VELOCITY_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_VELOCITY_2),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_VELOCITY_3),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_VELOCITY_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_POSITION_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_POSITION_2),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_POSITION_3),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_POSITION_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_INPUT_VOLTAGE_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_INPUT_VOLTAGE_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PRESENT_TEMPERATURE)
                            }
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        // For each port, write for all servos the addresses of the first 
        // write-bank to the indirect registers.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<std::array<uint16_t,11>>(
                            (uint8_t)id,
                            (uint16_t)platform::NUsense::AddressBook::SERVO_WRITE_ADDRESS_1,
                            {
                                uint16_t(dynamixel::DynamixelServo::Address::TORQUE_ENABLE),
                                uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_I_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_I_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_P_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_P_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_D_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_D_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_I_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_I_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_P_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::POSITION_P_GAIN_H)
                            }
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        // For each port, write for all servos the addresses of the second 
        // write-bank to the indirect registers.
        for (int i = 0; i < NUM_PORTS; i++) {

            // Re-use the same packet-hanlder for each port.
            dynamixel::PacketHandler packet_handler(ports[i]);

            for (const auto& id : chains[i]) {
                // Send the write-instruction again if there is something wrong 
                // with the returned status.
                do {
                    // Reset the packet-handler before a new interaction has begun.
                    packet_handler.reset();

                    // Send the write-instruction.
                    ports[i].write(
                        dynamixel::WriteCommand<std::array<uint16_t,24>>(
                            (uint8_t)id,
                            (uint16_t)platform::NUsense::AddressBook::SERVO_WRITE_ADDRESS_2,
                            {
                                uint16_t(dynamixel::DynamixelServo::Address::FEEDFORWARD_1ST_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::FEEDFORWARD_1ST_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::FEEDFORWARD_2ND_GAIN_L),
                                uint16_t(dynamixel::DynamixelServo::Address::FEEDFORWARD_2ND_GAIN_H),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_PWM_L),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_PWM_H),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_CURRENT_L),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_CURRENT_H),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_VELOCITY_L),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_VELOCITY_2),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_VELOCITY_3),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_VELOCITY_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_ACCELERATION_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_ACCELERATION_2),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_ACCELERATION_3),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_ACCELERATION_H),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_VELOCITY_L),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_VELOCITY_2),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_VELOCITY_3),
                                uint16_t(dynamixel::DynamixelServo::Address::PROFILE_VELOCITY_H),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_POSITION_L),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_POSITION_2),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_POSITION_3),
                                uint16_t(dynamixel::DynamixelServo::Address::GOAL_POSITION_H)
                            }
                        )
                    );

                    // Wait for the status to be received and decoded.
                    while (
                        packet_handler.check_sts<0>(id) 
                        == dynamixel::PacketHandler::Result::NONE
                    );
                } while (
                    packet_handler.get_result() 
                    != dynamixel::PacketHandler::Result::SUCCESS
                );
            }
        }

        // Send the first write-instruction to begin the chain-reaction on each
        // port.
        std::vector<platform::NUsense::NUgus::ID> chain = chains[0];
        for (int i = 0; i < NUM_PORTS; i++) {
            ports[i].write(
                dynamixel::ReadCommand(
                    (uint8_t)(chains[i])[chain_indices[i]],
                    (uint16_t)platform::NUsense::AddressBook::SERVO_READ,
                    (uint16_t)sizeof(platform::NUsense::DynamixelServoReadData)
                )
            );
        }

    }
} // namespace platform::NUsense
