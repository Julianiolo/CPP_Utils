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