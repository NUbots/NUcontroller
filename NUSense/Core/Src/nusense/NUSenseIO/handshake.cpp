#include "../NUSenseIO.hpp"
#include "usbd_cdc_if.h"

namespace nusense {
    bool NUSenseIO::handshake_received() {
        bool hs_rx = false;
        if (nuc.handle_incoming(true)) {
            if (nuc.get_curr_msg_hash() == utility::message::HANDSHAKE_HASH) {
                // Send reply to NUSense
                strcpy(handshake_msg.msg, "Hello NUC!");

                // Once everything else is filled we send it to the NUC. Just overwrite the bytes within encoding_payload
                // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
                // If one wishes to add messages to the protobuf message, one must first calculate the maximum bytes
                // within that message and then add enough bytes to make sure that nanopb doesn't cry about the output stream
                // being too small If the MAX_ENCODE_SIZE is inadequately defined, one can get a corrupted message and nanopb
                // errors.
                pb_ostream_t output_buffer = pb_ostream_from_buffer(&encoding_payload[0], MAX_ENCODE_SIZE);

                // TODO (NUSense people) Handle encoding errors properly using this member somehow
                nanopb_encoding_err = pb_encode(&output_buffer, message_platform_NUSenseHandshake_fields, &handshake_msg) ? false : true;
                if (nanopb_encoding_err) {
                    return false;
                }

                // Happiness, the encoding succeeded
                std::vector<uint8_t> nbs({0xE2, 0x98, 0xA2});

                // TODO (JohanneMontano) Implement timestamp field correctly, std::chrono is behaving weird and it needs to be
                // investigated
                uint64_t ts_u = 0;
                uint32_t size = uint32_t(output_buffer.bytes_written + sizeof(utility::message::HANDSHAKE_HASH) + sizeof(ts_u));

                // Encode size to uint8_t's
                for (size_t i = 0; i < sizeof(size); ++i) {
                    nbs.push_back(uint8_t((size >> (i * 8)) & 0xFF));
                }

                // Encode timestamp
                for (size_t i = 0; i < sizeof(ts_u); ++i) {
                    nbs.push_back(uint8_t((ts_u >> (i * 8)) & 0xFF));
                }

                // Encode handshake hash
                for (size_t i = 0; i < sizeof(utility::message::HANDSHAKE_HASH); ++i) {
                    nbs.push_back(uint8_t((utility::message::HANDSHAKE_HASH >> (i * 8)) & 0xFF));
                }

                // Add the protobuf bytes into the nbs vector
                nbs.insert(nbs.end(), std::begin(encoding_payload), std::begin(encoding_payload) + output_buffer.bytes_written);

                // Attempt to transmit data then handle it accordingly if it fails
                if (CDC_Transmit_HS(nbs.data(), nbs.size()) != USBD_OK) {
                    // Going into this block means that the usb failed to transmit our data
                    usb_tx_err = true;
                }
                hs_rx = true;
            }
        }

        return hs_rx;
    }
}  // namespace nusense
