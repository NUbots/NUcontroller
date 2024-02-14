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
        	SET_SIGNAL_1();
            platform::NUsense::NUgus::ID current_id = (chains[i])[chain_indices[i]];
        	RESET_SIGNAL_1();

            SET_SIGNAL_1();

            if (packet_handlers[i].check_sts
                <sizeof(platform::NUsense::DynamixelServoReadData)>
                (current_id)
                == dynamixel::PacketHandler::SUCCESS
            ) {
            	RESET_SIGNAL_1();

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
                            servo_states[(uint8_t)current_id-1].dirty = false;
                            send_servo_write_1_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = WRITE_1_RESPONSE;

                        } else {
                            send_servo_read_request(current_id, i);
                            status_states[(uint8_t)current_id-1] = READ_RESPONSE;
                        }

                        break;
                }
            }

            RESET_SIGNAL_1();
        }

        // Handle the incoming protobuf messages from the nuc.
        SET_SIGNAL_3();
        if (nuc.handle_incoming()) {
            // For every new target, update the state if it is a servo.
            message_actuation_ServoTargets* new_targets = nuc.get_targets();
            for (int i = 0; i < new_targets->targets_count; i++) {
                auto new_target = new_targets->targets[i];
                if (new_target.id < NUMBER_OF_DEVICES) {
                    servo_states[new_target.id].position_p_gain  = new_target.gain;
                    servo_states[new_target.id].goal_position    = new_target.position;
                    servo_states[new_target.id].torque_enabled   = (new_target.torque != 0);
                    // Set the dirty-flag so that the Dynamixel stream writes to the servo.
                    servo_states[new_target.id].dirty            = true;
                }
            }
        }
        RESET_SIGNAL_3();
    }
} // namespace platform::NUsense
