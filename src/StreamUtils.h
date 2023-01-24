#ifndef __STREAMUTILS_H__
#define __STREAMUTILS_H__

#include <streambuf>
#include <vector>

namespace StreamUtils {
    struct imem_streambuf : std::streambuf {
        imem_streambuf(const char* data, size_t size);
    };
    
    struct omem_streambuf : std::streambuf {
    public:
        std::vector<char> buf;
        
        std::streamsize xsputn(const char_type* s, std::streamsize n);
        int_type overflow(int_type c);
    };
}

#endif