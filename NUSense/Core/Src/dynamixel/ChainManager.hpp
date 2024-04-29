#ifndef DYNAMIXEL_CHAIN_MANAGER_HPP
#define DYNAMIXEL_CHAIN_MANAGER_HPP

#include "Chain.hpp"

namespace dynamixel {

    /**
     * @brief A collection of dynamixel device chains which belong together. There cannot be any shared IDs within a
     * ChainManager.
     */
    template <size_t N>
    class ChainManager {
    public:
        /// @brief Constructs a chain manager for N chains, and starts device discovery on each chain.
        /// @param ports The ports to use for each chain.
        ChainManager(std::array<uart::Port, N>& ports) : {
            // Construct a chain for each port
            for (auto& port : ports) {
                // Create the chain
                chains.push_back(Chain(port));
                // Start discovery in the background
                chains.end().ping_broadcast();
            };
            // Finish discovery on each chain
            // By allowing the process to happen in the background for each chain, we only have to wait for one
            // broadcast timeout.
            for (auto& chain : chains) {
                chain.discover_broadcast();
            };
        };

    private:
        std::array<dynamixel::Chain, N> chains;
    };
};  // namespace dynamixel

#endif  // DYNAMIXEL_CHAIN_MANAGER_HPP
