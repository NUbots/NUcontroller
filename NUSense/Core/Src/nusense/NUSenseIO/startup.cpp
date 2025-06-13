#include "../NUSenseIO.hpp"

namespace nusense {

    void NUSenseIO::startup() {

        /*
            ~~~ ~~~ ~~~ Discovery ~~~ ~~~ ~~~
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Here, the servos are polled on each chain so that the firmware
            knows on which chain a particular ID is.
        */

        chain_manager.discover();

        /*
            ~~~ ~~~ ~~~ Basic Set-up ~~~ ~~~ ~~~
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Here, the servos are set up to have no return-delay, to return always
            to a write-instruction (unlike the OpenCR set-up), and to have time-
            based profile-velocity control.
        */

        // For each port, write for all servos the return-delay-time to be 0 μs.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write instruction again if there is something wrong with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<uint8_t>(
                        uint8_t(id),
                        uint16_t(dynamixel::DynamixelServo::Address::RETURN_DELAY_TIME),
                        0x00));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // For each port, write for all servos the status-return-level to allow
        // all statuses to be returned.
        // Arguably, this is not needed since it is 0x02 by default.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong
                // with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<uint8_t>(
                        uint8_t(id),
                        uint16_t(dynamixel::DynamixelServo::Address::STATUS_RETURN_LEVEL),
                        0x02));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // For each port, write for all servos the drive-mode to have a
        // velocity-based profile.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong
                // with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(
                        dynamixel::WriteCommand<uint8_t>(uint8_t(id),
                                                         uint16_t(dynamixel::DynamixelServo::Address::DRIVE_MODE),
                                                         0x04));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // For each port, write for all servos the profile-velocity to be 1s.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<uint32_t>(
                        uint8_t(id),
                        uint16_t(dynamixel::DynamixelServo::Address::PROFILE_VELOCITY_L),
                        1000));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
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
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong
                // with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<std::array<uint16_t, 17>>(
                        uint8_t(id),
                        uint16_t(nusense::AddressBook::SERVO_READ_ADDRESS),
                        {uint16_t(dynamixel::DynamixelServo::Address::TORQUE_ENABLE),
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
                         uint16_t(dynamixel::DynamixelServo::Address::PRESENT_TEMPERATURE)}));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // For each port, write for all servos the addresses of the first
        // write-bank to the indirect registers.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong
                // with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<std::array<uint16_t, 11>>(
                        uint8_t(id),
                        uint16_t(nusense::AddressBook::SERVO_WRITE_ADDRESS_1),
                        {uint16_t(dynamixel::DynamixelServo::Address::TORQUE_ENABLE),
                         uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_I_GAIN_L),
                         uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_I_GAIN_H),
                         uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_P_GAIN_L),
                         uint16_t(dynamixel::DynamixelServo::Address::VELOCITY_P_GAIN_H),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_D_GAIN_L),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_D_GAIN_H),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_I_GAIN_L),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_I_GAIN_H),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_P_GAIN_L),
                         uint16_t(dynamixel::DynamixelServo::Address::POSITION_P_GAIN_H)}));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // For each port, write for all servos the addresses of the second
        // write-bank to the indirect registers.
        for (auto& chain : chain_manager.get_chains()) {
            // Get the packet-handler from the chain.
            dynamixel::PacketHandler& packet_handler = chain.get_packet_handler();
            for (const auto& id : chain.get_servos()) {
                // Send the write-instruction again if there is something wrong with the returned status.
                do {
                    // Send the instruction with reset and timeout.
                    chain.write(dynamixel::WriteCommand<std::array<uint16_t, 24>>(
                        uint8_t(id),
                        uint16_t(nusense::AddressBook::SERVO_WRITE_ADDRESS_2),
                        {uint16_t(dynamixel::DynamixelServo::Address::FEEDFORWARD_1ST_GAIN_L),
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
                         uint16_t(dynamixel::DynamixelServo::Address::GOAL_POSITION_H)}));

                    // Wait for the status to be received and decoded.
                    do {
                        packet_handler.check_sts<0>(id);
                    } while (packet_handler.get_result() == dynamixel::PacketHandler::Result::NONE
                             || packet_handler.get_result() == dynamixel::PacketHandler::Result::PARTIAL);
                } while (packet_handler.get_result() != dynamixel::PacketHandler::Result::SUCCESS);
            }
        }

        // Begin the 100-Hz timer.
        loop_timer.begin(10);

        // Begin an initial pulse as a heartbeat.
        right_rgb.set_value(0xFFFF00);
        right_rgb.pulse(1, true, device::back_panel::Led::Priority::LOW);

        // Set the state of each expect status as a response to a write-instruction.
        status_states.fill(StatusState::WRITE_1_RESPONSE);

        // Send the first write-instruction to begin the chain-reaction on each port.
        for (auto& chain : chain_manager.get_chains()) {
            if (!chain.empty()) {
                send_servo_write_1_request(chain);
            }
        }
    }
}  // namespace nusense
