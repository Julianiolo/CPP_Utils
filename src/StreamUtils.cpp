#include "StreamUtils.h"

StreamUtils::imem_streambuf::imem_streambuf(const char* data, size_t size) {
    char* p = (char*)data;
    this->setg(p,p,p+size);
}