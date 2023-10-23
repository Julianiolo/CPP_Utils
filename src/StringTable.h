#ifndef __STRING_TABLE_H__
#define __STRING_TABLE_H__

#include <vector>
#include <cstring>

#include "DataUtils.h"


#if 1

class StringTable {
private:
	std::vector<char> data;
public:
	typedef size_t str;

	inline StringTable() {

	}

	inline void clear() {
		data.clear();
	}

	inline size_t addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
		DU_ASSERT(str_end == 0 || str_end >= str);
		if (str_end == 0)
			str_end = str + std::strlen(str);

		size_t len = str_end - str;
		size_t totalLen = nullTerm ? len + 1 : len;

		const size_t currDataLen = data.size();
		data.resize(currDataLen + totalLen);

		std::memcpy(&data[currDataLen], str, len);

		if (nullTerm)
			data[currDataLen + len] = 0;

		return currDataLen;
	}

	inline void setChar(size_t off, char c) {
		data[off] = c;
	}

	inline const char* getStr(size_t off) const {
		return &data[0] + off;
	}

	inline size_t addTable(const StringTable& table) {
		const size_t currDataLen = data.size();
		if (table.data.size() > 0) {
			data.resize(currDataLen + table.data.size());

			std::memcpy(&data[currDataLen], &table.data[0], table.data.size());
		}

		return currDataLen;
	}

	inline void shrinkToFit() {
		data.shrink_to_fit();
	}

	inline void reserve(size_t amt) {
		data.reserve(amt);
	}
};

#else
class StringTable {
private:
	std::vector<char> data;
	size_t currSize = 0;
	size_t resizeAmt;


	void checkSize(size_t addAmt) {
		if (currSize + addAmt > data.size())
			data.resize(currSize + addAmt + resizeAmt);
	}
public:

	typedef size_t str;

	inline StringTable(size_t resizeAmt = 1000000): resizeAmt(resizeAmt) {

	}

	inline void clear() {
		data.clear();
		currSize = 0;
	}

	inline size_t addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
		DU_ASSERT(str_end == 0 || str_end >= str);
		if (str_end == 0)
			str_end = str + std::strlen(str);

		size_t len = str_end - str;
		size_t totalLen = nullTerm ? len + 1 : len;
		checkSize(totalLen);

		std::memcpy(&data[currSize], str, len);

		if (nullTerm)
			data[currSize + len] = 0;

		size_t off = currSize;
		currSize += totalLen;

		return off;
	}

	inline void setChar(size_t off, char c) {
		data[off] = c;
	}

	inline const char* getStr(size_t off) const {
		return &data[0] + off;
	}

	inline size_t addTable(const StringTable& table) {
		checkSize(table.currSize);

		for (size_t i = 0; i < table.currSize; i++) {
			data[currSize + i] = table.data[i];
		}

		size_t off = currSize;
		currSize += table.currSize;

		return off;
	}
};


#endif

#endif