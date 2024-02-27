#ifndef __CPPUTILS_COMPILER_UTILS_H__
#define __CPPUTILS_COMPILER_UTILS_H__

#include <cinttypes>


#define CU_CPPSTD_20 (__cplusplus >= 202002L)
#define CU_CPPSTD_17 (__cplusplus >= 201703L)
#define CU_CPPSTD_14 (__cplusplus >= 201402L)
#define CU_CPPSTD_11 (__cplusplus >= 201103L)
#define CU_CPPSTD_98 (__cplusplus >= 199711L)


#ifdef __BASE_FILE__
#define DU__FILE__ __BASE_FILE__
#else
#define DU__FILE__ __FILE__
#endif

#define CU_UNUSED(x) do { (void)(x); } while(0)

#if CU_CPPSTD_17
#define CU_FALLTHROUGH [[fallthrough]]
#else
#define CU_FALLTHROUGH // fall through
#endif


// Print size_t macros
#if CU_CPPSTD_11
#define CU_PRIdSIZE "zd"
#define CU_PRIuSIZE "zu"
#define CU_PRIxSIZE "zx"
#else
// fallback that doesn't always manage to dodge compiler warnings (bc e.g. size_t is a unsigned long of 32 bit, but uint32_t is a unsigned int of 32 bit)
// but should still guarantee the right size
#if SIZE_MAX == 0xffffull
#define CU_PRIdSIZE PRId16
#define CU_PRIuSIZE PRIu16
#define CU_PRIxSIZE PRIx16
#elif SIZE_MAX == 0xffffffffull
#define CU_PRIdSIZE PRId32
#define CU_PRIuSIZE PRIu32
#define CU_PRIxSIZE PRIx32
#elif SIZE_MAX == 0xffffffffffffffffull
#define CU_PRIdSIZE PRId64
#define CU_PRIuSIZE PRIu64
#define CU_PRIxSIZE PRIx64
#else
#define CU_PRIdSIZE "d"
#define CU_PRIuSIZE "u"
#define CU_PRIxSIZE "x"
#error lel
#endif
#endif

#define CU_USE_PREDICTION_INDICATORS 1

#if CU_USE_PREDICTION_INDICATORS
#if CU_CPPSTD_20  // check for c++std >= 2020
#define CU_LIKELY [[likely]]
#define CU_UNLIKELY [[unlikely]]

#define CU_CLIKELY(_x_) _x_
#define CU_CUNLIKELY(_x_) _x_
#else
#define CU_LIKELY
#define CU_UNLIKELY

#if defined(__GNUC__) || defined(__CLANG__)
#define CU_CLIKELY(_x_) __builtin_expect(_x_, 1)
#define CU_CUNLIKELY(_x_) __builtin_expect(_x_, 0)
#else
#define CU_CLIKELY(_x_) _x_
#define CU_CUNLIKELY(_x_) _x_
#endif
#endif
#else
#define CU_LIKELY
#define CU_UNLIKELY
#define CU_CLIKELY(_x_) _x_
#define CU_CUNLIKELY(_x_) _x_
#endif

#define CU_IF_LIKELY(_x_) if(CU_CLIKELY(_x_)) CU_LIKELY
#define CU_IF_UNLIKELY(_x_) if(CU_CUNLIKELY(_x_)) CU_UNLIKELY



// ##############################################################################################################
// ########### Member check from https://gist.github.com/maddouri/0da889b331d910f35e05ba3b7b9d869b ##############
// ##############################################################################################################

// A compile-time method for checking the existence of a class member
// @see https://general-purpose.io/2017/03/10/checking-the-existence-of-a-cpp-class-member-at-compile-time/

// This code uses "decltype" which, according to http://en.cppreference.com/w/cpp/compiler_support
// should be supported by Clang 2.9+, GCC 4.3+ and MSVC 2010+ (if you have an older compiler, please upgrade :)
// As of "constexpr", if not supported by your compiler, you could try "const"
// or use the value as an inner enum value e.g. enum { value = ... }

// check "test_has_member.cpp" for a usage example

/// Defines a "has_member_member_name" class template
///
/// This template can be used to check if its "T" argument
/// has a data or function member called "member_name"
#define CU_define_has_member(member_name)                                      \
    template <typename T>                                                      \
    class has_member_##member_name                                             \
    {                                                                          \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U> static yes_type test(decltype(&U::member_name)); \
        template <typename U> static no_type  test(...);                       \
    public:                                                                    \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);  \
    }

/// Shorthand for testing if "class_" has a member called "member_name"
///
/// @note "define_has_member(member_name)" must be used
///       before calling "has_member(class_, member_name)"
#define CU_has_member(class_, member_name)  (has_member_##member_name<class_>::value)

#endif