#include "../nusense/NUgus.hpp"
#include "../uart/Port.hpp"
#include "../utility/support/MicrosecondTimer.hpp"
#include "Dynamixel.hpp"
#include "Packetiser.hpp"
#include "signal.h"

#ifndef DYNAMIXEL_PACKETHANDLER_HPP
    #define DYNAMIXEL_PACKETHANDLER_HPP

namespace dynamixel {

    /**
     * @brief    the handler for any generic packet
     */
    class PacketHandler {
    public:
        /// @brief  the result of whether all the status-packets have been received,
        enum Result { NONE = 0x00, PARTIAL, SUCCESS, ERROR, CRC_ERROR, TIMEOUT };

        /**
         * @brief    Constructs the packet-handler.
         * @param    port the reference to the port to be communicated on,
         */
        PacketHandler(uart::Port& port) : port(port), packetiser(), result(NONE), timeout_timer() {}

        /**
         * @brief   Destructs the packet-handler.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~PacketHandler() {}

        /**
         * @brief     Checks whether the expected status-packet has been received.
         * @param     id the ID of the expected status-packet, if id=254 (broadcast), then an incoming packet with any
         * ID will be accepted.
         * @retval    #NONE if not all the packets have been decoded,
         *            #SUCCESS if all the expected packets have been decoded,
         */
        template <uint16_t N>
        const Result check_sts(const nusense::NUgus::ID id) {
            // Grab a new byte if there isn't a whole packet ready.
            if (!packetiser.is_packet_ready()) {
                // Attempt to read a byte from the buffer
                uint16_t read_result = port.read();
                // If there is no byte, then return early.
                if (read_result == uart::NO_BYTE_READ) {
                    if (timeout_timer.has_timed_out()) {
                        return (result = TIMEOUT);
                    }
                    else {
                        return (result = NONE);
                    }
                }
                // We received at least one byte, so restart the timer and decode it
                timeout_timer.restart(1000);
                packetiser.decode(read_result);

                // Unless the packetiser has a whole packet, return early.
                if (!packetiser.is_packet_ready()) {
                    return (result = PARTIAL);
                }
            }

            // Stop the timer since we have a full packet.
            timeout_timer.stop();

            // If so, then parse the array as a packet and add it with the rest.
            // Parse it as both a status-packet of expected length and a short status-packet, i.e.
            // only an error.
            auto sts       = reinterpret_cast<const StatusReturnCommand<N>*>(packetiser.get_decoded_packet());
            auto short_sts = reinterpret_cast<const StatusReturnCommand<0>*>(packetiser.get_decoded_packet());

            // Perform some checks on the packet
            bool id_correct          = (sts->id == static_cast<uint8_t>(id)) || (id == nusense::NUgus::ID::BROADCAST);
            bool packet_kind_correct = (sts->instruction == Instruction::STATUS_RETURN);

            // If the CRC, the ID, and the packet-kind are correct, then return any error.
            if (id_correct && packet_kind_correct) {
                // Check the received status packet has the expected length to ensure it isn't an error packet.
                if (packetiser.get_decoded_length() == 7 + 4 + N)
                    // Check the CRC of the status-packet before anything else.
                    if (sts->crc != packetiser.get_decoded_crc())
                        result = CRC_ERROR;
                    // Before we return an error, mask out the alert field to ignore hardware errors, as we often have
                    // servo voltages above 16V.
                    else if ((static_cast<uint8_t>(sts->error) & 0x7F) == static_cast<uint8_t>(CommandError::NO_ERROR))
                        result = SUCCESS;
                    else
                        result = ERROR;
                // If the status-packet is short, then we got an error packet, so check the CRC too.
                else if (short_sts->crc != packetiser.get_decoded_crc())
                    result = CRC_ERROR;
                // Before we return an error, mask out the alert field to ignore hardware errors, as we often have servo
                // voltages above 16V.
                else if ((static_cast<uint8_t>(short_sts->error) & (uint8_t) 0x7F)
                         == static_cast<uint8_t>(CommandError::NO_ERROR))
                    result = SUCCESS;
                else
                    result = ERROR;
            }

            // If there was an error, then reset the packetiser.
            if ((result == CRC_ERROR) || (result == ERROR))
                packetiser.reset();

            return result;
        }

        /**
         * @brief   Return to a clean slate to ready the handler for a new packet.
         */
        void ready() {
            packetiser.reset();
            result = NONE;
        }

        /**
         * @brief   Begins the timeout-timer.
         * @param   timeout the timeout in microseconds, at most 65535, default is 1000
         * @note    This must be called in order to handle timeouts.
         */
        void begin(uint16_t timeout = 1000) {
            timeout_timer.begin(timeout);
        }

        /**
         * @brief   Gets the status-packet.
         * @return  a reference to the decoded packet,
         */
        const uint8_t* get_sts_packet() const {
            return packetiser.get_decoded_packet();
        }

        /**
         * @brief   Gets the status-packet's length.
         * @return  the length of the packet decoded by the packetiser,
         */
        const uint16_t get_sts_length() const {
            return packetiser.get_decoded_length();
        }

        /**
         * @brief   Gets the result.
         * @return  the result of the packet-handling,
         */
        const Result get_result() const {
            return result;
        }

    private:
        /// @brief  the reference to the port that will be communicated thereon,
        uart::Port& port;
        /// @brief  the packetiser to encode the instruction and to decode the status,
        Packetiser packetiser;
        /// @brief  the result
        Result result;
        /// @brief  the timer for the packet-timeout,
        utility::support::MicrosecondTimer timeout_timer;
    };

}  // namespace dynamixel

#endif  // DYNAMIXEL_PACKETHANDLER_HPP
