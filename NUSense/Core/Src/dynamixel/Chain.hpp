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
        Chain(uart::Port& port) : port(port), packet_handler(PacketHandler(port)), index(0){};

        /// @brief  Destructs the chain.
        /// @note   Could ensure the packet handler isn't waiting on anything? idk
        virtual ~Chain(){};

        /// @brief  Discover which Dynamixel devices are connected to the chain
        void discover() {
            // Discard old devices
            devices.clear();

            // Start the packet handler for ID-by-ID discovery
            packet_handler.reset();
            packet_handler.begin(500);

            // For now, the packet handler can't handle broadcast pings, so we have to ping each device individually
            for (uint8_t id = static_cast<uint8_t>(platform::NUSense::NUgus::ID::MIN_ID);
                 id < static_cast<uint8_t>(platform::NUSense::NUgus::ID::MAX_ID);
                 id++) {
                PacketHandler::Result result = PacketHandler::Result::NONE;
                // Repeat ping unless we have a timeout or success in case a servo reply gets corrupted
                do {
                    // Send a ping to the target device
                    write(PingCommand(id));
                    // Wait for the status to be returned
                    while (result == PacketHandler::Result::NONE)
                        packet_handler.check_sts<3>(static_cast<platform::NUSense::NUgus::ID>(id));
                } while (result != PacketHandler::Result::SUCCESS && result != PacketHandler::Result::TIMEOUT);
                // If the status was received, add the device to the chain
                if (result == PacketHandler::Result::SUCCESS) {
                    devices.push_back(static_cast<platform::NUSense::NUgus::ID>(id));
                    if (id <= 20) {
                        servos.push_back(static_cast<platform::NUSense::NUgus::ID>(id));
                    }
                    /// TODO: Potentially use the returned data to store the device model number and firmware version.
                }
            }
        };

        /**
         * @brief Issue a broadcast ping to the underlying port and the timer for response timeout.
         * @note  discover_broadcast() must be called after this to listen for responses
         */
        void ping_broadcast() {
            // Discard old devices
            devices.clear();

            // Start the packet handler for ID-by-ID discovery
            packet_handler.reset();

            // Send a broadcast ping to discover all devices on the chain
            port.write(PingCommand(static_cast<uint8_t>(platform::NUSense::NUgus::ID::BROADCAST)));

            // Start the utility timer for the maximum timeout of 3 ms * 253 devices = 759 ms
            utility_timer.begin(759);
        };

        /// @brief  Listen for a response from a broadcast ping to discover all devices on the chain
        /// @note   This is a blocking function, and will wait for the full timeout of 759 ms
        /// @retval All devices found on the chain
        const std::vector<platform::NUSense::NUgus::ID>& discover_broadcast() {
            // Wait for the first status to be returned
            while (packet_handler.check_sts<3>(platform::NUSense::NUgus::ID::BROADCAST) == PacketHandler::Result::NONE)
                ;

            // now keep listening for packets until we timeout and there are no more packets
            while (!utility_timer.has_timed_out() || packet_handler.get_result() != PacketHandler::Result::NONE) {
                // If we got a good status, extract the ID
                if (packet_handler.get_result() == PacketHandler::Result::SUCCESS) {
                    auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                    // Add the ID to the chain
                    devices.push_back(static_cast<platform::NUSense::NUgus::ID>(sts->id));
                    if (sts->id <= 20) {
                        servos.push_back(static_cast<platform::NUSense::NUgus::ID>(sts->id));
                    }
                    /// TODO: Potentially use the returned data to store the device model number and firmware version.
                }
                // If we got an error, hold onto it for logging
                else if (packet_handler.get_result() == PacketHandler::Result::ERROR) {
                    auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                    error_devices.push_back(static_cast<platform::NUSense::NUgus::ID>(sts->id));
                }
                // Attempt to get the next status packet
                packet_handler.check_sts<3>(platform::NUSense::NUgus::ID::BROADCAST);
            };

            return devices;
        };

        /// @brief  Gets all devices in the chain.
        /// @retval A reference to the vector of devices in the chain.
        const std::vector<platform::NUSense::NUgus::ID>& get_devices() const {
            return devices;
        };

        /// @brief  Gets all servos in the chain.
        /// @retval A reference to the vector of servos in the chain.
        const std::vector<platform::NUSense::NUgus::ID>& get_servos() const {
            return servos;
        };

        /// @brief Gets a reference to the list of devices which errored out during discovery
        const std::vector<platform::NUSense::NUgus::ID>& get_error_devices() const {
            return error_devices;
        };

        /// @brief Whether a device is present in the chain
        bool contains(platform::NUSense::NUgus::ID id) const {
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
        const uint8_t get_index() const {
            return index;
        };

        /// @brief  Return the ID at the current index
        const platform::NUSense::NUgus::ID& current() const {
            return devices[index];
        };

        /// @brief Move along the chain
        const platform::NUSense::NUgus::ID& next() {
            index = (index + 1) % devices.size();
            return devices[index];
        };

        /// @brief  Pass a write instruction to the port of the chain
        /// @note   This also resets the packet handler before the write.
        template <typename T>
        const uint16_t write(const T& data) {
            // Reset the packet-handler before a new interaction has begun.
            packet_handler.reset();
            packet_handler.begin();

            return port.write(data);
        };

        /// @brief Gets the total number of devices in the chain
        const uint8_t size() const {
            return devices.size();
        };

        /// @brief Whether the chain is empty
        const bool empty() const {
            return devices.empty();
        };

        /// @brief Get a reference to the chain utility timer
        utility::support::MillisecondTimer& get_timer() {
            return utility_timer;
        };

        /// @brief Allow the chain to be indexed like a vector
        const platform::NUSense::NUgus::ID& operator[](uint8_t i) const {
            return devices[i];
        };


    private:
        /// @brief  The list of dynamixel devices in the chain.
        std::vector<platform::NUSense::NUgus::ID> devices;
        /// @brief  The list of servos on the chain (i.e. devices with ID <= 20)
        /// @note   For forward compatibility with, e.g. FSRs
        std::vector<platform::NUSense::NUgus::ID> servos;
        /// @brief  Dynamixel devices which error out during discovery
        std::vector<platform::NUSense::NUgus::ID> error_devices;
        /// @brief  The port that the chain is connected to.
        uart::Port& port;
        /// @brief  The packet-handler for the chain.
        PacketHandler packet_handler;
        /// @brief  Where the read request is up to in this chain.
        uint8_t index;
        /// @brief  A general utility timer for the chain.
        /// @note   Currently used to cooldown between write instructions, and to timeout during broadcast discovery.
        utility::support::MillisecondTimer utility_timer;
    };
};  // namespace dynamixel

#endif  // DYNAMIXEL_CHAIN_HPP