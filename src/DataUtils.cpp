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

void DataUtils::ThreadPool::start(uint32_t num_threads) {
	if (threads.size() != 0) // already running
		return;

	should_terminate = false;

	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
	threads.resize(num_threads);
	for (size_t i = 0; i < num_threads; i++) {
		threads[i] = std::thread([&] {
			threadRun();
		});
	}
}
void DataUtils::ThreadPool::stop() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (auto& thread : threads) {
		thread.join();
	}
	threads.clear();
}

bool DataUtils::ThreadPool::busy() {
	bool busy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		busy = !jobs.empty();
	}
	return busy;
}
bool DataUtils::ThreadPool::running() {
	return threads.size() > 0;
}

void DataUtils::ThreadPool::addJob(const std::function<void(void)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

void DataUtils::ThreadPool::threadRun() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
			});
			if (should_terminate) {
				return;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}
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

uint64_t DataUtils::EditMemory::readValue(const uint8_t* data, size_t dataLen, size_t editAddr, uint8_t editType, uint8_t editEndian) {
	DU_ASSERTEX(data, "data is null");
	DU_ASSERTEX(dataLen > 0 && editAddr < dataLen, StringUtils::format("EditMemory: size too small: trying to read at %" DU_PRIuSIZE " of %" DU_PRIuSIZE "bytes", editAddr, dataLen));
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
					throw std::runtime_error(StringUtils::format("EditMemory: size too small: %" DU_PRIuSIZE ", requested: %" DU_PRIuSIZE, dataLen, offset));
				}
				res <<= 8;
				res |= data[offset];
			}
			break;
	}
	return res;
}

std::string DataUtils::EditMemory::readString(const uint8_t* data, size_t dataLen, size_t editAddr, uint8_t editType, bool editReversed) {
	DU_ASSERTEX(data, "data is null");
	DU_ASSERTEX(dataLen > 0 && editAddr < dataLen, StringUtils::format("EditMemory: size too small: trying to read at %" DU_PRIuSIZE " of %" DU_PRIuSIZE "bytes", editAddr, dataLen));
	size_t i = editAddr;
	if (!editReversed) {
		while (true) {
			char c = data[i];

			if (!c)
				break;

			if (!StringUtils::isprint(c)) {
				throw std::runtime_error(StringUtils::format("not a valid character: %u at %" DU_PRIuSIZE, (uint8_t)c, i));
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
				throw std::runtime_error(StringUtils::format("not a valid character: %u at %" DU_PRIuSIZE, (uint8_t)c, i));
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
		throw std::runtime_error(StringUtils::format("Not enough bytes present: only got %" DU_PRIuSIZE, maxSize));
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
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" DU_PRIuSIZE ", got %" DU_PRIuSIZE, bytesToCopy, maxSize));
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
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" DU_PRIuSIZE ", got %" DU_PRIuSIZE, bytesToCopy, maxSize));
		}
		for (size_t i = 0; i < len; i++) {
			size_t offset = !editReversed ? (size_t)i : (len - (size_t)i - 1);
			setValueCallB(addr+offset, (uint8_t)str[i], setValueUserData);
		}
		break;
	}


	case EditType_bytestream:
		if (editStr.length() / 2 > maxSize) {
			throw std::runtime_error(StringUtils::format("Not enough bytes present: requested %" DU_PRIuSIZE ", got %" DU_PRIuSIZE, bytesToCopy, maxSize));
		}
		if (editStr.length() % 2 != 0) {
			throw std::runtime_error(StringUtils::format("bytestream invalid, must be of even length! got %" DU_PRIuSIZE, editStr.length()));
		}
		for (size_t i = 0; i < editStr.length(); i++) {
			const char c = editStr[i];
			if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
				throw std::runtime_error(StringUtils::format("character at %" DU_PRIuSIZE " is not a valid hex character: %c(0x%" PRIx8 ")", i, c, (uint8_t)c));
		}

		for (size_t i = 0; i < editStr.length()/2; i++) {
			size_t offset = !editReversed ? (size_t)i : (editStr.length()/2 - (size_t)i - 1);
			uint8_t byte = StringUtils::hexStrToUIntLen<uint8_t>(editStr.c_str() + i * 2, 2);
			setValueCallB(addr+offset, byte, setValueUserData);
		}
		break;
	}
}