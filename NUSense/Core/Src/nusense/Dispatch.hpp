#ifndef NUSENSE_DISPATCH_HPP
#define NUSENSE_DISPATCH_HPP

#include <string>
#include <set>

namespace nusense {

    constexpr uint16_t STRING_LENGTH = 2048;

    /**
     * @brief   A dispatch of messages for the NUC, e.g. errors.
     */
    class Dispatch {
    public:
        enum Level {
            DETAIL,
            ADVICE,
            FAULT
        };

        /// @brief  Constructs the dispatch.
        Dispatch() {};

        /// @brief  Destructs the dispatch.
        virtual ~Dispatch(){};

        /**
         * @brief   Logs a string.
         * @param   str the string to be logged.
         */
        template<Level level = DETAIL>
        void log(std::string str) {
            std::string head = "DETAIL";
            if constexpr (level == FAULT) {
                head = "FAULT: ";
            }
            else if constexpr (level == ADVICE) {
                head = "ADVICE: ";
            }

            if (message.size() + str.size() + head.size() <= STRING_LENGTH) {
                std::pair<std::set<std::string>::iterator, bool> ret = logs.insert(str);
                if (ret.second == true) {
                    message.append(head.append(str).append("\n"));
                }
            }
        }

        /**
         * @brief   Clears the logs to be written again.
         */
         inline void clear() {
            logs.clear();
            message.clear();
         }

    private:
        /// @brief  The message as a string.
        std::string message{};
        /// @brief  The logs to be gathered. 
        std::set<std::string> logs{};
    };
};  // namespace nusense

#endif  // NUSENSE_DISPATCH_HPP
