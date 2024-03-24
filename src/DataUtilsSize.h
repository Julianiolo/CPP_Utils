#ifndef __DATAUTILSSIZE_H__
#define __DATAUTILSSIZE_H__

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <unordered_set>
#include <map>

#include "DataUtils.h"
#include "comps/ringBuffer.h"

namespace DataUtils {
	inline constexpr size_t approxSizeOf(int8_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint8_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int16_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint16_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int32_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint32_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(int64_t v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(uint64_t v) { return sizeof(v); };
#ifdef __clang__
	inline constexpr size_t approxSizeOf(size_t v) { return sizeof(v); };
#endif
	inline constexpr size_t approxSizeOf(bool v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(float v) { return sizeof(v); };
	inline constexpr size_t approxSizeOf(double v) { return sizeof(v); };

	template<typename T>
	constexpr size_t approxSizeOf(T* v) {
		CU_UNUSED(v);
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
	template<typename T, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::unordered_set<T, Traits, Alloc>& v);
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
		if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_move_assignable_v<T>) {
			size_t sum = sizeof(std::vector<T, Alloc>);
			if (v.capacity() > 0)
				sum += v.capacity() * approxSizeOf(v[0]);
			return sum;
		}
		else {
			size_t sum = 0;
			sum += sizeof(std::vector<T, Alloc>);
			for (const auto& e : v) sum += approxSizeOf(e);

			sum += (v.capacity() - v.size()) * sizeof(T);

			return sum;
		}
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
		if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_move_assignable_v<T>) {
			return sizeof(std::array<T, N>);
		}
		else {
			size_t sum = 0;
			for (const auto& e : v) sum += approxSizeOf(e);
			return sum;
		}
	}
	template<typename T, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::set<T,Traits,Alloc>& v) {
		if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_move_assignable_v<T>) {
			size_t sum = sizeof(std::set<T, Traits, Alloc>);
			if (v.capacity() > 0)
				sum += v.capacity() * approxSizeOf(*v.begin());
			return sum;
		}
		else {
			size_t sum = 0;
			sum += sizeof(std::set<T, Traits, Alloc>);
			for (const auto& e : v) sum += approxSizeOf(e);
			return sum;
		}
	}
	template<typename T, typename Traits, typename Alloc>
	size_t approxSizeOf(const std::unordered_set<T,Traits,Alloc>& v) {
		if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_move_assignable_v<T>) {
			size_t sum = sizeof(std::unordered_set<T, Traits, Alloc>);
			if (v.size() > 0)
				sum += v.size() * approxSizeOf(*v.begin());
			return sum;
		}
		else {
			size_t sum = 0;
			sum += sizeof(std::unordered_set<T, Traits, Alloc>);
			for (const auto& e : v) sum += approxSizeOf(e);
			return sum;
		}
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
}



#endif