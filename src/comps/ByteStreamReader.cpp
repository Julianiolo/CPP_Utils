#include "ByteStreamReader.h"

#include <cstring> // memcpy

#include "../StringUtils.h"
#include "../CompilerUtils.h"

ByteStreamReader::NoDataLeftException::NoDataLeftException(size_t off, size_t getAmt, size_t dataLen) 
	: std::runtime_error(StringUtils::format("Trying to get %" CU_PRIuSIZE " Bytes but only %" CU_PRIuSIZE " are left! total: %" CU_PRIuSIZE, getAmt, dataLen-off,dataLen)), 
off(off), getAmt(getAmt), dataLen(dataLen) 
{

}

ByteStreamReader::ByteStreamReader(const uint8_t* data, size_t dataLen, bool isLsbFirst, size_t startOff) : data(data), dataLen(dataLen), lsbFirst(isLsbFirst), off(startOff) {

}

void ByteStreamReader::setIsLsbFirst(bool isLsbFirst) {
	lsbFirst = isLsbFirst;
}
void ByteStreamReader::setLen(size_t len) {
	dataLen = len;
}

uint64_t ByteStreamReader::getInt(size_t numBytes) {
	if (off + numBytes > dataLen)
		throw NoDataLeftException(off, numBytes, dataLen);

	uint64_t out = 0;
	if (!lsbFirst) {
		for (size_t i = 0; i < numBytes; i++) {
			out <<= 8;
			out |= data[off + i];
		}
	}
	else {
		for (size_t i = 0; i < numBytes; i++) {
			out <<= 8;
			out |= data[off + numBytes - 1 - i];
		}
	}

	off += numBytes;

	return out;
}
uint8_t ByteStreamReader::getByte(bool advance) {
	if (off >= dataLen)
		throw NoDataLeftException(off, 1, dataLen);

	uint8_t res = data[off];

	if (advance)
		off++;

	return res;
}
uint8_t ByteStreamReader::getByteAt(size_t off_) const{
	if (off_ >= dataLen)
		throw std::runtime_error(StringUtils::format("Index out of bounds: %" CU_PRIuSIZE " (len: %" CU_PRIuSIZE ")", off_, dataLen));
	return data[off_];
}

std::string_view ByteStreamReader::getBytes(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::string_view res((const char*)data+off, amt);
	off += amt;
	return res;
}
void ByteStreamReader::read(uint8_t* dest, size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::memcpy(dest, data + off, amt);
	off += amt;
}
std::string_view ByteStreamReader::readStr(char term, bool stopOnEnd) {
	const char* start = (const char*)data + off;
	while (true) {
		if(off >= dataLen) {
			throw NoDataLeftException(off, 1, dataLen);
		}

		char c = data[off++];

		if (c == term || (stopOnEnd && off >= dataLen)) {
			const char* end = (const char*)data + off-1;
			return std::string_view(start, end-start);
		}
	}
}

void ByteStreamReader::advance(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	off += amt;
}
void ByteStreamReader::goTo(size_t offset) {
	if (offset > dataLen)
		throw NoDataLeftException(off, offset-off, dataLen);

	off = offset;
}
size_t ByteStreamReader::getOff() const {
	return off;
}

bool ByteStreamReader::canReadAmt(size_t amt) const {
	return off + amt < dataLen;
}
bool ByteStreamReader::hasLeft() const {
	return off < dataLen;
}