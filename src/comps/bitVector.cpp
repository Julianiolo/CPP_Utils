#include "bitVector.h"
#include <stdexcept>

BitVector::BitVector(){

}
BitVector::BitVector(bsize_t size) : size(size), byteSize(size/8+(size%8==0?0:1)), data(byteSize,0){

}
bool BitVector::get(bsize_t i){
    if(i >= size)
        throw std::runtime_error("Bytearray get index out of bounds");
    return (data[i/8] & (1 << i%8)) != 0;
}
void BitVector::set(bsize_t i, bool val){
    if (i / 8 >= byteSize)
        throw std::runtime_error("Byte array set index out of bounds");
    if(val){
        data[i/8] |= (1 << i%8);
    }
    else{
        data[i/8] &= ~(1 << i%8);
    }
}

size_t BitVector::sizeBytes() const {
    return byteSize;
}