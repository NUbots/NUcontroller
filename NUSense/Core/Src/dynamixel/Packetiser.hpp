#ifndef DYNAMIXEL_PACKETISER_HPP
#define DYNAMIXEL_PACKETISER_HPP

#include <array>   // needed for the array container inside the packet
#include <vector>  // needed for the return of the encoding

#include "stdint.h"  // needed for explicit type-defines

namespace dynamixel {

#define PACKETISER_BUFFER_SIZE 2048

    static constexpr uint16_t crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D,
        0x8027, 0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A,
        0x804B, 0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5,
        0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082, 0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D,
        0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA,
        0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145,
        0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E,
        0x0104, 0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A,
        0x832B, 0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356,
        0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2, 0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD,
        0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A,
        0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6,
        0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E,
        0x0264, 0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219,
        0x0208, 0x820D, 0x8207, 0x0202};

    // sshhh ... most of this is stolen from the old NUSense code.

    /**
     * @brief   a packetiser for encoding an outgoing packet and for decoding an incoming packet,
     */
    class Packetiser {
    public:
        /**
         * @brief   Constructs the packetiser.
         */
        Packetiser() : buffer(), filled_length(7), expected_length(0), crc(0), packet_is_ready(false), state(INITIAL) {
            // Set the magic header number.
            buffer[0] = 0xFF;
            buffer[1] = 0xFF;
            buffer[2] = 0xFD;
            buffer[3] = 0x00;
        }
        /**
         * @brief   Encodes a Dynamixel packet with byte-stuffing and CRC.
         * @param   packet the packet to be encoded and modified,
         * @return  the reference to the encoded packet,
         */
        static std::vector<uint8_t>& encode(std::vector<uint8_t>& packet) {
            // Calculate the CRC of the header and the reserved byte.
            uint16_t crc = update_crc(0x00, (uint8_t*) &packet[0], 4);
            State state  = INITIAL;

            // Stuff any bytes after the reserved.
            for (auto it = std::next(packet.begin(), 4); it != std::next(packet.end(), -2); ++it) {
                crc = update_crc(crc, *it);

                // Perform the byte stuffing
                switch (state) {
                    case INITIAL: state = *it == 0xFF ? UNSTUFF_1 : INITIAL; break;
                    case UNSTUFF_1: state = *it == 0xFF ? UNSTUFF_2 : INITIAL; break;
                    case UNSTUFF_2: {
                        if (*it == 0xFD) {
                            crc = update_crc(crc, 0xFD);    // crc stuffing
                            it  = packet.insert(it, 0xFD);  // stuff
                        }
                        state = INITIAL;
                    } break;
                    default: break;  // Will never get here
                }
            }

            // Fix the packet length and crc
            uint16_t unstuffed_size   = packet.size() - 7;
            packet[5]                 = unstuffed_size & 0xFF;
            packet[6]                 = (unstuffed_size >> 8);
            packet[packet.size() - 2] = uint8_t(crc & 0xFF);
            packet[packet.size() - 1] = uint8_t(crc >> 8);

            return packet;
        }

        /**
         * @brief   Decodes bytes into a packet without byte-stuffing.
         * @param   read_byte the next individual byte in the stream,
         * @return  whether the whole packet is done,
         * @retval  #true if the whole packet is done, i.e. fully decoded,
         * @retval  #false if the whole packet has not been fully decoded,
         */
        const bool decode(const uint8_t read_byte) {
            switch (state) {
                case INITIAL: state = read_byte == 0xFF ? HEADER_BYTE_1 : INITIAL; break;
                case HEADER_BYTE_1: state = read_byte == 0xFF ? HEADER_BYTE_2 : INITIAL; break;
                case HEADER_BYTE_2: state = read_byte == 0xFD ? HEADER_BYTE_3 : INITIAL; break;
                case HEADER_BYTE_3: state = read_byte != 0xFD ? READ_ID : INITIAL; break;
                case READ_ID: {
                    buffer[4] = read_byte;
                    crc       = update_crc(update_crc(update_crc(update_crc(0x00, 0xFF), 0xFF), 0xFD), 0x00);
                    crc       = update_crc(crc, buffer[4]);
                    state     = READ_LEN_LOW;
                } break;
                case READ_LEN_LOW: {
                    buffer[5]       = read_byte;
                    crc             = update_crc(crc, buffer[5]);
                    expected_length = buffer[5];
                    state           = READ_LEN_HIGH;
                } break;
                case READ_LEN_HIGH: {
                    // Or in the high byte, and add 6 for the header
                    buffer[6] = read_byte << 8;
                    crc       = update_crc(crc, buffer[6]);
                    expected_length |= buffer[6];
                    expected_length += 7;
                    state = READING;
                } break;
                case READING:
                case UNSTUFF_1:
                case UNSTUFF_2: {
                    uint8_t b = read_byte;

                    // Put the byte in the buffer
                    buffer[filled_length++] = b;

                    // All but the last two bytes go into the CRC and the CRC doesn't do byte
                    // stuffing
                    if (filled_length <= expected_length - 2) {
                        crc = update_crc(crc, b);

                        // Keep track as we go past byte stuffing
                        switch (state) {
                            default:
                            case READING: state = b == 0xFF ? UNSTUFF_1 : READING; break;
                            case UNSTUFF_1: state = b == 0xFF ? UNSTUFF_2 : READING; break;
                            case UNSTUFF_2: state = b == 0xFD ? UNSTUFF_3 : READING; break;
                        }
                    }

                    // Packet has been read reset the internal state for the next packet
                    if (filled_length == expected_length) {
                        // Fix the packet length given the unstuffed size
                        uint16_t unstuffed_size = expected_length - 7;
                        buffer[5]               = unstuffed_size & 0xFF;
                        buffer[6]               = unstuffed_size >> 8;

                        packet_is_ready = true;
                        return packet_is_ready;
                    }

                } break;
                case UNSTUFF_3: {
                    uint8_t b = read_byte;
                    switch (b) {
                        case 0xFD: {
                            --expected_length;           // Stuffing, expect one less byte
                            crc   = update_crc(crc, b);  // We still CRC the stuffing
                            state = READING;
                        } break;
                        case 0x00: state = READ_ID; break;  // Somehow this is a header?
                        default: reset(); break;            // What just happened?
                    }
                } break;
            }

            return false;
        }

        /**
         * @brief   Gets the pointer to the decoded packet.
         */
        const uint8_t* get_decoded_packet() const {
            return buffer.data();
        }
        /**
         * @brief   Gets the length of the decoded packet.
         */
        const uint16_t get_decoded_length() const {
            return filled_length;
        }
        /**
         * @brief   Gets the computed CRC of the decoded packet.
         */
        const uint16_t get_decoded_crc() const {
            return crc;
        }
        /**
         * @brief   Gets whether a packet is ready.
         */
        const bool is_packet_ready() const {
            return packet_is_ready;
        }
        const bool has_begun() const {
            return (expected_length != 0);
        }
        /**
         * @brief   Resets the state of the packetiser.
         */
        void reset() {
            // Set back to initial state
            state           = INITIAL;
            expected_length = 0;

            // Start off with header + id + size
            filled_length   = 7;
            crc             = 0;
            packet_is_ready = false;
        }

    private:
        /// @brief  the buffer to store the decoded packet,
        std::array<uint8_t, PACKETISER_BUFFER_SIZE> buffer;
        /// @brief  the number of bytes filled in the buffer,
        uint16_t filled_length;
        /// @brief  the total expected length of the packet,
        uint16_t expected_length;
        /// @brief  the accumulated CRC value,
        uint16_t crc;
        /// @brief  whether the decoded packet is done, i.e. fully decoded,
        bool packet_is_ready;
        /// @brief  the state of the decoded packet,
        enum State {
            INITIAL,        // No bytes
            HEADER_BYTE_1,  // Received 0xFF
            HEADER_BYTE_2,  // Received 0xFFFF
            HEADER_BYTE_3,  // Received 0xFFFFFD
            READ_ID,        // Received 0xFFFFFD00 (header)
            READ_LEN_LOW,   // Read header + id
            READ_LEN_HIGH,  // Read Header + id + lower byte of len
            READING,        // Read Header + id + len
            UNSTUFF_1,      // Seen a 0xFF while reading
            UNSTUFF_2,      // Seen 0xFFFF while reading
            UNSTUFF_3,      // Seen 0xFFFFFD while reading, if next byte is 0xFD drop it
        };
        State state;

        // Helper functions:
        /**
         * @brief   Updates the CRC for a bulk of bytes.
         * @note    This function is taken from Robotis.
         */
        static uint16_t update_crc(uint16_t crc_accum, const uint8_t* data_blk_ptr, const uint16_t data_blk_size) {
            uint16_t i, j;

            for (j = 0; j < data_blk_size; j++) {
                i         = ((uint16_t) (crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
                crc_accum = (crc_accum << 8) ^ crc_table[i];
            }

            return crc_accum;
        }

        /**
         * @brief   Updates the CRC for a single byte.
         * @note    This function is taken from Robotis.
         */
        static uint16_t update_crc(uint16_t crc_accum, const uint8_t byte) {
            uint16_t i;

            i         = ((uint16_t) (crc_accum >> 8) ^ byte) & 0xFF;
            crc_accum = (crc_accum << 8) ^ crc_table[i];

            return crc_accum;
        }
    };

}  // namespace dynamixel

#endif  // DYNAMIXEL_PACKETISER_HPP
