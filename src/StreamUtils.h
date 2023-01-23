#ifndef __STREAMUTILS_H__
#define __STREAMUTILS_H__

#include <streambuf>

namespace StreamUtils {
    struct imem_streambuf : std::streambuf {
        imem_streambuf(const char* data, size_t size);
    };
}

#endif