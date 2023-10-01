#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <cinttypes>
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

// Print size_t macros
#if SIZE_MAX == 0xffffull
#define DU_PRIdSIZE PRId16
#define DU_PRIuSIZE PRIu16
#define DU_PRIxSIZE PRIx16
#elif SIZE_MAX == 0xffffffffull
#define DU_PRIdSIZE PRId32
#define DU_PRIuSIZE PRIu32
#define DU_PRIxSIZE PRIx32
#elif SIZE_MAX == 0xffffffffffffffffull
#define DU_PRIdSIZE PRId64
#define DU_PRIuSIZE PRIu64
#define DU_PRIxSIZE PRIx64
#else
#define DU_PRIdSIZE "d"
#define DU_PRIuSIZE "u"
#define DU_PRIxSIZE "x"
#error lel
#endif

inline void __assertion_failed__(const char* file, int line) {
	printf("Assertion Failed! %s:%d\n", file, line);
	abort();
}
#ifdef _DEBUG
#define DU_ASSERT(x) do {\
        if(!(x)){\
            __assertion_failed__(__FILE__, __LINE__);\
        }\
    } while(0)
#else
#define DU_ASSERT(x)
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

#define DU_UNUSED(x) do { (void)(x); } while(0)

#if __cplusplus >= 201703L
#define DU_FALLTHROUGH [[fallthrough]]
#else
#define DU_FALLTHROUGH // fall through
#endif

#define DU_USE_PREDICTION_INDICATORS 1

#if DU_USE_PREDICTION_INDICATORS
#if __cplusplus >= 202002L
#define DU_LIKELY [[likely]]
#define DU_UNLIKELY [[unlikely]]

#define DU_CLIKELY(_x_) _x_
#define DU_CUNLIKELY(_x_) _x_
#else
#define DU_LIKELY
#define DU_UNLIKELY

#if defined(__GNUC__) || defined(__CLANG__)
#define DU_CLIKELY(_x_) __builtin_expect(_x_, 1)
#define DU_CUNLIKELY(_x_) __builtin_expect(_x_, 0)
#else
#define DU_CLIKELY(_x_) _x_
#define DU_CUNLIKELY(_x_) _x_
#endif
#endif
#else
#define DU_LIKELY
#define DU_UNLIKELY
#define DU_CLIKELY(_x_) _x_
#define DU_CUNLIKELY(_x_) _x_
#endif

#define DU_IF_LIKELY(_x_) if(DU_CLIKELY(_x_)) DU_LIKELY
#define DU_IF_UNLIKELY(_x_) if(DU_CUNLIKELY(_x_)) DU_UNLIKELY


// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
#define DU_HASH(_x_) std::hash<decltype(_x_)>{}(_x_)
#define DU_HASH_COMB(_h_, _hash_) ((_h_) ^= (_hash_) + 0x9e3779b9 + ((_h_)<<6) + ((_h_)>>2))
#define DU_HASHC(_h_,_x_) DU_HASH_COMB(_h_,(decltype(_h_))DU_HASH(_x_))
#define DU_HASHCB(_h_,_x_,_xlen_) DU_HASH_COMB(_h_,DataUtils::hash_bytes<decltype(_h_)>(_x_,_xlen_))
#define DU_HASHCC(_h_,_container_) DU_HASH_COMB(_h_,DataUtils::hash_bytes<decltype(_h_)>((_container_).size()?&_container_[0]:nullptr,(_container_).size()*sizeof((_container_)[0])))

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

	// find value, if not found return where to insert it; compare needs to be a function like object with (const T& a, size_t ind_of_b) -> int
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

	template<size_t bytes, typename T = uint64_t, bool LSB = false>
	void intToBuf(const T& t, void* buf_) {
		uint8_t* buf = (uint8_t*)buf_;
		if(!LSB) {  // big endian
			for(size_t i = 0; i<bytes; i++) {
				buf[i] = (uint8_t)(t >> (bytes - i - 1)*8);
			}
		}else { // little endian
			for(size_t i = 0; i<bytes; i++) {
				buf[i] = (uint8_t)(t >> i*8);
			}
		}
	}

	template<typename T, typename Mutex>
	class ThreadSafeAccessor {
	protected:
		std::unique_lock<Mutex> lock;
		T& val;
	public:
		inline ThreadSafeAccessor(T& val, Mutex& mutex) : lock(mutex), val(val) {}
		T& get() { return val; };
	};

	class ByteStream {
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

		ByteStream(const uint8_t* data, size_t dataLen, bool isLsbFirst = true, size_t startOff = 0);

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