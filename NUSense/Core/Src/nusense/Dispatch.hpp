#ifndef NUSENSE_DISPATCHHANDLER_HPP
#define NUSENSE_DISPATCHHANDLER_HPP

#include <string>
#include <set>

namespace nusense {

    constexpr uint16_t MAX_DISPATCH_LENGTH = 1024;
    constexpr uint16_t MAX_TOTAL_LENGTH = 4096;

    /**
     * @brief   A handler of dispatches for the NUC, e.g. errors.
     * @note    This should be used fleetingly for unexpected faults and should not be used for 
     *          general debugging.
     */
    class DispatchHandler {
    public:
        enum Level {
            DETAIL,
            ADVICE,
            FAULT
        };

        /// @brief  Constructs the dispatch-handler.
        DispatchHandler() {};

        /// @brief  Destructs the dispatch-handler.
        virtual ~DispatchHandler(){};

        /**
         * @brief   Writes a string to the dispatch.
         * @param   str The string to be dispatched.
         */
        void write(const std::string& str) {
            if (dispatch.size() + str.size() + 1 <= MAX_DISPATCH_LENGTH) {
                dispatch.append(str + "\n");
            }
            else if (total_length + str.size() + 1 <= MAX_TOTAL_LENGTH) {
                queued_dispatches.push(str + "\n");
                total_length += str.size() + 1;
            }
        }

        /**
         * @brief   Gets the latest dispatch as a string.
         * @return  The dispatch as a string.
         */
        inline const std::string& dispatch() const {
            return dispatch;
        }

        /**
         * @brief   Clears the message and writes the dispatches to be written again.
         */
         inline void update() {
            dispatch.clear();
            while ((dispatch.size() <= MAX_DISPATCH_LENGTH) && (queued_dispatches.size() > 0)) {
                total_length -= queued_dispatches.front().size();
                dispatch += queued_dispatches.pop();
            }
         }

    private:
        /// @brief  The message as a string.
        std::string dispatch{};
        /// @brief  The dispatches to be gathered before writing. 
        std::queue<std::string> queued_dispatches{};
        /// @brief  The total length of all the dispatches queued.
        /// @note   This is only to limit the amount of memory used.
        uint16_t total_length = 0;
    };
};  // namespace nusense

#endif  // NUSENSE_DISPATCHHANDLER_HPP
