#ifndef PLATFORM_NUSENSE_NUSENSEIO_HPP
#define PLATFORM_NUSENSE_NUSENSEIO_HPP

#include <array>
#include <stdio.h>

#include "../../dynamixel/Chain.hpp"
#include "../../dynamixel/Dynamixel.hpp"
#include "../../dynamixel/PacketHandler.hpp"
#include "../../uart/Port.hpp"
#include "../../usb/PacketHandler.hpp"
#include "../../usb/protobuf/NUSenseData.pb.h"
#include "../../usb/protobuf/pb_encode.h"
#include "../../utility/support/Button.hpp"
#include "../../utility/support/MillisecondTimer.hpp"
#include "../ServoState.hpp"
#include "NUgus.hpp"
#include "imu.h"

namespace platform::NUSense {
    constexpr uint32_t MAX_ENCODE_SIZE = 1600;
    constexpr uint8_t NUM_PORTS        = 6;

    class NUSenseIO {
    private:
        /// @brief  These are the ports on the NUSense board. They are either to be used for
        ///         sending packets directly or to be passed to a packet-handler.
        std::array<uart::Port, NUM_PORTS> ports =
            {uart::Port(1), uart::Port(2), uart::Port(3), uart::Port(4), uart::Port(5), uart::Port(6)};

        /// @brief  This is the local storage of each servo's state. This is to be updated
        ///         regularly by polling the servos constantly and to be spammed to the NUC.
        std::array<platform::ServoState, NUMBER_OF_DEVICES> servo_states{};

        /// @brief  This is the list of known servos on each port or daisy-chain.
        ///         For now, it is written a priori until servo-discovery is made.
        std::array<dynamixel::Chain, NUM_PORTS> chains = {dynamixel::Chain(ports[0]),
                                                          dynamixel::Chain(ports[1]),
                                                          dynamixel::Chain(ports[2]),
                                                          dynamixel::Chain(ports[3]),
                                                          dynamixel::Chain(ports[4]),
                                                          dynamixel::Chain(ports[5])};

        enum StatusState { READ_RESPONSE = 0, WRITE_1_RESPONSE = 1, WRITE_2_RESPONSE = 2, WRITE_1_COOLDOWN = 3 };
        /// @brief  These are the states of all expected statuses.
        /// @note   This is to keep track what the original instruction was for so that one can
        ///         tell what the next one is.
        std::array<StatusState, NUMBER_OF_DEVICES> status_states{};
        /// @brief  This is the packet-handler for the serialised protobuf messages sent by the NUC.
        /// @note   Any better name than 'nuc' is welcome.
        usb::PacketHandler nuc{};

        /// @brief  This is the nanopb generated struct which will contain all the states
        ///         to serialise and sent to the NUC
        message_platform_NUSense nusense_msg = message_platform_NUSense_init_zero;

        /// @brief   The IMU instance
        IMU imu{};

        /// @brief  Nanopb will put the serialised bytes in this container. For some reason, the encode
        ///         function does not work with c++ defined data structures hence we use a c array for it
        uint8_t encoding_payload[2048]{};

        /// @brief  Flag to catch failed usb transmits for debugging / handling
        bool usb_tx_err = false;

        /// @brief  Flag to catch failed nanopb encode calls for debugging / handling
        bool nanopb_encoding_err = false;

        /// @brief  This is to synchronise the data sent to the NUC as well as the buttons, etc.
        utility::support::MillisecondTimer loop_timer{};

        /// @brief  The SW_MODE button
        utility::support::Button mode_button = utility::support::Button(GPIOC, 15);

        /// @brief  The SW_START button
        utility::support::Button start_button = utility::support::Button(GPIOH, 0);

    public:
        /// @brief   Constructs the instance for NUSense communications.
        NUSenseIO() {
            // Begin IMU for polling
            imu.init();
        }

        /// @brief   Begins the ports and sets the servos up with indirect addresses, etc.
        /// @note    Is loosely inspired by startup() in NUbots/NUbots OpenCR HardwareIO.
        void startup();

        /// @brief   Handles the loop of communicating with the sensors and the NUC.
        /// @note    This should eventually be replaced more detailed functions like that in HardwareIO.
        void loop();

        /// @brief   Parse the read data from a servo.
        /// @note    Is taken from NUbots/NUbots OpenCR HardwareIO.
        /// @param   packet the packet-structure to parse.
        void process_servo_data(const dynamixel::StatusReturnCommand<sizeof(DynamixelServoReadData)> packet);

        /// @brief   Sends a read-instruction for the read-bank of registers.
        /// @param   chain the chain of servos to send the read-instruction to.
        void send_servo_read_request(dynamixel::Chain& chain);

        /// @brief   Sends a write-instruction for the first write-bank of registers.
        /// @param   chain the chain of servos to send the write-instruction to.
        void send_servo_write_1_request(dynamixel::Chain& chain);

        /// @brief   Sends a write-instruction for the second write-bank of registers.
        /// @param   chain the chain of servos to send the write-instruction to.
        void send_servo_write_2_request(dynamixel::Chain& chain);

        /// @brief   Sends a serialised message_platform_NUSense to the nuc via usb.
        /// @return  Whether the message was sent successfully.
        bool nusense_to_nuc();
    };

}  // namespace platform::NUSense

#endif  // PLATFORM_NUSENSE_NUSENSEIO_HPP
