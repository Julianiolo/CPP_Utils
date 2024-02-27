#include "DataUtils.h"


#include "StringUtils.h"

DataUtils::AlignedBuffer::AlignedBuffer(): data(), size(0), buf(NULL) {

}
DataUtils::AlignedBuffer::AlignedBuffer(size_t size, size_t alignment): data(size+alignment-1), size(size), buf(NULL) {
	if(size > 0) {
		buf = &data[0];
		size_t space = size+alignment-1;
		void* buf_ = (void*)buf;
		std::align(alignment, size, buf_, space);
		buf = (char*)buf_;

		DU_ASSERT(space >= size);
	}
}

char* DataUtils::AlignedBuffer::get() const{
	return buf;
}
size_t DataUtils::AlignedBuffer::getSize() const {
	return size;
}

DataUtils::ReadByteStream::NoDataLeftException::NoDataLeftException(size_t off, size_t getAmt, size_t dataLen) 
	: std::runtime_error(StringUtils::format("Trying to get %" CU_PRIuSIZE " Bytes but only %" CU_PRIuSIZE " are left! total: %" CU_PRIuSIZE, getAmt, dataLen-off,dataLen)), 
off(off), getAmt(getAmt), dataLen(dataLen) 
{

}

DataUtils::ReadByteStream::ReadByteStream(const uint8_t* data, size_t dataLen, bool isLsbFirst, size_t startOff) : data(data), dataLen(dataLen), lsbFirst(isLsbFirst), off(startOff) {

}

void DataUtils::ReadByteStream::setIsLsbFirst(bool isLsbFirst) {
	lsbFirst = isLsbFirst;
}
void DataUtils::ReadByteStream::setLen(size_t len) {
	dataLen = len;
}

