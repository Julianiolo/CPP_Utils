#include <iostream>
#include "../src/ringBuffer.h"
#include "../src/DataUtils.h"



int main() {
	RingBuffer<uint32_t> buf(100);
	for (size_t i = 0; i < 20; i++) {
		buf.add(i);
	}

	for (size_t i = 0; i < 20; i++) {
		DU_ASSERT(buf.get(i) == i);
	}

	for (size_t i = 20; i < 200; i++) {
		buf.add(i);
	}

	for (size_t i = 0; i < 100; i++) {
		DU_ASSERT(buf.get(i) == i+100);
	}

	std::vector<uint32_t> vals;
	for (size_t i = 0; i < 100; i++) {
		vals.push_back(rand());
	}
	for (size_t i = 0; i < 100; i++) {
		DU_ASSERT(buf.get(i) == vals[i]);
	}

	return 0;
}