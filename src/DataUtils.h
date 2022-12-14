#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#include <iterator>
#include <string>

#define DU_ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))

namespace DataUtils {
	template<typename RandomIt,typename T>
	T* binarySearchExlusive(RandomIt first, RandomIt last, const T& value, int (*compare)(const T& a, const T& b)) {
		RandomIt from = first;
		RandomIt to = last;
		while (from != to) {
			RandomIt mid = from + (to-from) / 2;

			int cmp = compare(value, *mid);

			if (cmp == 0) {
				return mid;
			}
			else if (cmp < 0) {
				if (mid == to)
					goto fail;
				to = mid;
			}
			else {
				if (mid == from)
					goto fail;
				from = mid;
			}
		}

		if (compare(value, *from) == 0)
			return from;

	fail:
		return nullptr;
	}

	// find value, if not found return where to insert it; compare needs to be a funktion like object with (const T& a, size_t ind_of_b) -> int
	template<typename T,typename CMP>
	size_t binarySearchInclusive(size_t len, const T& value, CMP compare) {
		size_t from = 0;
		size_t to = len-1;
		while (from < to) {
			size_t mid = from + (to-from) / 2;

			int cmp = compare(value, mid);

			if (cmp == 0) {
				return mid;
			}
			else if (cmp < 0) {
				if (mid == to) {
					return from;
				}
				to = mid;
			}
			else {
				if (mid == from) {
					return from;
				}
				from = mid;
			}
		}
		
		return from;
	}

	template<typename RandomIt, typename T>
	size_t findVal(RandomIt first, RandomIt last, const T& value) {
		size_t len = std::distance(first, last);
		for(size_t i = 0; i<len; i++) {
			if(*(first + i) == value)
				return i;
		}
		return -1;
	}

	uint64_t simpleHash(uint64_t v);

	namespace EditMemory {
		enum {
			EditBase_2 = 0,
			EditBase_10,
			EditBase_16,
			EditBase_COUNT
		};
		enum {
			EditType_8bit = 0,
			EditType_16bit,
			EditType_32bit,
			EditType_64bit,
			EditType_float,
			EditType_double,
			EditType_string,
			EditType_bytestream,
			EditType_COUNT
		};
		enum {
			EditEndian_Little = 0,
			EditEndian_Big,
			EditEndian_COUNT
		};

		typedef void (*SetValueCallB)(size_t addr, uint8_t val, void* userData);

		uint64_t readValue(const uint8_t* data, size_t dataLen, uint8_t editType, uint8_t editEndian=EditEndian_Little);
		bool writeValue(size_t addr, uint64_t val, const std::string& editStr, SetValueCallB setValueCallB, void* setValueUserData, size_t dataLen, bool editStringTerm, bool editReversed, uint8_t editType, uint8_t editEndian=EditEndian_Little);
	}
}

#endif