uint64_t DataUtils::ReadByteStream::getInt(size_t numBytes) {
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
uint8_t DataUtils::ReadByteStream::getByte(bool advance) {
	if (off >= dataLen)
		throw NoDataLeftException(off, 1, dataLen);

	uint8_t res = data[off];

	if (advance)
		off++;

	return res;
}
uint8_t DataUtils::ReadByteStream::getByteAt(size_t off_) const{
	if (off_ >= dataLen)
		throw std::runtime_error(StringUtils::format("Index out of bounds: %" CU_PRIuSIZE " (len: %" CU_PRIuSIZE ")", off_, dataLen));
	return data[off_];
}

std::string_view DataUtils::ReadByteStream::getBytes(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::string_view res((const char*)data+off, amt);
	off += amt;
	return res;
}
void DataUtils::ReadByteStream::read(uint8_t* dest, size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	std::memcpy(dest, data + off, amt);
	off += amt;
}
std::string_view DataUtils::ReadByteStream::readStr(char term, bool stopOnEnd) {
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

void DataUtils::ReadByteStream::advance(size_t amt) {
	if (off + amt > dataLen)
		throw NoDataLeftException(off, amt, dataLen);

	off += amt;
}
void DataUtils::ReadByteStream::goTo(size_t offset) {
	if (offset > dataLen)
		throw NoDataLeftException(off, offset-off, dataLen);

	off = offset;
}
size_t DataUtils::ReadByteStream::getOff() const {
	return off;
}

bool DataUtils::ReadByteStream::canReadAmt(size_t amt) const {
	return off + amt < dataLen;
}
bool DataUtils::ReadByteStream::hasLeft() const {
	return off < dataLen;
}


DataUtils::WriteByteStream::WriteByteStream(uint8_t* data, size_t dataLen, bool lsbFirst) : data(data), off(0), dataLen(dataLen), lsbFirst(lsbFirst) {

}

template<typename T>
void DataUtils::WriteByteStream::writeInt(T value) {
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

void DataUtils::WriteByteStream::write(uint8_t value) {
	writeInt(value);
}
void DataUtils::WriteByteStream::write(int8_t value) {
	write((uint8_t)value);
}
void DataUtils::WriteByteStream::write(uint16_t value) {
	writeInt(value);
}
void DataUtils::WriteByteStream::write(int16_t value) {
	write((uint16_t)value);
}
void DataUtils::WriteByteStream::write(uint32_t value) {
	writeInt(value);
}
void DataUtils::WriteByteStream::write(int32_t value) {
	write((uint32_t)value);
}
void DataUtils::WriteByteStream::write(uint64_t value) {
	writeInt(value);
}
void DataUtils::WriteByteStream::write(int64_t value) {
	write((uint64_t)value);
}

void DataUtils::WriteByteStream::writeBytes(const uint8_t* buf, size_t bufLen) {
	if(off+bufLen > dataLen)
		abort();
	
	std::memcpy(data+off, buf, bufLen);
	off += bufLen;
}





uint64_t DataUtils::EditMemory::readValue(const uint8_t* data, size_t dataLen, size_t editAddr, uint8_t editType, uint8_t editEndian) {
	DU_ASSERTEX(data, "data is null");
	DU_ASSERTEX(dataLen > 0 && editAddr < dataLen, StringUtils::format("EditMemory: size too small: trying to read at %" CU_PRIuSIZE " of %" CU_PRIuSIZE "bytes", editAddr, dataLen));
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
				size_t offset = editAddr + (editEndian == EditEndian_Big ? (size_t)i : (bytesToCopy - (size_t)i - 1));
				if(offset >= dataLen) {
					throw std::runtime_error(StringUtils::format("EditMemory: size too small: %" CU_PRIuSIZE ", requested: %" CU_PRIuSIZE, dataLen, offset));
				}
				res <<= 8;
				res |= data[offset];
			}
			break;
	}
	return res;
}

std::string DataUtils::EditMemory::readString(const uint8_t* data, size_t dataLen, size_t editAddr, bool editReversed) {
	DU_ASSERTEX(data, "data is null");
	DU_ASSERTEX(dataLen > 0 && editAddr < dataLen, StringUtils::format("EditMemory: size too small: trying to read at %" CU_PRIuSIZE " of %" CU_PRIuSIZE "bytes", editAddr, dataLen));
	size_t i = editAddr;
	if (!editReversed) {
		while (true) {
			char c = data[i];

			if (!c)
				break;

			if (!StringUtils::isprint(c)) {
				throw std::runtime_error(StringUtils::format("not a valid character: %u at %" CU_PRIuSIZE, (uint8_t)c, i));
			}

			i++;

			if (i >= dataLen) {
				throw std::runtime_error("reached end of buffer before encountering termination");
			}
		}
		return std::string(data + editAddr, data + i);
	}
	else {
		while (true) {
			char c = data[i];

			if (!c)
				break;

			if (!StringUtils::isprint(c)) {
				throw std::runtime_error(StringUtils::format("not a valid character: %u at %" CU_PRIuSIZE, (uint8_t)c, i));
			}

			if (i == 0) {
				throw std::runtime_error("reached beginning of buffer before encountering termination");
			}

			i--;
		}
		size_t len = editAddr - i;
		std::string res(len, ' ');
		for (size_t j = 0; j < len; j++) {
			res[j] = data[i + len - j - 1];
		}
		return res;
	}
}

void DataUtils::EditMemory::writeValue(size_t addr, uint64_t val, const std::string& editStr, SetValueCallB setValueCallB, void* setValueUserData, size_t dataLen, bool editStringTerm, bool editReversed, uint8_t editType, uint8_t editEndian) {
	DU_ASSERTEX(setValueCallB, "setValueCallB is null");
	const size_t maxSize = dataLen - addr;

	if (maxSize == 0) {
		throw std::runtime_error(StringUtils::format("Not enough bytes present: only got %" CU_PRIuSIZE, maxSize));
	}

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
		if (bytesToCopy > maxSize) {
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" CU_PRIuSIZE ", got %" CU_PRIuSIZE, bytesToCopy, maxSize));
		}
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

		if (bytesToCopy > maxSize) {
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" CU_PRIuSIZE ", got %" CU_PRIuSIZE, bytesToCopy, maxSize));
		}
		for (size_t i = 0; i < len; i++) {
			size_t offset = !editReversed ? (size_t)i : (len - (size_t)i - 1);
			setValueCallB(addr+offset, (uint8_t)str[i], setValueUserData);
		}
		break;
	}


	case EditType_bytestream:
		if (editStr.length() / 2 > maxSize) {
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" CU_PRIuSIZE ", got %" CU_PRIuSIZE, bytesToCopy, maxSize));
		}
		if (editStr.length() % 2 != 0) {
			throw std::runtime_error(StringUtils::format("bytestream invalid, must be of even length! got %" CU_PRIuSIZE, editStr.length()));
		}
		for (size_t i = 0; i < editStr.length(); i++) {
			const char c = editStr[i];
			if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
				throw std::runtime_error(StringUtils::format("character at %" CU_PRIuSIZE " is not a valid hex character: %c(0x%" PRIx8 ")", i, c, (uint8_t)c));
		}

		for (size_t i = 0; i < editStr.length()/2; i++) {
			size_t offset = !editReversed ? (size_t)i : (editStr.length()/2 - (size_t)i - 1);
			uint8_t byte = StringUtils::hexStrToUIntLen<uint8_t>(editStr.c_str() + i * 2, 2);
			setValueCallB(addr+offset, byte, setValueUserData);
		}
		break;
	}
}