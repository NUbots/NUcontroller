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

                    pb_length = static_cast<uint16_t>(rx_buffer.data[(rx_buffer.front + 3) % RX_BUF_SIZE] << 8)
                                | static_cast<uint16_t>(rx_buffer.data[(rx_buffer.front + 4) % RX_BUF_SIZE]);

                    // If the overall packet, including the header, is smaller than
                    // the current size of the buffer, then pop all of the payload.
                    if ((pb_length + 5) <= rx_buffer.size) {
                        pop((uint8_t*) pb_packets, pb_length, 5);
                        is_packet_ready = true;
                    }
                    // Else, work out what the remaining length is, that is the
                    // payload's length minus the buffer's size, excluding the
                    // five bytes of header.
                    else {
                        remaining_length = pb_length - rx_buffer.size + 5;
                        pop((uint8_t*) pb_packets, rx_buffer.size - 5, 5);
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

                // Decoding the protobuf packet
                pb_istream_t input_stream =
                    pb_istream_from_buffer(reinterpret_cast<const pb_byte_t*>(&pb_packets[0]), pb_length);
                pb_decode(&input_stream, message_actuation_ServoTargets_fields, &targets);

                return true;
            }

            return false;
        }

        /// @brief   Gets the targets of the last decoded packet.
        /// @return  The pointer to the targets.
        message_actuation_ServoTargets* get_targets() {
            return &targets;
        }

    private:
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
        uint16_t pb_length = 0;

        /// @brief  the remaining length of the protobuf packet to be gathered by the lower-level
        ///         firmware, namely CDC_Receive_HS.
        uint16_t remaining_length = 0;

        /// @brief  whether a complete protobuf packet has been gathered to be decoded,
        bool is_packet_ready = false;

        /// @brief The servo targets to send to the servos
        message_actuation_ServoTargets targets = message_actuation_ServoTargets_init_zero;
    };

}  // namespace usb

#endif  // USB_PACKETHANDLER_HPP
