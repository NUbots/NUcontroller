#include <algorithm>
#include <cstdint>
#include <iterator>

#include "protobuf/ServoTarget.pb.h"
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"
#include "usbd_cdc_if.h"

#ifndef USB_PACKETHANDLER_HPP
    #define USB_PACKETHANDLER_HPP

namespace usb {

    /// @brief   Handles the USB protobuf packets.
    class PacketHandler {
    public:
        /// @brief   Constructs the packet handler.
        PacketHandler() {
            rx_buffer.front = 0;
            rx_buffer.back  = 0;
            rx_buffer.size  = 0;
        }

        /// @brief   Handles outgoing bytes from the ring-buffer, parses any packet, and decodes it.
        /// @return  Whether the packet has been decoded.
        bool handle_incoming() {

            if (rx_buffer.size != 0) {
                HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
                // Check if we have a header and if we do extract our lengths and pb bytes
                if ((rx_buffer.data[rx_buffer.front] == (char) 0xE2)
                    && (rx_buffer.data[(rx_buffer.front + 1) % RX_BUF_SIZE] == (char) 0x98)
                    && (rx_buffer.data[(rx_buffer.front + 2) % RX_BUF_SIZE] == (char) 0xA2)) {

                    pb_length = static_cast<uint32_t>(rx_buffer.data[(rx_buffer.front + 3) % RX_BUF_SIZE] << 0)
                                | static_cast<uint32_t>(rx_buffer.data[(rx_buffer.front + 4) % RX_BUF_SIZE] << 8)
                                | static_cast<uint32_t>(rx_buffer.data[(rx_buffer.front + 5) % RX_BUF_SIZE] << 16)
                                | static_cast<uint32_t>(rx_buffer.data[(rx_buffer.front + 6) % RX_BUF_SIZE] << 24);

                    // If the overall packet, including the header, is smaller than
                    // the current size of the buffer, then pop all of the payload.
                    if ((pb_length + 7) <= rx_buffer.size) {
                        pop((uint8_t*) pb_packets, pb_length, 7);
                        is_packet_ready = true;
                    }
                    // Else, work out what the remaining length is, that is the
                    // payload's length minus the buffer's size, excluding the
                    // three bytes of header and four bytes for size.
                    else {
                        remaining_length = pb_length - rx_buffer.size + 7;
                        pop((uint8_t*) pb_packets, rx_buffer.size - 7, 7);
                    }
                }
                else if (remaining_length != 0) {
                    uint16_t old_size;
                    old_size = pop((uint8_t*) &pb_packets[pb_length - remaining_length],
                                   remaining_length <= rx_buffer.size ? remaining_length : rx_buffer.size);
                    remaining_length -= old_size;
                    if (remaining_length == 0)
                        is_packet_ready = true;
                }
                else {
                    // Update index accessor after receiving a packet, making sure to wrap around
                    // in case it exceeds the buffer's length
                    rx_buffer.front = (rx_buffer.front + 1) % RX_BUF_SIZE;
                    rx_buffer.size--;
                }
                HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
            }

            if (is_packet_ready) {
                is_packet_ready = false;

                // Create a buffer to hold our timestamp and hash packets in
                uint8_t tmp_buf[8] = {0};

                // Fill timestamp
                memcpy(&tmp_buf[0], &pb_packets[0], sizeof(uint64_t));
                msg_timestamp = read_le_64(&tmp_buf[0]);

                // Fill hash
                memcpy(&tmp_buf[0], &pb_packets[sizeof(uint64_t)], sizeof(uint64_t));
                msg_hash = read_le_64(&tmp_buf[0]);

                // Decoding the protobuf packet
                pb_istream_t input_stream = pb_istream_from_buffer(
                    reinterpret_cast<const pb_byte_t*>(&pb_packets[sizeof(uint64_t) + sizeof(uint64_t)]),
                    pb_length);
                
                pb_decode(&input_stream, message_actuation_SubcontrollerServoTargets_fields, &targets);
                return true;

                // TODO (JohanneMontano) nanopb is complaining about something if we handle it using the code below. It seems like the nanopb error
                // always happens but also always successfully decodes the SubcontrollerServoTargets message anyway. This must be investigated.
                // nanopb_decoding_err = pb_decode(&input_stream, message_actuation_SubcontrollerServoTargets_fields, &targets) ? false : true;
                // if (!nanopb_decoding_err) {
                //     return true;
                // }
            }
            return false;
        }

