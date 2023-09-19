#include "../NUsenseIO.hpp"

#include <sstream>
#include "usbd_cdc_if.h"

namespace platform::NUsense {
    
    void NUsenseIO::loop() {
        // For each port, check whether the expected status has been
        // successfully received. If so, then handle it and send the next read-
        // instruction.
        for (int i = 0; i < NUM_PORTS; i++) {
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
        }
    }
} // namespace platform::NUsense
