#ifndef DYNAMIXEL_CHAIN_MANAGER_HPP
#define DYNAMIXEL_CHAIN_MANAGER_HPP

#include "../dynamixel/Chain.hpp"

namespace nusense {

    /**
     * @brief A collection of dynamixel device chains which belong together. There cannot be any shared IDs within a
     * ChainManager.
     */
    template <uint8_t N>
    class ChainManager {
    public:
        /// @brief Constructs a chain manager for N chains, and starts device discovery on each chain.
        /// @param chains: An array of chains to manage.
        ChainManager(std::array<dynamixel::Chain, N> chains) : chains(chains) {
            // Initialise each chain
            for (auto& chain : chains) {
                // Begin the receiving. This should be done only once if we are using the DMA as a buffer.
                chain.get_port().begin_rx();
                chain.get_port().flush_rx();
                // Start the broadcast ping in the background before we listen on any chain so we only have to wait for
                // one broadcast timeout (759ms) total.
                chain.ping_broadcast();
            }

            // Finish discovery on each chain
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
};  // namespace nusense

#endif  // DYNAMIXEL_CHAIN_MANAGER_HPP
