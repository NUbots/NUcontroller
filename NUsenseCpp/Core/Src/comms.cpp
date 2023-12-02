/*
 * comms.cpp
 *
 *  Created on: 3 Dec. 2023
 *      Author: Johanne Montano
 */

#include "comms.hpp"

bool receive() {
	if (rx_flag) {
		// Reset rx_flag - this flag is turned on by the USB receive call back and turned off here
		rx_flag = 0;

		// Check if we have a header and if we do extract our lengths and pb bytes
		// Must apply modulo here to make sure that we're always wrapping around
		bool has_magic_one   = rx_buf[rx_buf_idx] == (char)0xE2;
		bool has_magic_two   = rx_buf[(rx_buf_idx + 1) % RX_BUF_SIZE] == (char)0x98;
		bool has_magic_three = rx_buf[(rx_buf_idx + 2) % RX_BUF_SIZE] == (char)0xA2;

		bool header_detected = has_magic_one && has_magic_two && has_magic_three;

		if (header_detected) {
			uint16_t pb_length = static_cast<uint16_t>(rx_buf[(rx_buf_idx + 3) % RX_BUF_SIZE] << 8) | static_cast<uint16_t>(rx_buf[(rx_buf_idx + 4) % RX_BUF_SIZE]);
			memcpy(&pb_packets[0], &rx_buf[(rx_buf_idx + 5) % RX_BUF_SIZE], pb_length);
			packet_fin = 1;
		}

		// Update index accessor after receiving a packet, making sure to wrap around in case it exceeds the buffer's length
		rx_buf_idx = rx_buf_len;
	}

	if (packet_fin) {
		// Reset the flag then decoding time
		packet_fin = 0;

		// Decoding time
		pb_istream_t input_stream = pb_istream_from_buffer(reinterpret_cast<const pb_byte_t*>(&pb_packets[0]), rx_buf_len - rx_buf_idx);
		message_actuation_ServoTargets servo_targets_pb = message_actuation_ServoTargets_init_zero;

		// This will be false if nanopb thinks that our data is not a protobuf packet
		bool decoded = pb_decode(&input_stream, message_actuation_ServoTargets_fields, &servo_targets_pb);

		// After decoding, empty the pb_packets vector to avoid corruption
		memset(&pb_packets[0], 0, RX_BUF_SIZE);

		// TODO Johanne Montano Extract id, position, gain, torque from servo_targets_pb then sort them by id inside servo_commands
		for (int i = 0; i < servo_targets_pb._message_actuation_ServoTargets; ++i) {

		}

		return decoded;
	}

	return false;
}


