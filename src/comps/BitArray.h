#ifndef __BITARRAY_H__
#define __BITARRAY_H__

#include <cstring>
#include <cmath>
#include <array>
#include <cinttypes>

#include "DataUtils.h"

template<size_t Size>
class BitArray {
private:
    static constexpr size_t byteSize = ((Size / 8) + (((Size % 8) == 0) ? 0 : 1));
    std::array<uint8_t,byteSize> data;
public:
    BitArray(){
        std::memset(&data[0], 0, data.size());
    }

    void setBitTo(size_t ind, bool val) {
        if(val) {
            data[ind/8] |= 1<<(ind%8);
        }else{
            data[ind/8] &= ~(1<<(ind%8));
        }
    }

    BitArray& operator|=(const BitArray<Size>& other) {
        for(size_t i = 0; i<data.size(); i++) {
            data[i] |= other.data[i];
        }
        return *this;
    }

    BitArray operator|(const BitArray<Size>& other) const {
        BitArray ba = *this;
        ba |= other;
        return ba;
    }

    size_t getLBC() const {
        for(size_t i = 0; i< data.size(); i++) {
            if(data[i] != 0xFF) {
                for(uint8_t b = 0; b<8; b++) {
                    if((data[i] & (1<<b)) == 0) {
                        return i*8+b;
                    }
                }
            }
        }
        return -1;
    }

    static constexpr size_t sizeBytes() {
        return byteSize;
    }
};

namespace DataUtils {
    template<size_t size>
    constexpr inline size_t approxSizeOf(const BitArray<size>& v) {
        CU_UNUSED(v);
        return BitArray<size>::sizeBytes();
    }
}

#endif