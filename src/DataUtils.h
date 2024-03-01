#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <iterator>
#include <cstdint>
#include <string>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <string_view>
#include <climits>
#include <string_view>
#include <vector>

#include "CompilerUtils.h"


inline void __assertion_failed__(const char* exp, const char* file, int line) {
	printf("Assertion Failed: %s %s:%d\n", exp, file, line);
	abort();
}
#define DU_ASSERT(x) do {\
        if(!(x)){\
            __assertion_failed__(#x, DU__FILE__, __LINE__);\
        }\
    } while(0)

#ifndef NDEBUG
#define DU_DASSERT(x) do {\
        if(!(x)){\
            __assertion_failed__(#x, DU__FILE__, __LINE__);\
        }\
    } while(0)
#else
#define DU_DASSERT(x)
#endif



#define DU_ASSERTEX(x, msg) do {\
	if(!(x)){\
            throw std::runtime_error(msg);\
        }\
    } while(0)

#ifdef _MSC_VER
#define DU_STATIC_ASSERT(x) static_assert(x,"")
#else
#define DU_STATIC_ASSERT(x) static_assert(x)
#endif
#define DU_STATIC_ASSERT_MSG(x,msg) static_assert(x,msg)

#define DU_ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))




// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
#define DU_HASH(_x_) std::hash<decltype(_x_)>{}(_x_)
#define DU_HASH_COMB(_h_, _hash_) ((_h_) ^= (_hash_) + 0x9e3779b9 + ((_h_)<<6) + ((_h_)>>2))
#define DU_HASHC(_h_,_x_) DU_HASH_COMB(_h_,(decltype(_h_))DU_HASH(_x_))
#define DU_HASHCB(_h_,_x_,_xlen_) DU_HASH_COMB(_h_,DataUtils::hash_bytes<decltype(_h_)>(_x_,_xlen_))
#define DU_HASHCC(_h_,_container_) DU_HASH_COMB(_h_,DataUtils::hash_bytes<decltype(_h_)>((_container_).size()?&_container_[0]:nullptr,(_container_).size()*sizeof((_container_)[0])))

namespace DataUtils {
	// find value, if not found return (size_t)-1; compare needs to be a function like object with (const T& a, size_t ind_of_b) -> int
	template<typename T,typename CMP>
	size_t binarySearchExclusive(size_t len, const T& value, const CMP& compare) {
		if (len == 0)
			return (size_t)-1;

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
		return (size_t)-1;
	}

