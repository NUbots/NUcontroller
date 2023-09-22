#ifndef PLATFORM_NUSENSE_NUSENSEIO_HPP
#define PLATFORM_NUSENSE_NUSENSEIO_HPP

#include "../../uart/Port.hpp"
#include "../ServoState.hpp"
#include "NUgus.hpp"
#include "../../dynamixel/PacketHandler.hpp"
#include <array>
#include "../../dynamixel/Dynamixel.hpp"

namespace platform::NUsense {

    class NUsenseIO {
    private:
        /// @brief  These are the ports on the NUsense board. They are either to be used for
        ///         sending packets directly or to be passed to a packet-handler.
        std::array<uart::Port,NUM_PORTS> ports;

        /// @brief  This is the local storage of each servo's state. This is to be updated
        ///         regularly by polling the servos constantly and to be spammed to the NUC.
        std::array<platform::ServoState, NUMBER_OF_DEVICES> local_cache;

        /// @brief  This is the list of known servos on each port or daisy-chain.
        ///         For now, it is written a priori until servo-discovery is made.
        std::array<std::vector<NUgus::ID>,NUM_PORTS> chains;

        /// @brief  These are permanent packet-handlers, one for each port to handle incoming 
        ///         statuses.
        std::array<dynamixel::PacketHandler,NUM_PORTS> packet_handlers;

        /// @brief  These are the indices of each chain to keep track of which servos the read 
        ///         request is up thereto.
        std::array<uint8_t,NUM_PORTS> chain_indices;

    public:

        /**
         * @brief   Constructs the instance for NUsense communications.
         */
        NUsenseIO() :
            ports({{
                uart::Port(1),uart::Port(2),uart::Port(3),
                uart::Port(4),uart::Port(5),uart::Port(6)
            }}),
            local_cache(),
            // For now, this is just a very crude way of knowing what devices are on what port 
            // without polling each port. Later we will get polling at start-up so that the 
            // devices do not have to be connected to a specific port.
            chains({
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::R_SHOULDER_PITCH,
                        platform::NUsense::NUgus::ID::R_SHOULDER_ROLL,
                        platform::NUsense::NUgus::ID::R_ELBOW,
                        platform::NUsense::NUgus::ID::HEAD_YAW
                },
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::L_SHOULDER_PITCH,
                        platform::NUsense::NUgus::ID::L_SHOULDER_ROLL,
                        platform::NUsense::NUgus::ID::L_ELBOW,
                        platform::NUsense::NUgus::ID::HEAD_PITCH
                },
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::R_HIP_YAW,
                        platform::NUsense::NUgus::ID::R_HIP_ROLL,
                        platform::NUsense::NUgus::ID::R_HIP_PITCH
                },
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::L_HIP_YAW,
                        platform::NUsense::NUgus::ID::L_HIP_ROLL,
                        platform::NUsense::NUgus::ID::L_HIP_PITCH
                },
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::R_KNEE,
                        platform::NUsense::NUgus::ID::R_ANKLE_PITCH,
                        platform::NUsense::NUgus::ID::R_ANKLE_ROLL
                },
                std::vector<platform::NUsense::NUgus::ID>{
                        platform::NUsense::NUgus::ID::L_KNEE,
                        platform::NUsense::NUgus::ID::L_ANKLE_PITCH,
                        platform::NUsense::NUgus::ID::L_ANKLE_ROLL
                }
            }),
            // Make a packet-handler for each port.
            packet_handlers({
                dynamixel::PacketHandler(ports[0]),
                dynamixel::PacketHandler(ports[1]),
                dynamixel::PacketHandler(ports[2]),
                dynamixel::PacketHandler(ports[3]),
                dynamixel::PacketHandler(ports[4]),
                dynamixel::PacketHandler(ports[5])
            })
        {
            // Begin at the beginning of the chains.
            chain_indices.fill(0);
        }

        /** 
         * @brief   Begins the ports and sets the servos up with indirect 
         *          addresses, etc.
         * @note    Is loosely inspired by startup() in HardwareIO.
         */
        void startup();

        /**
         * @brief   Handles everything.
         * @note    This should eventually be replaced more detailed 
         *          functions like that in HardwareIO.
         */
        void loop();

        /**
         * @brief   Helps parse the read data from a servo.
         * @note    Is taken from HardwareIO.
         */
        void process_servo_data(
            std::array<platform::ServoState,NUMBER_OF_DEVICES>& servo_states,
            const dynamixel::StatusReturnCommand<sizeof(DynamixelServoReadData)> packet
        );

    };

} // namespace platform::NUsense

#endif  // PLATFORM_NUSENSE_NUSENSEIO_HPP