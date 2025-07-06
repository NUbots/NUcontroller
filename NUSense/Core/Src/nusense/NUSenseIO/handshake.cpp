#include "../NUSenseIO.hpp"
#include "usbd_cdc_if.h"

namespace nusense {
    bool NUSenseIO::handshake_received() {
        bool hs_rx = false;
        if (nuc.handle_incoming(true)) {
            if (nuc.get_curr_msg_hash() == utility::message::HANDSHAKE_HASH) {

                // If the handshake type is not INIT (0) then the NUC is trying to reconnect when we are still waiting
                // for an init handshake This means that the red button was pressed NUSense was reset.
                if (nuc.get_handshake_msg()->type != false) {
                    return false;
                }

                // Send reply to NUSense
                strcpy(handshake_msg.msg, "Hello NUC!");
                hs_rx = encode_and_transmit_nbs(handshake_msg,
                                                utility::message::HANDSHAKE_HASH,
                                                message_platform_NUSenseHandshake_fields);
            }
        }
        return hs_rx;
    }
}  // namespace nusense
