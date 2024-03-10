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

	inline StringTable::str addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
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

struct _CST_string_view {
	enum class Mode : uint8_t {
		RawString,
		StringTableString
	};

	const Mode mode;

	struct RawString {
		const char* s;
		const char* s_end;
	};

	struct STString {
		StringTable::str s;
		const StringTable* src;
	};

	union {
		RawString rs;
		STString sts;
	};

	_CST_string_view(const char* str, const char* str_end) : mode(Mode::RawString), rs({ str , str_end ? str_end : str + strlen(str)}) {

	}
	_CST_string_view(StringTable::str s, StringTable* src) : mode(Mode::StringTableString), sts({ s, src }) {

	}

	void setSrc(const StringTable* src) {
		DU_ASSERT(mode == Mode::StringTableString);
		sts.src = src;
	}

	std::string_view getStr() const {
		switch (mode) {
			case Mode::RawString:
				return std::string_view(rs.s, rs.s_end-rs.s);
			case Mode::StringTableString:
				return sts.src->getStr(sts.s);
		}
	}

	bool operator==(const _CST_string_view& other) const {
		return getStr() == other.getStr();
	}

	size_t hash() const {
		return DU_HASH(getStr());
	}
};

template<>
struct std::hash<_CST_string_view> {
	hash() {}
	std::size_t operator()(const _CST_string_view& s) const noexcept {
		return s.hash();
	}
};

class CachingStringTable : public StringTable {
private:
	std::unordered_map<_CST_string_view, StringTable::str> cache;
	
	void constructUpdatedCache(const std::unordered_map<_CST_string_view, StringTable::str>& other_cache) {
		cache.clear();
		cache.reserve(other_cache.size());
		StringTable* me = static_cast<StringTable*>(this);
		
		for (const auto& kv : other_cache) {
			_CST_string_view key = kv.first;
			key.setSrc(me);
			cache.insert({ key, kv.second });
		}
	}
public:
	CachingStringTable(const CachingStringTable& other) : StringTable(other), cache() {
		constructUpdatedCache(other.cache);
	}
	CachingStringTable(CachingStringTable&& other) : StringTable(std::move(other)), cache() {
		constructUpdatedCache(other.cache);
	}

	CachingStringTable& operator=(const CachingStringTable& other) {
		StringTable::operator=(other);
		constructUpdatedCache(other.cache);
		return *this;
	}
	CachingStringTable& operator=(CachingStringTable&& other) {
		StringTable::operator=(std::move(other));
		constructUpdatedCache(other.cache);
		other.cache.clear();
		return *this;
	}

	inline void clear() {
		cache.clear();
		StringTable::clear();
	}

	inline void clearCache() {
		cache.clear();
	}

	inline StringTable::str addStr(const char* str, const char* str_end = 0, bool nullTerm = true) {
		if (nullTerm == false || (data.size() > 0 && data[data.size()-1] != 0)) {  // check if previous entry wasn't yet null terminated
			return StringTable::addStr(str, str_end, nullTerm);
		}

		DU_ASSERT(str_end == 0 || str_end >= str);
		if (str_end == 0)
			str_end = str + std::strlen(str);

		auto res = cache.find(_CST_string_view(str, str_end));
		if (res != cache.end()) {
			return res->second;
		}

		const StringTable::str off = StringTable::addStr(str, str_end, nullTerm);

		cache[_CST_string_view(off, static_cast<StringTable*>(this))] = off;

		return off;
	}
};

#endif