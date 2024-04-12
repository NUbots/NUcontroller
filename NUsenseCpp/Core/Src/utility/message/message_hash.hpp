/*
 * xxhash64.hpp
 *
 *  Created on: Apr 11, 2024
 *      Author: TrentHouliston
 */

#include <string>

#ifndef SRC_UTILITY_MESSAGE_XXHASH64_HPP_
#define SRC_UTILITY_MESSAGE_XXHASH64_HPP_

namespace utility::message {
	uint64_t xxhash64(const void* input_v, const size_t& length, const uint64_t& seed) {

		static constexpr uint64_t PRIME1 = 11400714785074694791ULL;
		static constexpr uint64_t PRIME2 = 14029467366897019727ULL;
		static constexpr uint64_t PRIME3 = 1609587929392839161ULL;
		static constexpr uint64_t PRIME4 = 9650029242287828579ULL;
		static constexpr uint64_t PRIME5 = 2870177450012600261ULL;

		// Cast the input pointer to a character pointer.
		const auto* input = static_cast<const char*>(input_v);
		/// The hash value being calculated.
		uint64_t h{};
		/// A pointer to the current position in the input buffer.
		const char* p = input;

		// Process 32 byte chunks if we can
		if (length >= 32) {
			uint64_t v1 = seed + PRIME1 + PRIME2;
			uint64_t v2 = seed + PRIME2;
			uint64_t v3 = seed;
			uint64_t v4 = seed - PRIME1;

			for (; p < input + (length & ~0x1F); p += 32) {
				v1 = rotl(v1 + read64(p) * PRIME2, 31) * PRIME1;
				v2 = rotl(v2 + read64(p + 8) * PRIME2, 31) * PRIME1;
				v3 = rotl(v3 + read64(p + 16) * PRIME2, 31) * PRIME1;
				v4 = rotl(v4 + read64(p + 24) * PRIME2, 31) * PRIME1;
			}
			// Mix
			h = rotl(v1, 1) + rotl(v2, 7) + rotl(v3, 12) + rotl(v4, 18);
			h = (h ^ rotl(v1 * PRIME2, 31) * PRIME1) * PRIME1 + PRIME4;
			h = (h ^ rotl(v2 * PRIME2, 31) * PRIME1) * PRIME1 + PRIME4;
			h = (h ^ rotl(v3 * PRIME2, 31) * PRIME1) * PRIME1 + PRIME4;
			h = (h ^ rotl(v4 * PRIME2, 31) * PRIME1) * PRIME1 + PRIME4;
		}
		else {
			h = seed + PRIME5;
		}

		h += length;

		// Process in 8-byte chunks.
		for (; p < input + (length & ~0x7); p += 8) {
			h = rotl(h ^ (rotl(read64(p) * PRIME2, 31) * PRIME1), 27) * PRIME1 + PRIME4;
		}

		// Process in 4-byte chunks.
		for (; p < input + (length & ~0x3); p += 4) {
			h = rotl(h ^ (read32(p) * PRIME1), 23) * PRIME2 + PRIME3;
		}

		// Process the remainder.
		for (; p < input + length; p++) {
			h = rotl(h ^ (uint8_t(*p) * PRIME5), 11) * PRIME1;
		}

		// Avalanche
		h = (h ^ (h >> 33)) * PRIME2;
		h = (h ^ (h >> 29)) * PRIME3;
		h = h ^ (h >> 32);

		return h;
	}

	static constexpr uint64t seed = 0x4e55436c;
	static const std::string NUSENSE_TYPENAME = "message.platform.NUSense";
	const uint64_t NUSENSE_HASH = xxhash64(NUSENSE_TYPENAME, NUSENSE_TYPENAME.size(), seed);
}


#endif /* SRC_UTILITY_MESSAGE_XXHASH64_HPP_*/