	// find value, if not found return where to insert it; compare needs to be a function like object with (const T& a, size_t ind_of_b) -> int
	template<typename T,typename CMP>
	constexpr inline size_t binarySearchInclusive(size_t len, const T& value, const CMP& compare) {
		if (len == 0)
			return 0;

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

	/*
		CMP: (IT, IT) -> int
			 0: Same
			<0: a < b
			>0: a > b

	*/
	template<typename IT, typename ENDA, typename ENDB, typename CMP,typename SAME_FUN, typename ONLY_FUN>
	void compare_sorted(IT a, IT b, ENDA a_end, ENDB b_end, CMP cmp, SAME_FUN same, ONLY_FUN only) {
		while(true) {
			if(a_end(a)) {
				while(!b_end(b)) {
					only(b, 1);
					++b;
				}
				return;
			}

			if(b_end(b)) {
				while(!a_end(a)) {
					only(a, 0);
					++a;
				}
				return;
			}

			const int c = cmp(a, b);
			if(c == 0) {
				same(a,b);
				++a;
				++b;
			}else{
				if(c < 0) {
					only(a,0);
					++a;
				}else {
					only(b,1);
					++b;
				}
			}
		}
	}

	constexpr inline uint64_t simpleHash(uint64_t v){
		// TODO: is probably trash

		v ^= 0xFA42FE00;

		v = v << 7 | v >> (64 - 7);
		v *= 47;
		v += 2246;
		v = v << 37 | v >> (64 - 37);
		v *= 63;
		v -= 5124723;
		v = v << 12 | v >> (64 - 12);
		v *= 123;
		v -= 219840392;
		return v;
	}

	// https://stackoverflow.com/questions/34597260/stdhash-value-on-char-value-and-not-on-memory-address
	template <typename ResultT, ResultT OffsetBasis, ResultT Prime>
	class basic_fnv1a final {
		static_assert(std::is_unsigned<ResultT>::value, "need unsigned integer");
	public:
		using result_type = ResultT;
	private:
		result_type state {};
	public:

		constexpr basic_fnv1a() noexcept : state (OffsetBasis) {

		}

		constexpr void update(const void *const data, const std::size_t size) noexcept {
			const auto cdata = static_cast<const unsigned char *>(data);
			auto acc = this->state;
			for (size_t i = 0; i < size; i++) {
				const result_type next = cdata[i];
				acc = (acc ^ next) * Prime;
			}
			this->state = acc;
		}

		constexpr result_type digest() const noexcept {
			return this->state;
		}
	};

	using fnv1a_32 = basic_fnv1a<std::uint32_t,           UINT32_C(2166136261),      UINT32_C(16777619)>;
	using fnv1a_64 = basic_fnv1a<std::uint64_t, UINT64_C(14695981039346656037), UINT64_C(1099511628211)>;

	template <std::size_t Bits>
	struct fnv1a;

	template <>
	struct fnv1a<32> {
		using type = fnv1a_32;
	};
	template <>
	struct fnv1a<64>{
		using type = fnv1a_64;
	};

	template <std::size_t Bits>
	using fnv1a_t = typename fnv1a<Bits>::type;

	template<typename T = size_t>
	constexpr T hash_bytes(const void *const data, const std::size_t size) noexcept {
		auto hashfn = fnv1a_t<CHAR_BIT * sizeof(T)> {};
		hashfn.update(data, size);
		return hashfn.digest();
	}

	template<size_t bytes, typename T = uint64_t, bool LSB = false>
	T intFromBuf(const void* buf_) {
		const uint8_t* buf = (const uint8_t*)buf_;
		T out = 0;
		if(!LSB) {  // big endian
			for(size_t i = 0; i<bytes; i++) {
				out <<= 8;
				out |= buf[i];
			}
		}else { // little endian
			for(size_t i = 0; i<bytes; i++) {
				out |= (T)buf[i] << i*8;
			}
		}
		return out;
	}

	template<size_t nbytes, typename T = uint64_t, bool LSB = false>
	void intToBuf(const T& t, void* buf_) {
		uint8_t* buf = (uint8_t*)buf_;
		if(!LSB) {  // big endian
			for(size_t i = 0; i<nbytes; i++) {
				buf[i] = (uint8_t)(t >> (nbytes - i - 1)*8);
			}
		}else { // little endian
			for(size_t i = 0; i<nbytes; i++) {
				buf[i] = (uint8_t)(t >> i*8);
			}
		}
	}

	class AlignedBuffer {
	private:
		std::vector<char> data;
		size_t size;
		char* buf;
	public:
		AlignedBuffer();
		AlignedBuffer(size_t size, size_t alignment=32);
		char* get() const;
		size_t getSize() const;
	};

	template<typename T, typename Mutex>
	class ThreadSafeAccessor {
	protected:
		std::unique_lock<Mutex> lock;
		T& val;
	public:
		inline ThreadSafeAccessor(T& val, Mutex& mutex) : lock(mutex), val(val) {}
		T& get() { return val; };
	};

	class ReadByteStream {
	public:
		class NoDataLeftException : public std::runtime_error {
		public:
			size_t off;
			size_t getAmt;
			size_t dataLen;
			NoDataLeftException(size_t off, size_t getAmt, size_t dataLen);
		};

		const uint8_t* data;
		size_t dataLen;
		bool lsbFirst;

		size_t off;

		ReadByteStream(const uint8_t* data, size_t dataLen, bool isLsbFirst = true, size_t startOff = 0);

		void setIsLsbFirst(bool isLsbFirst);
		void setLen(size_t len);

		uint64_t getInt(size_t numBytes);
		uint8_t getByte(bool advance = true);
		uint8_t getByteAt(size_t off) const;
		std::string_view getBytes(size_t amt);
		void read(uint8_t* dest, size_t amt);
		std::string_view readStr(char term = 0, bool stopOnEnd = false);

		void advance(size_t amt);
		void goTo(size_t offset);
		size_t getOff() const;

		bool canReadAmt(size_t amt) const;
		bool hasLeft() const;
	};

	class WriteByteStream {
	public:
		uint8_t* data;
		size_t off;
		size_t dataLen;
		bool lsbFirst;

		WriteByteStream(uint8_t* data, size_t dataLen, bool lsbFirst = false);

		void write(uint8_t value);
		void write(int8_t value);
		void write(uint16_t value);
		void write(int16_t value);
		void write(uint32_t value);
		void write(int32_t value);
		void write(uint64_t value);
		void write(int64_t value);

		void writeBytes(const uint8_t* buf, size_t bufLen);

	private:
		template<typename T>
		void writeInt(T value);
	};

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

		uint64_t readValue(const uint8_t* data, size_t dataLen, size_t editAddr, uint8_t editType, uint8_t editEndian=EditEndian_Little);
		std::string readString(const uint8_t* data, size_t dataLen, size_t editAddr, bool editReversed);
		void writeValue(size_t addr, uint64_t val, const std::string& editStr, SetValueCallB setValueCallB, void* setValueUserData, size_t dataLen, bool editStringTerm, bool editReversed, uint8_t editType, uint8_t editEndian=EditEndian_Little);
	}
}

#endif