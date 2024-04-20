#ifndef DYNAMIXEL_CHAIN_HPP
#define DYNAMIXEL_CHAIN_HPP


#include "../uart/Port.hpp"
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
        /// @brief  Constructs the chain, and starts device discovery.
        Chain(uart::Port& port) : port(port), packet_handler(PacketHandler(port)), index(0), utility_timer() {
            // start chain discovery
            discover();
            // discover_broadcast();
        };

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
            for (NUgus::ID id = 1; id < NUgus::ID::BROADCAST; id++) {
                PacketHandler::Result result = PacketHandler::Result::NONE;
                // Repeat ping unless we have a timeout or success in case a servo reply gets corrupted
                do {
                    // Send a ping to the target device
                    port.write(PingCommand(id));
                    // Wait for the status to be returned
                    while (result == PacketHandler::Result::NONE)
                        packet_handler.check_sts<3>(id);
                } while (result != PacketHandler::Result::SUCCESS && result != PacketHandler::Result::TIMEOUT);
                // If the status was received, add the device to the chain
                if (result == PacketHandler::Result::SUCCESS) {
                    devices.push_back(id);
                    if (sts->id <= 20) {
                        servos.push_back(sts->id);
                    }
                    /// TODO: Potentially use the returned data to store the device model number and firmware version.
                }
            }
        }

        /**
         * @brief discover which Dynamixel devices are connected to the chain using a broadcast ping
         * @note  I actually think the packet handler can handle broadcast pings, secretly.
         * The only issue would be processing fast enough to not fill the UART buffer, but, the buffer is 2048
         * bytes, and the status packet of a ping is 10 bytes (I think?) So, although we can't discover the
         * theoretical maximum of 254 devices on one chain, we can definitely discover a reasonable number of
         * devices on a chain.
         *
         * This also means being able to handle the long timeout (253 devices * 3 ms/device = 759 ms) of a broadcast
         * ping, which is longer than the maximum timeout of 65.535 ms.
         *
         * Here is the problem, though. We don't know how many devices are on the chain, or which devices, so we are
         * expecting a timeout (unless ID 1 is on this chain), but we don't know for how long. The servos will wait
         * 3 ms for each ID before them, prior to responding to a broadcast ping. In standard operation (without
         * FSRs), we can expect to wait up to 19*3=57 ms for the initial response on the head chain. Then for any
         * subsequent responses, because chains are generally non-consecutive, we also don't know how long to wait
         * for. In theory (again, without FSRs, although I want to make this robust to any dynamixel devices up to
         * ID 253), the maximum inter servo wait time in 20 servo operation is ~20*3=60ms, assuming a chain has only
         * ID 1 and 20. This is possible with the current timeout.
         */
        void discover_broadcast() {
            // Discard old devices
            devices.clear();

            // Start the packet handler for ID-by-ID discovery
            packet_handler.reset();
            PacketHandler::Result result = PacketHandler::Result::NONE;

            // Send a broadcast ping to discover all devices on the chain
            port.write(PingCommand(uint8_t(NUgus::ID::BROADCAST)));

            // now keep listening for packets until we timeout (at which point the chain is done)
            do {
                // Start the timeout for current maximum of 60ms (expected) + 500us (timeout)
                packet_handler.begin(60500);

                // Wait for the status to be returned
                while (result == PacketHandler::Result::NONE)
                    packet_handler.check_sts<3>(NUgus::ID::BROADCAST);

                // If we got a good status, extract the ID
                if (result == PacketHandler::Result::SUCCESS) {
                    auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                    // Add the ID to the chain
                    devices.push_back(sts->id);
                    if (sts->id <= 20) {
                        servos.push_back(sts->id);
                    }
                    /// TODO: Potentially use the returned data to store the device model number and firmware version.
                }
                // If we got an error, hold onto it for logging
                else if (result == PacketHandler::Result::ERROR) {
                    auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                    error_devices.push_back(sts->id);
                }
            } while (result != PacketHandler::Result::TIMEOUT);
        }

        /// @brief  Gets all devices in the chain.
        /// @retval A reference to the vector of devices in the chain.
        std::vector<NUgus::ID>& get_devices() {
            return devices;
        }

        /// @brief  Gets all servos in the chain.
        /// @retval A reference to the vector of servos in the chain.
        std::vector<NUgus::ID>& get_servos() {
            return servos;
        }

        /// @brief Gets the list of devices which errored out during discovery
        std::vector<NUgus::ID>& get_error_devices() {
            return error_devices;
        }

        /// @brief Whether a device is present in the chain
        bool contains(NUgus::ID id) {
            return std::find(devices.begin(), devices.end(), id) != devices.end();
        }

        /// @brief  Gets the port that the chain is connected to.
        uart::Port& get_port() {
            return port;
        }

        /// @brief  Gets the packet-handler for the chain.
        PacketHandler get_packet_handler() {
            return packet_handler;
        }

        /// @brief  Gets the current index along the chain.
        uint8_t get_index() {
            return index;
        }

        /// @brief  Return the ID at the current index
        NUgus::ID current() {
            return devices[index];
        }

        /// @brief Move along the chain
        NUgus::ID next() {
            index = (index + 1) % devices.size();
            return devices[index];
        }

        /// @brief  Pass a write instruction to the port of the chain
        /// @note   This also resets the packet handler before the write.
        template <typename T>
        const uint16_t write(const T& data) {
            // Reset the packet-handler before a new interaction has begun.
            packet_handler.reset();
            pakcet_handler.begin();

            return port.write(data);
        }

        /// @brief Gets the total number of devices in the chain
        uint8_t size() {
            return devices.size();
        }

        /// @brief Whether the chain is empty
        bool empty() {
            return devices.empty();
        }

        /// @brief Get the chain utility timer
        utility::support::MicrosecondTimer& get_timer() {
            return utility_timer;
        }

        /// @brief Allow the chain to be indexed like a vector
        NUgus::ID operator[](uint8_t i) {
            return devices[i];
        }


    private:
        /// @brief  The list of dynamixel devices in the chain.
        std::vector<NUgus::ID> devices;
        /// @brief  The list of servos on the chain (i.e. devices with ID <= 20)
        /// @note   For forward compatibility with, e.g. FSRs
        std::vector<NUgus::ID> servos;
        /// @brief  Dynamixel devices which error out during discovery
        std::vector<NUgus::ID> error_devices;
        /// @brief  The port that the chain is connected to.
        uart::Port& port;
        /// @brief  The packet-handler for the chain.
        PacketHandler packet_handler;
        /// @brief  Where the read request is up to in this chain.
        uint8_t index;
        /// @brief  To allow for cooldowns between write instructions
        utility::support::MicrosecondTimer utility_timer;
    };
};  // namespace dynamixel

#endif  // DYNAMIXEL_CHAIN_HPP