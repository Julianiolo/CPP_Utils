#include "ByteStreamWriter.h"

#include <cstring> // memcpy
#include "DataUtils.h"

ByteStreamWriter::ByteStreamWriter(uint8_t* data, size_t dataLen, bool lsbFirst) : data(data), off(0), dataLen(dataLen), lsbFirst(lsbFirst) {

}

template<typename T>
void ByteStreamWriter::writeInt(T value) {
	constexpr const size_t len = sizeof(T);
	if(off+len > dataLen)
		abort();
	
	if(lsbFirst) {
		DataUtils::intToBuf<len,T,true>(value, data+off);
	}else{
		DataUtils::intToBuf<len,T,false>(value, data+off);
	}
	off += len;
}

void ByteStreamWriter::write(uint8_t value) {
	writeInt(value);
}
void ByteStreamWriter::write(int8_t value) {
	write((uint8_t)value);
}
void ByteStreamWriter::write(uint16_t value) {
	writeInt(value);
}
void ByteStreamWriter::write(int16_t value) {
	write((uint16_t)value);
}
void ByteStreamWriter::write(uint32_t value) {
	writeInt(value);
}
void ByteStreamWriter::write(int32_t value) {
	write((uint32_t)value);
}
void ByteStreamWriter::write(uint64_t value) {
	writeInt(value);
}
void ByteStreamWriter::write(int64_t value) {
	write((uint64_t)value);
}

void ByteStreamWriter::writeBytes(const uint8_t* buf, size_t bufLen) {
	DU_DASSERT(off+bufLen <= dataLen);
	
	std::memcpy(data+off, buf, bufLen);
	off += bufLen;
}