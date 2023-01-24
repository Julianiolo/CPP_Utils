#include "StreamUtils.h"

StreamUtils::imem_streambuf::imem_streambuf(const char* data, size_t size) {
    char* p = (char*)data;
    this->setg(p,p,p+size);
}

std::streamsize StreamUtils::omem_streambuf::xsputn(const char_type* s, std::streamsize n) {
    buf.insert(buf.end(), s, s+n);
    return n;
}
StreamUtils::omem_streambuf::int_type StreamUtils::omem_streambuf::overflow(int_type c) {
    buf.push_back(c);
    return 0;
}