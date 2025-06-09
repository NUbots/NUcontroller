#include <algorithm>
#include <chrono>
#include <sstream>

#include "../NUSenseIO.hpp"
#include "signal.h"
#include "usbd_cdc_if.h"

namespace nusense {

    void NUSenseIO::loop() {
        // For each port, check whether the expected status has been
        // successfully received. If so, then handle it and send the next read-
        // instruction.
        for (auto& chain : chain_manager.get_chains()) {
            // Index of the current servo in the chain, 0 indexed.
            uint8_t current_servo_index = static_cast<uint8_t>(chain.current()) - 1;

            dynamixel::PacketHandler::Result result =
                chain.get_packet_handler().check_sts<sizeof(nusense::DynamixelServoReadData)>(chain.current());
            // If there is a status-response waiting, then handle it.
            if (result == dynamixel::PacketHandler::SUCCESS) {

                // Log a success.
                servo_states[current_servo_index].num_successes++;

                switch (status_states[current_servo_index]) {
                    // After a response for the first bank of registers, send a write-instruction
                    // for the second bank of registers.
                    case StatusState::WRITE_1_RESPONSE:

                        // If the torque has just been enabled by the last write-instruction, then
                        // cool down for 1 ms until the servo decides to behave itself.
                        if ((servo_states[current_servo_index].torque_enabled == false)
                            && (servo_states[current_servo_index].torque != 0.0)) {
                            chain.get_packet_handler().ready();
                            chain.get_timer().begin(1);
                            status_states[current_servo_index] = WRITE_1_COOLDOWN;
                        }
                        // Otherwise, send the next write-instruction as normal.
                        else {
                            send_servo_write_2_request(chain);
                            status_states[current_servo_index] = WRITE_2_RESPONSE;
                        }

                        break;

                    // After a response for the second bank of registers, send a read-instruction
                    // for the read bank of registers.
                    case StatusState::WRITE_2_RESPONSE:

                        send_servo_read_request(chain);
                        status_states[current_servo_index] = READ_RESPONSE;

                        break;

                    default:
                    // Parse and convert the read data to the local cache and then send the first
                    // write instruction if the servo is dirty.
                    case StatusState::READ_RESPONSE:
                        process_servo_data(
                            *reinterpret_cast<
                                const dynamixel::StatusReturnCommand<sizeof(nusense::DynamixelServoReadData)>*>(
                                chain.get_packet_handler().get_sts_packet()));

                        // Move along the chain.
                        chain.next();
                        // update servo index variable
                        current_servo_index = static_cast<uint8_t>(chain.current()) - 1;

                        // If the servo-state is dirty, then send a write-instruction.
                        if (servo_states[current_servo_index].dirty) {

                            // Reset the flag now that the two write-instructions have begun.
                            servo_states[current_servo_index].dirty = false;

                            send_servo_write_1_request(chain);
                            status_states[current_servo_index] = WRITE_1_RESPONSE;
                        }
                        else {
                            // Else, send a read-instruction.
                            send_servo_read_request(chain);
                            status_states[current_servo_index] = READ_RESPONSE;
                        }

                        break;
                }
            }
            // If there was an error, then just restart the stream.
            else if ((result == dynamixel::PacketHandler::ERROR) || (result == dynamixel::PacketHandler::CRC_ERROR)
                     || (result == dynamixel::PacketHandler::TIMEOUT)) {

                // Log the kind of fault.
                switch (result) {
                    case dynamixel::PacketHandler::TIMEOUT: servo_states[current_servo_index].num_timeouts++; break;
                    case dynamixel::PacketHandler::CRC_ERROR: servo_states[current_servo_index].num_crc_errors++; break;
                    default:
                    case dynamixel::PacketHandler::ERROR: servo_states[current_servo_index].num_packet_errors++; break;
                }

                // Move along the chain.
                chain.next();
                // update servo index variable
                current_servo_index = static_cast<uint8_t>(chain.current()) - 1;

                // If the servo-state is dirty, then send a write-instruction.
                if (servo_states[current_servo_index].dirty) {

                    // Reset the flag now that the two write-instructions have begun.
                    servo_states[current_servo_index].dirty = false;

                    send_servo_write_1_request(chain);
                    status_states[current_servo_index] = WRITE_1_RESPONSE;
                }
                else {
                    send_servo_read_request(chain);
                    status_states[current_servo_index] = READ_RESPONSE;
                }

                break;
            }

            // If we are cooling down, then see whether the timer has timed out. If so, then send
            // the next write-instruction.
            if ((status_states[current_servo_index] == WRITE_1_COOLDOWN) && (chain.get_timer().has_timed_out())) {
                send_servo_write_2_request(chain);
                status_states[current_servo_index] = WRITE_2_RESPONSE;
            }
        }

        // Handle the incoming protobuf messages from the nuc.
        if (nuc.handle_incoming()) {
            // If we get a message with servo targets, start decoding
            if (nuc.get_curr_msg_hash() == utility::message::SUBCONTROLLER_SERVO_TARGETS_HASH) {
                // TODO (JohanneMontano) use below somehow somewhere?
                //  uint64_t msg_ts = nuc.get_curr_msg_timestamp();

                // For every new target, update the state if it is a servo.
                message_actuation_SubcontrollerServoTargets* new_targets = nuc.get_targets();
                for (int i = 0; i < new_targets->targets_count; i++) {
                    message_actuation_SubcontrollerServoTarget* new_target = &(new_targets->targets[i]);
                    if ((new_target->id) < NUMBER_OF_DEVICES) {
                        servo_states[new_target->id].profile_velocity =
                            std::max(0.0,
                                     (float(new_target->time.seconds) * 1000) + (float(new_target->time.nanos) / 1e6));
                        servo_states[new_target->id].position_p_gain = new_target->gain;
                        servo_states[new_target->id].goal_position   = new_target->position;
                        servo_states[new_target->id].torque          = new_target->torque;
                        // Set the dirty-flag so that the Dynamixel stream writes to the servo.
                        servo_states[new_target->id].dirty = true;
                    }
                }
            }
            // If we get a handshake message from the NUC while NUSense is looping, then we have to send the NUC an ACK
            else if (nuc.get_curr_msg_hash() == utility::message::HANDSHAKE_HASH) {
                
                // Send reply to NUSense
                strcpy(handshake_msg.msg, "NUSense ack rec req");

                // Once everything else is filled we send it to the NUC. Just overwrite the bytes within
                // encoding_payload Allow max size for the output buffer so it doesn't throw an error if there's not
                // enough space If one wishes to add messages to the protobuf message, one must first calculate the
                // maximum bytes within that message and then add enough bytes to make sure that nanopb doesn't cry
                // about the output stream being too small If the MAX_ENCODE_SIZE is inadequately defined, one can get a
                // corrupted message and nanopb errors.
                pb_ostream_t output_buffer = pb_ostream_from_buffer(&encoding_payload[0], MAX_ENCODE_SIZE);

                // TODO (NUSense people) Handle encoding errors properly using this member somehow
                nanopb_encoding_err =
                    pb_encode(&output_buffer, message_platform_NUSenseHandshake_fields, &handshake_msg) ? false : true;

                // Happiness, the encoding succeeded
                std::vector<uint8_t> nbs({0xE2, 0x98, 0xA2});

                // TODO (JohanneMontano) Implement timestamp field correctly, std::chrono is behaving weird and it needs
                // to be investigated
                uint64_t ts_u = 0;
                uint32_t size =
                    uint32_t(output_buffer.bytes_written + sizeof(utility::message::HANDSHAKE_HASH) + sizeof(ts_u));

                // Encode size to uint8_t's
                for (size_t i = 0; i < sizeof(size); ++i) {
                    nbs.push_back(uint8_t((size >> (i * 8)) & 0xFF));
                }

                // Encode timestamp
                for (size_t i = 0; i < sizeof(ts_u); ++i) {
                    nbs.push_back(uint8_t((ts_u >> (i * 8)) & 0xFF));
                }

                // Encode handshake hash
                for (size_t i = 0; i < sizeof(utility::message::HANDSHAKE_HASH); ++i) {
                    nbs.push_back(uint8_t((utility::message::HANDSHAKE_HASH >> (i * 8)) & 0xFF));
                }

                // Add the protobuf bytes into the nbs vector
                nbs.insert(nbs.end(),
                           std::begin(encoding_payload),
                           std::begin(encoding_payload) + output_buffer.bytes_written);

                // Attempt to transmit data then handle it accordingly if it fails
                if (CDC_Transmit_HS(nbs.data(), nbs.size()) != USBD_OK) {
                    // Going into this block means that the usb failed to transmit our data
                    usb_tx_err = true;
                }

                // Buzz after the handshake has been received
                HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
                HAL_Delay(500);
                HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
            }
        }

        // Here send data to the NUC at 100 Hz.
        if (loop_timer.has_timed_out()) {
            // If it has timed out, then restart the timer straight away.
            loop_timer.begin(10);

            // Encode a message and send it to the NUC.
            if (nusense_to_nuc()) {
                // If the message was successfully sent, then reset the averaging filter.
                // For now, this is how we are downsampling the ~500-Hz data to 100-Hz fixed data.
                // One day, we may get a better filter (if we can get this chip faster).
                for (auto& servo_state : servo_states) {
                    servo_state.filter_count     = 0.0f;
                    servo_state.packet_error     = 0x00;
                    servo_state.hardware_error   = 0x00;
                    servo_state.present_pwm      = 0.0f;
                    servo_state.present_current  = 0.0f;
                    servo_state.present_velocity = 0.0f;
                    servo_state.voltage          = 0.0f;
                    servo_state.temperature      = 0.0f;
                    servo_state.mean_present_position.reset();
                    servo_state.num_successes     = 0;
                    servo_state.num_timeouts      = 0;
                    servo_state.num_crc_errors    = 0;
                    servo_state.num_packet_errors = 0;
                }
            }

            // Handle any of the pulser objects.
            tx_led.handle();
            rx_led.handle();
            red_led.handle();
            blue_led.handle();
            green_led.handle();
            left_rgb.handle();
            right_rgb.handle();
            buzzer.handle();
        }
    }
}  // namespace nusense
