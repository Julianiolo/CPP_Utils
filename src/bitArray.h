#ifndef __BITARRAY_H__
#define __BITARRAY_H__

#include <cstring>
#include <cmath>

#define BA_DATA_SIZE ((Size/8) + (((Size%8)==0)?0:1))

template<size_t Size>
class BitArray {
private:
    static constexpr size_t size = Size;
    uint8_t data[BA_DATA_SIZE];
public:
    BitArray(){
        std::memset(data, 0, BA_DATA_SIZE);
    }

    void setBitTo(size_t ind, bool val) {
        if(val) {
            data[ind/8] |= 1<<(ind%8);
        }else{
            data[ind/8] &= ~(1<<(ind%8));
        }
    }

    BitArray operator|=(const BitArray<Size>& other) {
        for(size_t i = 0; i<BA_DATA_SIZE; i++) {
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
        for(size_t i = 0; i< BA_DATA_SIZE; i++) {
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
};

#undef BA_DATA_SIZE

#endif