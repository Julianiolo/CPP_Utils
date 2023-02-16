#ifndef __BITUTILS_H__
#define __BITUTILS_H__

#include <cstdint>

namespace BitUtils {
	template<typename T>
	constexpr inline T setBitTo(T v, bool bitVal, uint8_t bit) {
		return v ^ ((-(T)bitVal ^ v) & (1 << bit));
	}

	constexpr inline uint8_t getLBS(uint64_t x) {
		if (x == 0) return 64;

		uint8_t ret = 0;
		while ((x & 1) == 0) {
			x >>= 1;
			ret++;
		}
		return ret;
	}
	constexpr inline uint8_t getHBS(uint64_t x) {
		uint8_t ret = 0;
		while (x >>= 1) ret++;
		return ret;
	}
}

#endif