        /// @brief Getter for the member nanopb_decode_err
        /// @brief A boolean value that describes the state of the most recent pb_decode() call
        bool get_nanopb_decoding_status() {
            return nanopb_decoding_err;
        }

        /// @brief Get the hash of the most recently decoded message
        /// @return 64 bit hash of the message type that the NUC sent
        uint64_t get_curr_msg_hash() {
            return msg_hash;
        }

        /// @brief Get the timestamp of the most recently decoded message
        /// @return 64 bit timestamp of the most recently decoded message
        uint64_t get_curr_msg_timestamp() {
            return msg_timestamp;
        }

        /// @brief   Gets the targets of the last decoded packet.
        /// @return  The pointer to the targets.
        message_actuation_SubcontrollerServoTargets* get_targets() {
            return &targets;
        }

    private:
        /**
         * @brief Read a 64 byte message from a buffer of uint8_t[8]. Mainly used for timestamps and message hashes.
         * @param ptr The pointer to the bytes buffer
         * @return the decoded 64 bit value, either for timestamps or hashes
         */
        uint64_t read_le_64(const uint8_t* ptr) {
            return (uint64_t(ptr[0]) << 0) | (uint64_t(ptr[1]) << 8) | (uint64_t(ptr[2]) << 16)
                   | (uint64_t(ptr[3]) << 24) | (uint64_t(ptr[4]) << 32) | (uint64_t(ptr[5]) << 40)
                   | (uint64_t(ptr[6]) << 48) | (uint64_t(ptr[7]) << 56);
        }

        /**
         * @brief   Removes bytes from the ring-buffer of a given length, passing from a given
         *          offset.
         * @note    This is a helper function; it is not meant to encapsulate anything.
         * @param   bytes the bytes that would be taken,
         * @param   length the number of bytes to take,
         * @param   offset the number of bytes skipped first,
         * @note    Normally, offset would be either zero for a traditional pop, or 5 to ignore the
         *          header.
         * @return  the number of bytes removed,
         */
        uint16_t pop(volatile uint8_t* bytes, uint16_t length, uint16_t offset = 0) {
            // Update the front to move back (higher) in the array unless there
            // is nothing left in the buffer.
            if (rx_buffer.size >= (length + offset)) {
                // If the bytes to be popped span across No Man's Land, then use two distinct
                // copies.
                if ((uint16_t(rx_buffer.front + length + offset) >= RX_BUF_SIZE)
                    && (uint16_t(rx_buffer.front + offset) < RX_BUF_SIZE)) {
                    std::copy(&rx_buffer.data[(rx_buffer.front + offset) % RX_BUF_SIZE],
                              &rx_buffer.data[RX_BUF_SIZE],
                              &bytes[0]);
                    std::copy(&rx_buffer.data[0],
                              &rx_buffer.data[length - RX_BUF_SIZE + rx_buffer.front + offset],
                              &bytes[RX_BUF_SIZE - rx_buffer.front - offset]);
                }
                // Else, use one straightforward copy.
                else {
                    std::copy(&rx_buffer.data[(rx_buffer.front + offset) % RX_BUF_SIZE],
                              &rx_buffer.data[(rx_buffer.front + offset + length) % RX_BUF_SIZE],
                              &bytes[0]);
                }
                // Move the front forward and decrease the size.
                rx_buffer.front = (rx_buffer.front + length + offset) % RX_BUF_SIZE;
                rx_buffer.size -= length + offset;
            }
            return length;
        }

        /// @brief  the buffer for the protobuf payload to be decoded,
        char pb_packets[RX_BUF_SIZE]{};

        /// @brief  the length of the protobuf packet,
        uint32_t pb_length = 0;

        /// @brief the hash of the received message
        uint64_t msg_hash = 0;

        /// @brief the
        uint64_t msg_timestamp = 0;

        /// @brief  the remaining length of the protobuf packet to be gathered by the lower-level
        ///         firmware, namely CDC_Receive_HS.
        uint32_t remaining_length = 0;

        /// @brief  whether a complete protobuf packet has been gathered to be decoded,
        bool is_packet_ready = false;

        /// @brief The servo targets to send to the servos
        message_actuation_SubcontrollerServoTargets targets = message_actuation_SubcontrollerServoTargets_init_zero;

        /// @brief A flag that describes the status of the most recent call to pb_decode
        bool nanopb_decoding_err = false;
    };

}  // namespace usb

#endif  // USB_PACKETHANDLER_HPP
