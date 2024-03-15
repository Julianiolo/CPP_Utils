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