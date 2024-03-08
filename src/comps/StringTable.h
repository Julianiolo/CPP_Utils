#ifndef __STRING_TABLE_H__
#define __STRING_TABLE_H__

#include <vector>
#include <cstring>

#include <unordered_map>
#include <string_view>

#include "DataUtils.h"


class StringTable {
protected:
	std::vector<char> data;
public:
	typedef size_t str;

	inline StringTable() {

	}

	inline void clear() {
		data.clear();
	}

	inline void shrinkToFit() {
		data.shrink_to_fit();
	}

	inline void reserve(size_t amt) {
		data.reserve(amt);
	}

	inline size_t size() const {
		return data.size();
	}

	inline size_t addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
		DU_ASSERT(str_end == 0 || str_end >= str);
		if (str_end == 0)
			str_end = str + std::strlen(str);

		const size_t len = str_end - str;
		const size_t totalLen = nullTerm ? len + 1 : len;

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

	inline const char* getStr(StringTable::str off) const {
		DU_ASSERT(off < data.size());
		return &data[off];
	}

	/*
		returns the offset that has to be added to all entrys of the given table
	*/
	inline size_t addTable(const StringTable& table) {
		const size_t currDataLen = data.size();
		if (table.data.size() > 0) {
			data.resize(currDataLen + table.data.size());

			std::memcpy(&data[currDataLen], &table.data[0], table.data.size());
		}

		return currDataLen;
	}
};

class CachingStringTable : public StringTable {
private:
	std::unordered_map<std::string_view, StringTable::str> cache;
public:
	inline void clear() {
		cache.clear();
		StringTable::clear();
	}

	inline void clearCache() {
		cache.clear();
	}

	inline size_t addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
		if (nullTerm == false || (data.size() > 0 && data[data.size()-1] != 0)) {  // check if previous entry wasn't yet null terminated
			return StringTable::addStr(str, str_end, nullTerm);
		}

		DU_ASSERT(str_end == 0 || str_end >= str);
		if (str_end == 0)
			str_end = str + std::strlen(str);

		const size_t str_len = str_end - str;

		auto res = cache.find(std::string_view(str, str_len));
		if (res != cache.end()) {
			return res->second;
		}

		const size_t off = StringTable::addStr(str, str_end, nullTerm);

		cache[std::string_view(getStr(off), str_len)] = off;

		return off;
	}
};

#endif