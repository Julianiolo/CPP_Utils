#include "DataUtils.h"


#include "StringUtils.h"


uint64_t DataUtils::simpleHash(uint64_t v) {
	// TODO: is probably trash

	v ^= 0xFA42FE00;

	v = v << 7 | v >> (64 - 7);
	v *= 47;
	v += 2246;
	v = v << 37 | v >> (64 - 37);
	v *= 63;
	v -= 5124723;
	v = v << 12 | v >> (64 - 12);
	v *= 123;
	v -= 219840392;
	return v;
}

DataUtils::ByteStream::NoDataLeftException::NoDataLeftException(size_t off, size_t getAmt, size_t dataLen) : std::runtime_error(StringUtils::format("Trying to get %" DU_PRIuSIZE " Bytes but only %" DU_PRIuSIZE " are left! total: %" DU_PRIuSIZE, getAmt, dataLen-off,dataLen)), 
off(off), getAmt(getAmt), dataLen(dataLen) 
{

}

DataUtils::ByteStream::ByteStream(const uint8_t* data, size_t dataLen, bool isLsbFirst, size_t startOff) : data(data), dataLen(dataLen), lsbFirst(isLsbFirst), off(startOff) {

}

void DataUtils::ByteStream::setIsLsbFirst(bool isLsbFirst) {
	lsbFirst = isLsbFirst;
}
void DataUtils::ByteStream::setLen(size_t len) {
	dataLen = len;
}

uint64_t DataUtils::ByteStream::getInt(size_t numBytes) {
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
uint8_t DataUtils::ByteStream::getByte(bool advance) {
	if (off + 1 > dataLen)
		throw NoDataLeftException(off, 1, dataLen);

	uint8_t res = data[off];

	if (advance)
		off++;

	return res;
}
std::string_view DataUtils::ByteStream::getBytes(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::string_view res((const char*)data+off, amt);
	off += amt;
	return res;
}
void DataUtils::ByteStream::read(uint8_t* dest, size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::memcpy(dest, data + off, amt);
	off += amt;
}
std::string_view DataUtils::ByteStream::readStr() {
	const char* start = (const char*)data + off;
	while (true) {
		if(off + 1 > dataLen)
			throw NoDataLeftException(off, 1, dataLen);

		char c = data[off++];

		if (!c) {
			const char* end = (const char*)data + off-1;
			return std::string_view(start, end-start);
		}
	}
}

void DataUtils::ByteStream::advance(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	off += amt;
}
void DataUtils::ByteStream::goTo(size_t offset) {
	if (offset > dataLen)
		throw NoDataLeftException(off, offset-off, dataLen);

	off = offset;
}
size_t DataUtils::ByteStream::getOff() const {
	return off;
}

bool DataUtils::ByteStream::canReadAmt(size_t amt) const {
	return off + amt < dataLen;
}
bool DataUtils::ByteStream::hasLeft() const {
	return off < dataLen;
}

uint64_t DataUtils::EditMemory::readValue(const uint8_t* data, size_t dataLen, uint8_t editType, uint8_t editEndian) {
	uint64_t res = 0;
	uint16_t bytesToCopy = 0;
	switch (editType) {
		case EditType_8bit:
			res = (uint64_t)*data;
			break;
		case EditType_16bit:
			bytesToCopy = 2;
			goto read_multi;
		case EditType_32bit:
		case EditType_float:
			bytesToCopy = 4;
			goto read_multi;
		case EditType_64bit: 
		case EditType_double:
			bytesToCopy = 8;
			goto read_multi;

		read_multi:
		
			for (size_t i = 0; i < bytesToCopy; i++) {
				size_t offset = editEndian == EditEndian_Big ? (size_t)i : (bytesToCopy - (size_t)i - 1);
				res <<= 8;
				res |= data[offset];
			}
			break;
	}
	return res;
}
bool DataUtils::EditMemory::writeValue(size_t addr, uint64_t val, const std::string& editStr, SetValueCallB setValueCallB, void* setValueUserData, size_t dataLen, bool editStringTerm, bool editReversed, uint8_t editType, uint8_t editEndian) {
	const size_t maxSize = dataLen - addr;

	uint16_t bytesToCopy = 0;
	switch (editType) {
	case EditType_8bit:
		setValueCallB(addr, (uint8_t)val, setValueUserData);
		break;

	case EditType_16bit:
		bytesToCopy = 2;
		goto edit_cpy_tmpval;
	case EditType_32bit:
		bytesToCopy = 4;
		goto edit_cpy_tmpval;
	case EditType_64bit:
		bytesToCopy = 8;
		goto edit_cpy_tmpval;

	case EditType_float:
		val = StringUtils::stof(editStr.c_str(), editStr.c_str() + editStr.size(), 8, 23);
		bytesToCopy = 4;
		goto edit_cpy_tmpval;
	case EditType_double:
		val = StringUtils::stof(editStr.c_str(), editStr.c_str() + editStr.size(), 11, 52);
		bytesToCopy = 8;
		goto edit_cpy_tmpval;

	edit_cpy_tmpval:
		if (bytesToCopy > maxSize) return false;
		for (size_t i = 0; i < bytesToCopy; i++) {
			size_t offset = editEndian == EditEndian_Big ? (size_t)i : (bytesToCopy - (size_t)i - 1);
			setValueCallB(addr+offset, (uint8_t)((val>>(i*8)) & 0xFF), setValueUserData);
		}
		break;

	case EditType_string:
	{
		const char* str = editStr.c_str();
		size_t len = editStr.length();
		if (editStringTerm)
			len++;

		if (len > maxSize) return false;
		for (size_t i = 0; i < len; i++) {
			size_t offset = !editReversed ? (size_t)i : (len - (size_t)i - 1);
			setValueCallB(addr+offset, (uint8_t)str[i], setValueUserData);
		}
		break;
	}


	case EditType_bytestream:
		if (editStr.length() / 2 > maxSize) return false;
		if (editStr.length() % 2 != 0) return false;
		for (size_t i = 0; i < editStr.length(); i++) {
			const char c = editStr[i];
			if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) return false;
		}

		for (size_t i = 0; i < editStr.length()/2; i++) {
			size_t offset = !editReversed ? (size_t)i : (editStr.length()/2 - (size_t)i - 1);
			uint8_t byte = StringUtils::hexStrToUIntLen<uint8_t>(editStr.c_str() + i * 2, 2);
			setValueCallB(addr+offset, byte, setValueUserData);
		}
		break;
	}

	return true;
}