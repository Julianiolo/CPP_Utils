#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#include <iterator>
#include <cstdint>
#include <cinttypes>
#include <string>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>

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
#error
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

	uint64_t simpleHash(uint64_t v);

	// heavily inspired by this: https://stackoverflow.com/a/32593825
	class ThreadPool {
	private:
		bool should_terminate = false;
		std::mutex queue_mutex;
		std::condition_variable mutex_condition;
		std::vector<std::thread> threads;
		std::queue<std::function<void(void)>> jobs;

		void threadRun();
	public:
		void start(uint32_t num_threads = -1);
		void stop();
		bool busy();
		bool running();
		void addJob(const std::function<void(void)>& job);
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

		uint64_t readValue(const uint8_t* data, size_t dataLen, uint8_t editType, uint8_t editEndian=EditEndian_Little);
		bool writeValue(size_t addr, uint64_t val, const std::string& editStr, SetValueCallB setValueCallB, void* setValueUserData, size_t dataLen, bool editStringTerm, bool editReversed, uint8_t editType, uint8_t editEndian=EditEndian_Little);
	}
}

#endif