#ifndef DYNAMIXEL_CHAIN_HPP
#define DYNAMIXEL_CHAIN_HPP

#include <algorithm>

#include "../uart/Port.hpp"
#include "../utility/support/MillisecondTimer.hpp"
#include "Dynamixel.hpp"
#include "PacketHandler.hpp"

namespace dynamixel {

    /**
     * @brief A chain of dynamixel devices, connected to a port. The chain owns a packet handler, and self populates
     * itsself with connected devices by ID discovery.
     *
     */
    class Chain {
    public:
        /// @brief  Constructs the chain, without starting device discovery.
        /// @note   Discovery must be performed before the chain can be used.
        Chain(uart::Port& port, uint8_t chain_id = 0)
            : port(port), packet_handler(PacketHandler(port)), chain_id(chain_id){};

        /// @brief  Destructs the chain.
        /// @note   Could ensure the packet handler isn't waiting on anything? idk
        virtual ~Chain(){};

        /**
         * @brief Issue a broadcast ping to the underlying port and the timer for response timeout.
         * @note  discover_broadcast() must be called after this to listen for responses
         */
        void ping_broadcast() {
            // Set flag
            discovering = true;

            // Discard old devices
            devices.clear();

            // Start the packet handler for ID-by-ID discovery
            packet_handler.ready();

            // Send a broadcast ping to discover all devices on the chain
            port.write(PingCommand(static_cast<uint8_t>(nusense::NUgus::ID::BROADCAST)));

            // Start the utility timer for the maximum timeout of 3 ms * 253 devices = 759 ms
            utility_timer.begin(759);
        };

        /// @brief  Listen for a response from a broadcast ping to discover all devices on the chain
        /// @note   This is a blocking function, and will wait for the full timeout of 759 ms
        /// @return All devices found on the chain
        const std::vector<nusense::NUgus::ID>& discover_broadcast() {
            // Only do discovery if we're currently discovering
            if (!discovering) {
                return devices;
            }

            // keep listening for packets until we timeout and there are no more packets
            while (packet_handler.check_sts<3>(nusense::NUgus::ID::BROADCAST) != PacketHandler::Result::NONE
                   || !utility_timer.has_timed_out()) {
                switch (packet_handler.get_result()) {
                    // If we got a good status, extract the ID
                    // note: we need the braces for scoping the sts variable
                    case PacketHandler::Result::SUCCESS: {
                        auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                        // Add the ID to the chain
                        devices.push_back(static_cast<nusense::NUgus::ID>(sts->id));
                        if (sts->id <= static_cast<uint8_t>(nusense::NUgus::ID::MAX_SERVO_ID)) {
                            servos.push_back(static_cast<nusense::NUgus::ID>(sts->id));
                        }
                        /// TODO: Potentially use the returned data to store the device model number and firmware
                        /// version.
                    } break;
                    // If we got an error, hold onto it for logging
                    case PacketHandler::Result::ERROR: {
                        auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                        error_devices.push_back(static_cast<nusense::NUgus::ID>(sts->id));
                    } break;
                    // Skip the packet handler reset for a partial or none packet
                    case PacketHandler::Result::PARTIAL:
                    case PacketHandler::Result::NONE: continue;
                    // We should never recieve a timeout here as the timeout is for the PacketHandler::timeout_timer,
                    // which is not used for broadcast discovery, as we use the Chain::utility_timer for that.
                    case PacketHandler::Result::TIMEOUT:
                    // For completeness
                    default: break;
                }
                // Reset the packet handler for the next packet (that is likely already in the buffer)
                // if we got a full packet (including an error), or a timeout.
                packet_handler.ready();
            };

            // Unset flag
            discovering = false;

            return devices;
        };

        /// @brief  Gets all devices in the chain.
        /// @return A reference to the vector of devices in the chain.
        const std::vector<nusense::NUgus::ID>& get_devices() const {
            return devices;
        };

        /// @brief  Gets all servos in the chain.
        /// @retval A reference to the vector of servos in the chain.
        const std::vector<nusense::NUgus::ID>& get_servos() const {
            return servos;
        };

        /// @brief Gets a reference to the list of devices which errored out during discovery
        const std::vector<nusense::NUgus::ID>& get_error_devices() const {
            return error_devices;
        };

        /// @brief Whether a device is present in the chain
        /// @param id: The ID of the device to check for
        bool contains(nusense::NUgus::ID id) const {
            // Due to the response policy we know `devices` is sorted
            return std::binary_search(devices.begin(), devices.end(), id);
        };

        /// @brief  Gets a reference to the port that the chain is connected to.
        uart::Port& get_port() const {
            return port;
        };

        /// @brief  Gets a reference to the packet-handler for the chain.
        PacketHandler& get_packet_handler() {
            return packet_handler;
        };

        /// @brief  Gets the current index along the chain.
        uint8_t get_index() const {
            return index;
        };

        /// @brief  Return the ID at the current index
        const nusense::NUgus::ID& current() const {
            return devices[index];
        };

        /// @brief Move along the chain
        const nusense::NUgus::ID& next() {
            index = (index + 1) % devices.size();
            return devices[index];
        };

        /// @brief  Pass a write instruction to the port of the chain
        /// @note   This also resets the packet handler before the write.
        template <typename T>
        uint16_t write(const T& data) {
            // Prepare the packet handler for the response packet.
            packet_handler.ready();

            // Flush the port for any received bytes. Since we are starting a new request-response exchange, any bytes
            // that happen to be in the buffer are not needed.
            port.flush_rx();

            // Send the packet
            const uint16_t len = port.write(data);

            // Start the timeout timer
            packet_handler.begin();

            return len;
        };

        /// @brief Gets the total number of devices in the chain
        uint8_t size() const {
            return devices.size();
        };

        /// @brief Whether the chain is empty
        bool empty() const {
            return devices.empty();
        };

        /// @brief Get a reference to the chain utility timer
        utility::support::MillisecondTimer& get_timer() {
            return utility_timer;
        };

        /// @brief Allow the chain to be indexed like a vector
        const nusense::NUgus::ID& operator[](uint8_t i) const {
            return devices[i];
        };


    private:
        /// @brief  The list of dynamixel devices in the chain.
        std::vector<nusense::NUgus::ID> devices;
        /// @brief  The list of servos on the chain (i.e. devices with ID <= 20)
        /// @note   For forward compatibility with, e.g. FSRs
        std::vector<nusense::NUgus::ID> servos;
        /// @brief  Dynamixel devices which error out during discovery
        std::vector<nusense::NUgus::ID> error_devices;
        /// @brief  The port that the chain is connected to.
        uart::Port& port;
        /// @brief  The packet-handler for the chain.
        PacketHandler packet_handler;
        /// @brief  Where the read request is up to in this chain.
        uint8_t index = 0;
        /// @brief  A general utility timer for the chain.
        /// @note   Currently used to cooldown between write instructions, and to timeout during broadcast discovery.
        utility::support::MillisecondTimer utility_timer;
        /// @brief  Whether we're currently disocvering devices
        bool discovering = false;
        /// @brief Optional identifier for the chain to help debugging
        const uint8_t chain_id;
    };
};  // namespace dynamixel

#endif  // DYNAMIXEL_CHAIN_HPP
