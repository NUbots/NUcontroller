#ifndef PLATFORM_NUSENSE_NUSENSEIO_HPP
#define PLATFORM_NUSENSE_NUSENSEIO_HPP

#include "../../uart/Port.hpp"
#include "../ServoState.hpp"
#include "NUgus.hpp"
#include "../../dynamixel/PacketHandler.hpp"
#include <array>
#include "../../dynamixel/Dynamixel.hpp"
#include "../../usb/PacketHandler.hpp"

namespace platform::NUsense {

    constexpr uint8_t NUM_PORTS = 6;

    class NUsenseIO {
    private:
        /// @brief  These are the ports on the NUsense board. They are either to be used for
        ///         sending packets directly or to be passed to a packet-handler.
        std::array<uart::Port,NUM_PORTS> ports;

        /// @brief  This is the local storage of each servo's state. This is to be updated
        ///         regularly by polling the servos constantly and to be spammed to the NUC.
        std::array<platform::ServoState, NUMBER_OF_DEVICES> servo_states;

        /// @brief  This is the list of known servos on each port or daisy-chain.
        ///         For now, it is written a priori until servo-discovery is made.
        std::array<std::vector<NUgus::ID>,NUM_PORTS> chains;

        /// @brief  These are permanent packet-handlers, one for each port to handle incoming 
        ///         statuses.
        std::array<dynamixel::PacketHandler,NUM_PORTS> packet_handlers;

        /// @brief  These are the indices of each chain to keep track of which servos the read 
        ///         request is up thereto.
        std::array<uint8_t,NUM_PORTS> chain_indices;

        enum StatusState {
            READ_RESPONSE     = 0,
            WRITE_1_RESPONSE  = 1,
            WRITE_2_RESPONSE  = 2
        };
        /// @brief  These are the states of all expected statuses.
        /// @note   This is to keep track what the original instruction was for so that one can 
        ///         tell what the next one is.
        std::array<StatusState, NUMBER_OF_DEVICES> status_states;

        /// @brief  This is the packet-handler for protobuf messages.
        /// @note   Any better name than 'nuc' is welcome.
        usb::PacketHandler nuc;

    public:

        /**
         * @brief   Constructs the instance for NUsense communications.
         */
        NUsenseIO() :
            ports({{
                uart::Port(1),uart::Port(2),uart::Port(3),
                uart::Port(4),uart::Port(5),uart::Port(6)
            }}),
            servo_states(),
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
            }),//*/
			/*chains({
				std::vector<platform::NUsense::NUgus::ID>{
						platform::NUsense::NUgus::ID::R_SHOULDER_PITCH,
						platform::NUsense::NUgus::ID::R_SHOULDER_ROLL,
						platform::NUsense::NUgus::ID::R_ELBOW
				},
				std::vector<platform::NUsense::NUgus::ID>{
						platform::NUsense::NUgus::ID::L_SHOULDER_PITCH,
						platform::NUsense::NUgus::ID::L_SHOULDER_ROLL,
						platform::NUsense::NUgus::ID::L_ELBOW
				},
				std::vector<platform::NUsense::NUgus::ID>{
						platform::NUsense::NUgus::ID::R_HIP_YAW,
						platform::NUsense::NUgus::ID::R_HIP_ROLL,
						platform::NUsense::NUgus::ID::R_HIP_PITCH,
						platform::NUsense::NUgus::ID::R_KNEE,
						platform::NUsense::NUgus::ID::R_ANKLE_PITCH,
						platform::NUsense::NUgus::ID::R_ANKLE_ROLL
				},
				std::vector<platform::NUsense::NUgus::ID>{
						platform::NUsense::NUgus::ID::L_HIP_YAW,
						platform::NUsense::NUgus::ID::L_HIP_ROLL,
						platform::NUsense::NUgus::ID::L_HIP_PITCH,
						platform::NUsense::NUgus::ID::L_KNEE,
						platform::NUsense::NUgus::ID::L_ANKLE_PITCH,
						platform::NUsense::NUgus::ID::L_ANKLE_ROLL
				},
				std::vector<platform::NUsense::NUgus::ID>{
						platform::NUsense::NUgus::ID::HEAD_YAW,
						platform::NUsense::NUgus::ID::HEAD_PITCH
				},
				std::vector<platform::NUsense::NUgus::ID>{
				}
			}),*/
            // Make a packet-handler for each port.
            packet_handlers({
                dynamixel::PacketHandler(ports[0]),
                dynamixel::PacketHandler(ports[1]),
                dynamixel::PacketHandler(ports[2]),
                dynamixel::PacketHandler(ports[3]),
                dynamixel::PacketHandler(ports[4]),
                dynamixel::PacketHandler(ports[5])
            }),
            nuc()
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
         * @param   packet the packet-structure to parse,
         */
        void process_servo_data(
            const dynamixel::StatusReturnCommand<sizeof(DynamixelServoReadData)> packet
        );

        /**
         * @brief   Sends a read-instruction for the read-bank of registers.
         * @param   id the Dynamixel ID of the servo,
         * @param   port_i the index of the port on which to send,
         */
        void send_servo_read_request(const NUgus::ID id, const uint8_t port_i);

        /**
         * @brief   Sends a write-instruction for the first write-bank of registers.
         * @param   id the Dynamixel ID of the servo,
         * @param   port_i the index of the port on which to send,
         */
        void send_servo_write_1_request(const NUgus::ID id, const uint8_t port_i);

        /**
         * @brief   Sends a write-instruction for the second write-bank of registers.
         * @param   id the Dynamixel ID of the servo,
         * @param   port_i the index of the port on which to send,
         */
        void send_servo_write_2_request(const NUgus::ID id, const uint8_t port_i);

    };

} // namespace platform::NUsense

#endif  // PLATFORM_NUSENSE_NUSENSEIO_HPP
