#ifndef DYNAMIXEL_CHAIN_MANAGER_HPP
#define DYNAMIXEL_CHAIN_MANAGER_HPP

#include "../../dynamixel/Chain.hpp"

namespace platform::NUSense {

    /**
     * @brief A collection of dynamixel device chains which belong together. There cannot be any shared IDs within a
     * ChainManager.
     */
    template <uint8_t N>
    class ChainManager {
    public:
        /// @brief Constructs a chain manager for N chains, and starts device discovery on each chain.
        /// @param ports The ports to use for each chain.
        ChainManager(std::array<uart::Port, N>& ports) : chains() {
            // Construct a chain for each port
            for (uint8_t i = 0; i < N; i++) {
                // Create the chain
                chains[i] = dynamixel::Chain(ports[i]);
                // Start discovery in the background
                chains[i].ping_broadcast();
            };
            // Finish discovery on each chain
            // By allowing the process to happen in the background for each chain, we only have to wait for one
            // broadcast timeout.
            for (auto& chain : chains) {
                chain.discover_broadcast();
            };
        };

        /// @brief Destructor for the chain manager.
        virtual ~ChainManager(){};

        /// @brief Gets all the chains managed by the ChainManager
        std::array<dynamixel::Chain, N>& get_chains() {
            return chains;
        };

        /// @todo implement duplicate ID check

        /// @todo implement missing ID check

    private:
        std::array<dynamixel::Chain, N> chains{};
    };
};  // namespace platform::NUSense

#endif  // DYNAMIXEL_CHAIN_MANAGER_HPP
