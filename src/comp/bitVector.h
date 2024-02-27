#ifndef __BITVECTOR_H__
#define __BITVECTOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h> //for size_t
#include <vector>

class BitVector{
private:
    typedef size_t bsize_t;

    
    bsize_t size;
    size_t byteSize;
    std::vector<uint8_t> data;
public:
    BitVector();
    BitVector(bsize_t size);
    bool get(bsize_t i);
    void set(bsize_t i, bool val);

    size_t sizeBytes() const;
};

namespace DataUtils {
    inline size_t approxSizeOf(const BitVector& v) {
        return v.sizeBytes();
    }
}

#endif