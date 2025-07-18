#ifndef NUSENSE_NUSENSEIO_HPP
#define NUSENSE_NUSENSEIO_HPP

#include <array>
#include <iterator>
#include <stdio.h>

#include "../device/Buzzer.hpp"
#include "../device/back_panel/Button.hpp"
#include "../device/back_panel/Led.hpp"
#include "../device/back_panel/RgbLed.hpp"
#include "../dynamixel/Chain.hpp"
#include "../dynamixel/Dynamixel.hpp"
#include "../dynamixel/PacketHandler.hpp"
#include "../uart/Port.hpp"
#include "../usb/PacketHandler.hpp"
#include "../usb/protobuf/NUSenseData.pb.h"
#include "../usb/protobuf/pb_encode.h"
#include "../utility/message/hash.hpp"
#include "../utility/support/MillisecondTimer.hpp"
#include "ChainManager.hpp"
#include "NUgus.hpp"
#include "ServoState.hpp"
#include "imu.h"

namespace nusense {
    constexpr uint32_t MAX_ENCODE_SIZE = 1600;
    constexpr uint8_t NUM_PORTS        = 6;
    constexpr uint8_t NUM_CHAINS       = NUM_PORTS;

    class NUSenseIO {
    private:
        /// @brief  These are the ports on the NUSense board. They are either to be used for
        ///         sending packets directly or to be passed to a packet-handler.
        std::array<uart::Port, NUM_PORTS> ports =
            {uart::Port(1), uart::Port(2), uart::Port(3), uart::Port(4), uart::Port(5), uart::Port(6)};

        /// @brief  This is the local storage of each servo's state. This is to be updated
        ///         regularly by polling the servos constantly and to be spammed to the NUC.
        std::array<nusense::ServoState, NUMBER_OF_DEVICES> servo_states{};

        /// @brief  Collection of Chain objects used to interface with the servos.
        ChainManager<NUM_CHAINS> chain_manager;

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

        /// @brief The nanopb generated struct to contain the handshake message
        message_platform_NUSenseHandshake handshake_msg = message_platform_NUSenseHandshake_init_zero;

        /// @brief The nanopb generated struct which contains the IDs found by discovery
        message_platform_ServoIDStates servo_id_states_msg = message_platform_ServoIDStates_init_zero;

        /// @brief NUgus object to get access to a list of the servos
        NUgus nugus;

        /// @brief  The IMU instance
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
        device::back_panel::Button mode_button = device::back_panel::Button(GPIOC, 15);

        /// @brief  The SW_START button
        device::back_panel::Button start_button = device::back_panel::Button(GPIOH, 0);

        /// @brief  The LEDs on the back-panel, arranged in the order from left to right.
        device::back_panel::Led tx_led      = device::back_panel::Led(GPIOC, 14);
        device::back_panel::Led rx_led      = device::back_panel::Led(GPIOC, 13);
        device::back_panel::Led red_led     = device::back_panel::Led(GPIOE, 6);
        device::back_panel::Led blue_led    = device::back_panel::Led(GPIOE, 5);
        device::back_panel::Led green_led   = device::back_panel::Led(GPIOE, 4);
        device::back_panel::RgbLed left_rgb = device::back_panel::RgbLed(
            {device::back_panel::Led{GPIOE, 3}, device::back_panel::Led{GPIOE, 1}, device::back_panel::Led{GPIOE, 2}});
        device::back_panel::RgbLed right_rgb = device::back_panel::RgbLed(
            {device::back_panel::Led{GPIOE, 0}, device::back_panel::Led{GPIOB, 8}, device::back_panel::Led{GPIOB, 9}});

        /// @brief  The on-board buzzer
        device::Buzzer buzzer = device::Buzzer(GPIOB, 7);

        /// @brief  Whether any servo is too hot.
        bool any_servo_hot = false;

