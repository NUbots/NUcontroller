#ifndef NUSENSE_DISPATCHER_HPP
#define NUSENSE_DISPATCHER_HPP

#include <queue>
#include <string>

namespace nusense {

    constexpr uint16_t MAX_DISPATCH_LENGTH = 128;
    constexpr uint16_t MAX_TOTAL_LENGTH    = 4096;

    /**
     * @brief   A handler of dispatches for the NUC, e.g. errors.
     * @note    This should be used fleetingly for unexpected faults and should not be used for
     *          general debugging.
     */
    class Dispatcher {
    public:
        /// @brief  Constructs the dispatch-handler.
        Dispatcher(){};

        /// @brief  Destructs the dispatch-handler.
        virtual ~Dispatcher(){};

        /**
         * @brief   Writes a string to the dispatch.
         * @param   str The string to be dispatched.
         */
        void write(const std::string& str) {
            // If dispatch's length is still within the maximum, then add the new string.
            if (dispatch.size() + str.size() + 1 <= MAX_DISPATCH_LENGTH) {
                dispatch.append("\n" + str);
            }
            // Else, if total length of the queue is still within the maximum, then add the new string.
            else if (total_length + str.size() + 1 <= MAX_TOTAL_LENGTH) {
                queued_strings.push("\n" + str);
                total_length += str.size() + 1;
            }
        }

        /**
         * @brief   Gets the latest dispatch as a string.
         * @return  The dispatch as a string.
         */
        inline const std::string& get_dispatch() const {
            return dispatch;
        }

        /**
         * @brief   Clears the message and writes the dispatches to be written again.
         */
        inline void update() {
            // Clear the dispatch.
            dispatch.clear();
            // As long as the dispatch's length is still within the maximum, unload from the queue.
            while ((dispatch.size() <= MAX_DISPATCH_LENGTH) && (queued_strings.size() > 0)) {
                total_length -= queued_strings.front().size();
                dispatch += queued_strings.front();
                queued_strings.pop();
            }
        }

    private:
        /// @brief  The dispatch as a string.
        std::string dispatch{};
        /// @brief  The strings to be gathered before writing.
        std::queue<std::string> queued_strings{};
        /// @brief  The total length of all the dispatches queued.
        /// @note   This is only to limit the amount of memory used.
        uint16_t total_length = 0;
    };
};  // namespace nusense

#endif  // NUSENSE_DISPATCHER_HPP
