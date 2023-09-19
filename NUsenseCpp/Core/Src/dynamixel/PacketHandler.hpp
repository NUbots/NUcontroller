#include "../uart/Port.hpp"
#include "Dynamixel.hpp"
#include "Packetiser.hpp"
#include "../platform/NUsense/NUgus.hpp"

#ifndef DYNAMIXEL_PACKETHANDLER_HPP
#define DYNAMIXEL_PACKETHANDLER_HPP

namespace dynamixel {

    /**
     * @brief    the handler for any generic packet
     */
    class PacketHandler {
    public:
        /// @brief  the result of whether all the status-packets have been received,
        enum Result {
            NONE = 0x00,
            SUCCESS,
            ERROR,
            CRC_ERROR,
            TIMEOUT
        };
        /**
         * @brief    Constructs the packet-handler.
         * @param    port the reference to the port to be communicated on,
         */
        PacketHandler(uart::Port& port) :
            port(port),
            packetiser(),
            result(NONE)
        {

        };
        /**
         * @brief   Destructs the packet-handler.
         * @note    nothing needs to be freed as of yet,
         */
        virtual ~PacketHandler() {};

        /**
         * @brief     Checks whether the expected status-packet has been received.
         * @retval    #NONE if not all the packets have been decoded,
         *            #SUCCESS if all the expected packets have been decoded,
         */
        template <uint16_t N>
        const Result check_sts(const platform::NUsense::NUgus::ID id) {

            // Peek to see if there is a byte on the buffer yet.
            uint16_t read_result = port.read();
            if (read_result == NO_BYTE_READ)
                return NONE;
            
            // If so, then decode it.
            packetiser.decode(read_result);

            // Unless the packetiser has a whole packet, return.
            if (!packetiser.is_packet_ready()) {
                return NONE;
            }

            // If so, then parse the array as a packet and add it with the rest.
            // Parse it as both a status-packet of expected length and a short status-packet, i.e. 
            // only an error.
            auto sts = 
                reinterpret_cast<const dynamixel::StatusReturnCommand<N>*>(
                    packetiser.get_decoded_packet()
                );

            auto short_sts =
                reinterpret_cast<const dynamixel::StatusReturnCommand<0>*>(
                    packetiser.get_decoded_packet()
                );

            // If the CRC, the ID, and the packet-kind are correct, then return any error.
            if (    (sts->id            == (uint8_t)id)
                &&  (sts->instruction   == dynamixel::STATUS_RETURN)
            ) {
                // If the status-packet is not short, then check the CRC and the error.
                if (packetiser.get_decoded_length() == 7+4+N)
                    if (sts->crc != packetiser.get_decoded_crc())
                        result = CRC_ERROR;
                    else if (sts->error == dynamixel::CommandError::NO_ERROR)
                        result = SUCCESS;
                    else
                        result = ERROR;
                else
                    if (short_sts->crc != packetiser.get_decoded_crc())
                        result = CRC_ERROR;
                    else if (short_sts->error == dynamixel::CommandError::NO_ERROR)
                        result = SUCCESS;
                    else
                        result = ERROR;
            }

            return result;
        }

        /**
         * @brief   Resets the handler.
         */
        void reset() {
            packetiser.reset();
            result = NONE;
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
        /// @brief    the reference to the port that will be communicated thereon,
        uart::Port& port;
        /// @brief    the packetiser to encode the instruction and to decode the status,
        dynamixel::Packetiser packetiser;
        /// @brief    the result
        /// @note     this is not needed yet.
        Result result;
    };

} // namespace dynamixel

#endif // DYNAMIXEL_PACKETHANDLER_HPP
