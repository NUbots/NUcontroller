#include "../NUsenseIO.hpp"

#include <sstream>
#include "usbd_cdc_if.h"

#include "signal.h"

namespace platform::NUsense {
    
    void NUsenseIO::loop() {
        // For each port, check whether the expected status has been
        // successfully received. If so, then handle it and send the next read-
        // instruction.
        for (int i = 0; i < NUM_PORTS; i++) {
            // This line may be slow. The whole data-structure of chains may need to optimised as 
            // something faster than vectors.
            platform::NUsense::NUgus::ID current_id = (chains[i])[chain_indices[i]];

            dynamixel::PacketHandler::Result result = packet_handlers[i].check_sts
                            <sizeof(platform::NUsense::DynamixelServoReadData)>
                            (current_id);
            // If there is a status-response waiting, then handle it.
            if (result == dynamixel::PacketHandler::SUCCESS) {

                switch (status_states[(uint8_t)current_id-1]) {
                    // After a response for the first bank of registers, send a write-instruction 
                    // for the second bank of registers.
                    case StatusState::WRITE_1_RESPONSE:

                        send_servo_write_2_request(current_id, i);
                        status_states[(uint8_t)current_id-1] = WRITE_2_RESPONSE;

                        break;

                    // After a response for the second bank of registers, send a read-instruction 
                    // for the read bank of registers.
                    case StatusState::WRITE_2_RESPONSE:

                        // Reset the flag now that the two write-instructions were properly 
                        // received.
                        servo_states[(uint8_t)current_id-1].dirty = false;

                        send_servo_read_request(current_id, i);
                        status_states[(uint8_t)current_id-1] = READ_RESPONSE;

                        break;

                    default:
                    // Parse and convert the read data to the local cache and then send the first 
                    // write instruction if the servo is dirty.
                    case StatusState::READ_RESPONSE:
                        process_servo_data(
                            *reinterpret_cast<const dynamixel::StatusReturnCommand<sizeof(
                                platform::NUsense::DynamixelServoReadData
                            )>*>(
                                packet_handlers[i].get_sts_packet()
                            )
                        );

                        // Move along the chain.
                        chain_indices[i] = (chain_indices[i] + 1) % chains[i].size();
                        current_id = (chains[i])[chain_indices[i]];

                        // If the servo-state is dirty, then send a write-instruction.
                        if (servo_states[(uint8_t)current_id-1].dirty) {
                            send_servo_write_1_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = WRITE_1_RESPONSE;
                        } else {
                            // Else, send a read-instruction.
                            send_servo_read_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = READ_RESPONSE;
                        }

                        break;
                }
            }
            // If there was an error, then just restart the stream.
            else if (   (result == dynamixel::PacketHandler::ERROR)
                    ||  (result == dynamixel::PacketHandler::CRC_ERROR)
                    ||  (result == dynamixel::PacketHandler::TIMEOUT)
            ) {

                switch (status_states[(uint8_t)current_id-1]) {

                    // If there was an error with the read-response, then go to the next servo 
                    // along the chain.
                    default:
                    case StatusState::READ_RESPONSE:

                        // Move along the chain.
                        chain_indices[i] = (chain_indices[i] + 1) % chains[i].size();
                        current_id = (chains[i])[chain_indices[i]];

                        // If the servo-state is dirty, then send a write-instruction.
                        if (servo_states[(uint8_t)current_id-1].dirty) {
                            send_servo_write_1_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = WRITE_1_RESPONSE;
                        } else {
                            send_servo_read_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = READ_RESPONSE;
                        }

                        break;
                }

            }
        }

        // Handle the incoming protobuf messages from the nuc.
        if (nuc.handle_incoming()) {
            // For every new target, update the state if it is a servo.
            message_actuation_ServoTargets* new_targets = nuc.get_targets();
            for (int i = 0; i < new_targets->targets_count; i++) {
                auto new_target = new_targets->targets[i];
                if ((new_target.id) < NUMBER_OF_DEVICES) {
                    servo_states[new_target.id].position_p_gain    = new_target.gain;
                    servo_states[new_target.id].goal_position      = new_target.position;
                    servo_states[new_target.id].torque             = new_target.torque;
                    // Set the dirty-flag so that the Dynamixel stream writes to the servo.
                    servo_states[new_target.id].dirty            = true;
                }
            }
        }

        // Here send data to the NUC at 100 Hz.
        if (loop_timer.has_timed_out()) {
            // If it has timed out, then restart the timer straight away.
            loop_timer.begin(10);

            // Add the encoding and USB stuff here.

            if (mode_button.filter()) {
                SET_SIGNAL_1();
                RESET_SIGNAL_1();
            }
        }
    }
} // namespace platform::NUsense
