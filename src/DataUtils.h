#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#include <iterator>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>
#include "ringBuffer.h"

#define DU_ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))

namespace DataUtils {
	template<typename T,typename CMP>
	size_t binarySearchExlusive(size_t len, const T& value, CMP compare) {
		if (len == 0)
			return -1;

		size_t from = 0;
		size_t to = len-1;
		while (from != to) {
			size_t mid = from + (to-from) / 2;

			int cmp = compare(value, mid);

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

		if (compare(value, from) == 0)
			return from;

	fail:
		return -1;
	}

	// find value, if not found return where to insert it; compare needs to be a funktion like object with (const T& a, size_t ind_of_b) -> int
	template<typename T,typename CMP>
	constexpr inline size_t binarySearchInclusive(size_t len, const T& value, CMP compare) {
		if (len == 0)
			return -1;

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

	inline constexpr size_t approxSizeOf(int8_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint8_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int16_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint16_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int32_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint32_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int64_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint64_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(bool v) { return sizeof(v); };

	template<typename T>
	constexpr size_t approxSizeOf(T* v) {
		return sizeof(T*);
	}
	inline size_t approxSizeOf(const std::string& v) {
		return sizeof(std::string) + v.capacity() * sizeof(char);
	}
	inline size_t approxSizeOf(const std::wstring& v) {
		return sizeof(std::wstring) + v.capacity() * sizeof(wchar_t);
	}

	template<typename T, size_t N>
	size_t approxSizeOf(const std::array<T, N>& v);
	template<typename Alloc>
	size_t approxSizeOf(const std::vector<bool, Alloc>& v);
	template<typename T, typename Alloc>
	size_t approxSizeOf(const std::vector<T, Alloc>& v);
	template<typename T, typename Alloc, typename SIZEFNC>
	size_t approxSizeOf(const std::vector<T, Alloc>& v, SIZEFNC sf);
	template<typename T, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::set<T, Traits, Alloc>& v);
	template<typename T1, typename T2>
	size_t approxSizeOf(const std::pair<T1, T2>& v);
	template<typename TK, typename TV, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::map<TK, TV, Traits, Alloc>& v);
	template<typename T>
	size_t approxSizeOf(const RingBuffer<T>& v);

	template<typename Alloc>
	size_t approxSizeOf(const std::vector<bool,Alloc>& v) {
		size_t sum = 0;
		sum += sizeof(std::vector<bool, Alloc>);
		sum += v.capacity()/8 + v.capacity() % 8 != 0;

		return sum;
	}
	template<typename T, typename Alloc>
	size_t approxSizeOf(const std::vector<T,Alloc>& v) {
		size_t sum = 0;
		sum += sizeof(std::vector<T, Alloc>);
		for (const auto& e : v) sum += approxSizeOf(e);

		sum += (v.capacity() - v.size()) * sizeof(T);

		return sum;
	}
	template<typename T, typename Alloc, typename SIZEFNC>
	size_t approxSizeOf(const std::vector<T,Alloc>& v, SIZEFNC sf) {
		size_t sum = 0;
		sum += sizeof(std::vector<T, Alloc>);
		for (const auto& e : v) sum += sf(e);

		sum += (v.capacity() - v.size()) * sizeof(T);

		return sum;
	}
	template<typename T, size_t N>
	size_t approxSizeOf(const std::array<T,N>& v) {
		size_t sum = 0;
		for (const auto& e : v) sum += approxSizeOf(e);
		return sum;
	}
	template<typename T, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::set<T,Traits,Alloc>& v) {
		size_t sum = 0;
		sum += sizeof(std::set<T, Traits, Alloc>);
		for (const auto& e : v) sum += approxSizeOf(e);
		return sum;
	}
	template<typename T1, typename T2>
	size_t approxSizeOf(const std::pair<T1,T2>& v) {
		return approxSizeOf(v.first) + approxSizeOf(v.second);
	}
	template<typename TK, typename TV, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::map<TK,TV,Traits,Alloc>& v) {
		size_t sum = 0;
		sum += sizeof(std::map<TK,TV,Traits,Alloc>);
		for (const auto& e : v) sum += approxSizeOf(e);
		return sum;
	}
	template<typename T>
	size_t approxSizeOf(const RingBuffer<T>& v) {
		size_t sum = 0;
		sum += sizeof(RingBuffer<T>);
		for (const auto& e : v) sum += approxSizeOf(e);

		sum += (v.sizeMax() - v.size()) * sizeof(T);

		return sum;
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