    public:
        /// @brief   Constructs the instance for NUSense communications.
        NUSenseIO()
            : chain_manager({dynamixel::Chain(ports[0], 0),
                             dynamixel::Chain(ports[1], 1),
                             dynamixel::Chain(ports[2], 2),
                             dynamixel::Chain(ports[3], 3),
                             dynamixel::Chain(ports[4], 4),
                             dynamixel::Chain(ports[5], 5)})
            , imu() {}

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

        /// @brief   Serialise the given data into the nbs format and send it to the NUC.
        /// @tparam  MessageType the type of the message to serialise.
        /// @param   message_object The message object to serialise.
        /// @param   message_hash The hash of the message to serialise.
        /// @param   message_fields The fields of the message to serialise.
        /// @return  Whether the message was serialised and sent successfully.
        template <typename MessageType>
        bool encode_and_transmit_nbs(const MessageType& message_object,
                                     const uint64_t& message_hash,
                                     const pb_msgdesc_t* message_fields);

        /// @brief   Sends a write-instruction for the first write-bank of registers.
        /// @param   chain the chain of servos to send the write-instruction to.
        void send_servo_write_1_request(dynamixel::Chain& chain);

        /// @brief   Sends a write-instruction for the second write-bank of registers.
        /// @param   chain the chain of servos to send the write-instruction to.
        void send_servo_write_2_request(dynamixel::Chain& chain);

        /// @brief   Sends a serialised message_platform_nusense to the nuc via usb.
        /// @return  Whether the message was sent successfully.
        bool nusense_to_nuc();

        /// @brief   Expects to receive a handshake message from the NUC
        /// @return  Whether the handshake process succeeded
        bool handshake_received();
    };

    template <typename MessageType>
    bool NUSenseIO::encode_and_transmit_nbs(const MessageType& message_object,
                                            const uint64_t& message_hash,
                                            const pb_msgdesc_t* message_fields) {
        // Once everything else is filled we send it to the NUC. Just overwrite the bytes within encoding_payload
        // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
        // If one wishes to add messages to the protobuf message, one must first calculate the maximum bytes
        // within that message and then add enough bytes to make sure that nanopb doesn't cry about the output stream
        // being too small If the MAX_ENCODE_SIZE is inadequately defined, one can get a corrupted message and nanopb
        // errors.
        pb_ostream_t output_buffer = pb_ostream_from_buffer(&encoding_payload[0], MAX_ENCODE_SIZE);

        // TODO (NUSense people) Handle encoding errors properly using this member somehow
        if (!pb_encode(&output_buffer, message_fields, &message_object)) {
            return false;
        }

        // Happiness, the encoding succeeded
        std::vector<uint8_t> nbs({0xE2, 0x98, 0xA2});

        // TODO (JohanneMontano) Implement timestamp field correctly, std::chrono is behaving weird and it needs to be
        // investigated
        uint64_t ts_u = 0;
        uint32_t size = uint32_t(output_buffer.bytes_written + sizeof(message_hash) + sizeof(ts_u));

        // Encode size to uint8_t's
        for (size_t i = 0; i < sizeof(size); ++i) {
            nbs.push_back(uint8_t((size >> (i * 8)) & 0xFF));
        }

        // Encode timestamp
        for (size_t i = 0; i < sizeof(ts_u); ++i) {
            nbs.push_back(uint8_t((ts_u >> (i * 8)) & 0xFF));
        }

        // Encode nusense hash
        for (size_t i = 0; i < sizeof(message_hash); ++i) {
            nbs.push_back(uint8_t((message_hash >> (i * 8)) & 0xFF));
        }

        // Add the protobuf bytes into the nbs vector
        nbs.insert(nbs.end(), std::begin(encoding_payload), std::begin(encoding_payload) + output_buffer.bytes_written);

        // Attempt to transmit data then handle it accordingly if it fails
        if (CDC_Transmit_HS(nbs.data(), nbs.size()) != USBD_OK) {
            // Going into this block means that the usb failed to transmit our data
            usb_tx_err = true;
            return false;
        }

        return true;
    }

}  // namespace nusense

#endif  // NUSENSE_NUSENSEIO_HPP
