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
        /// @brief  Constructs the chain.
        Chain(uart::Port& port) : port(port), index(0) {
            packet_handler = PacketHandler(port);
        };

        /// @brief  Destructs the chain.
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
                    /// TODO: Potentially use the returned data to store the device model number and firmware version.
                }
                // If we got an error, hold onto it for logging
                else if (result == PacketHandler::Result::ERROR) {
                    auto sts = reinterpret_cast<const StatusReturnCommand<3>*>(packet_handler.get_sts_packet());
                    error_devices.push_back(sts->id);
                }
            } while (result != PacketHandler::Result::TIMEOUT);
        }

        /// @brief  Gets the list of devices in the chain.
        std::vector<NUgus::ID> get_devices() {
            return devices;
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

        /// @brief  Gets the index of the chain.
        uint8_t get_index() {
            return index;
        }

    private:
        /// @brief  The list of dynamixel devices in the chain.
        std::vector<NUgus::ID> devices;
        /// @brief  Dynamixel devices which error out during discovery
        std::vector<NUgus::ID> error_devices;
        /// @brief  The port that the chain is connected to.
        uart::Port& port;
        /// @brief  The packet-handler for the chain.
        PacketHandler packet_handler;
        /// @brief  Where the read request is up to in this chain.
        uint8_t index;
    };
};  // namespace dynamixel

#endif  // DYNAMIXEL_CHAIN_HPP