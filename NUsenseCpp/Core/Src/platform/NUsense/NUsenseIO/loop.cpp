#include "../NUsenseIO.hpp"

#include <sstream>
#include "usbd_cdc_if.h"

<<<<<<< HEAD
#include "signal.h"

=======
>>>>>>> 7c81a4a620df92baa139a6dc1669529b3993de71
namespace platform::NUsense {
    
    void NUsenseIO::loop() {
        // For each port, check whether the expected status has been
        // successfully received. If so, then handle it and send the next read-
        // instruction.
        for (int i = 0; i < NUM_PORTS; i++) {
<<<<<<< HEAD
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
=======
        //for (int i = 0; i < 1; i++) {
            platform::NUsense::NUgus::ID chain_index = (chains[i])[chain_indices[i]];

            if (packet_handlers[i].check_sts
                <sizeof(platform::NUsense::DynamixelServoReadData)>
                (chain_index)
                == dynamixel::PacketHandler::SUCCESS
            ) {
                // Parse and convert the read data to the local cache.
                process_servo_data(
                    local_cache, 
                    *reinterpret_cast<const dynamixel::StatusReturnCommand<sizeof(
                        platform::NUsense::DynamixelServoReadData
                    )>*>(
                        packet_handlers[i].get_sts_packet()
                    )
                );

                // For now, print the read-bank for testing.
                // Later on, this should be done somewhere else outside of
                // this if-statement and without cout-style output.
                std::stringstream ss;
                ss     << "Port: " << i
                        // For some ungodly reason, stringstream reads a
                        // uint8_t as an ASCII character. Thus, chain_index
                        // needs to cast to a uint16_t.
                    << " Servo: " << ((uint16_t)chain_index)
                    << "\t" << local_cache[(uint8_t)chain_index-1];
                CDC_Transmit_HS((uint8_t*)ss.str().data(), ss.str().size());

                // Wait for a little bit. This is purely for testing.
                HAL_Delay(500);

                // Send a read-instruction for the next servo along the chain.
                packet_handlers[i].reset();
                chain_indices[i] = (chain_indices[i] + 1) % chains[i].size();
                ports[i].write(
                    dynamixel::ReadCommand(
                        (uint8_t)(chains[i])[chain_indices[i]],
                        (uint16_t)platform::NUsense::AddressBook::SERVO_READ,
                        (uint16_t)sizeof(platform::NUsense::DynamixelServoReadData)
                    )
                );
            }
>>>>>>> 7c81a4a620df92baa139a6dc1669529b3993de71
        }
    }
} // namespace platform::NUsense
