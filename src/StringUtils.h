#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#define STRINGUTILS_STD_COMPLIANT 1

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <algorithm>

namespace StringUtils {
	/*
		General String functions
	*/

	std::string paddLeft(const std::string& s, size_t length, char padd);
	std::string paddRight(const std::string& s, size_t length, char padd);

	void LEllipsisBuf(char* buf, size_t bufSize, const char* s, size_t maxLen);
	void REllipsisBuf(char* buf, size_t bufSize, const char* s, size_t maxLen);

	std::pair<const char*, const char*> stripString(const char* str, const char* str_end = 0);
	std::string_view stripString_(const std::string_view& str);

	std::string replace(const char* str, const char* old, const char* val, const char* str_end = 0);
	std::string replace(const char* str, char old, char val, const char* str_end = 0);
	void replace_(char* str, char old, char val, const char* str_end = 0);

	template<typename ... Args>
	std::string format(const char* str, Args ... args) { // https://stackoverflow.com/a/26221725
		int size_i = std::snprintf(NULL, 0, str, args ...);
		if (size_i <= 0)
			throw std::runtime_error("error during string formatting");

		size_i++; // add size for null term

		char* buf;
		std::string s;
#if STRINGUTILS_STD_COMPLIANT
		buf = new char[size_i];
#else
		s.resize(size_i);
		buf = &s[0];
#endif

		std::snprintf(buf, size_i, str, args ...);

#if STRINGUTILS_STD_COMPLIANT
		s = std::string(buf, buf + size_i - 1);
		delete[] buf;
#endif
		return s;
	}

	constexpr inline bool isprint(uint32_t c) {
		return c > 0x1f && c < 0x7f;
	}
	constexpr inline bool isWhitespace(uint32_t c) {
		return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	}

	inline size_t ustrlen(const char* s) {
		return std::strlen(s);
	}
	inline size_t ustrlen(const wchar_t* s) {
		return std::wcslen(s);
	}

	char* ustrncpy(char* dst, const char* src, size_t len);
	wchar_t* ustrncpy(wchar_t* dst, const wchar_t* src, size_t len);

	char* ustrncat(char* dst, const char* src, size_t len);
	wchar_t* ustrncat(wchar_t* dst, const wchar_t* src, size_t len);

	

	template<typename CHAR_TYPE>
	constexpr const CHAR_TYPE* findCharInStr(CHAR_TYPE c, const CHAR_TYPE* str, const CHAR_TYPE* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + ustrlen(str);
		for (const CHAR_TYPE* ptr = str; ptr < strEnd; ptr++) {
			if (*ptr == c)
				return ptr;
		}
		return nullptr;
	}
	inline const char* findCharInStr(char c, const std::string& str) {
		return findCharInStr(c, str.c_str(), str.c_str() + str.size());
	}
	inline const wchar_t* findCharInStr(wchar_t c, const std::wstring& str) {
		return findCharInStr(c, str.c_str(), str.c_str() + str.size());
	}

	template<typename CHAR_TYPE>
	constexpr const CHAR_TYPE* findCharInStrFromBack(CHAR_TYPE c, const CHAR_TYPE* str, const CHAR_TYPE* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + ustrlen(str);
		for (const CHAR_TYPE* ptr = strEnd-1; ptr >= str; ptr--) {
			if (*ptr == c)
				return ptr;
		}
		return nullptr;
	}
	inline const char* findCharInStrFromBack(char c, const std::string& str) {
		return findCharInStrFromBack<char>(c, str.c_str(), str.c_str() + str.size());
	}
	inline const wchar_t* findCharInStrFromBack(wchar_t c, const std::wstring& str) {
		return findCharInStrFromBack<wchar_t>(c, str.c_str(), str.c_str() + str.size());
	}


	std::vector<std::pair<size_t,std::string>> findStrings(const uint8_t* data, size_t dataLen, size_t minLen = 1);

	int strcasecmp(const char* a, const char* b, const char* a_end = NULL, const char* b_end = NULL);
	const char* strcasestr(const char* str, const char* search, const char* str_end = NULL, const char* search_end = NULL);

	std::string addThousandsSeperator(const char* str, const char* str_end = 0, const char* seperator = ",");
	std::string addThousandsSeperator(const std::string& str, const char* seperator = ",");
	void addThousandsSeperatorBuf(char* buf, size_t size, uint64_t num, const char* seperator = ",");

	int getDecimalsForPrecision(double v, uint8_t n_precision);

	/*
		Conversion functions
	*/

	constexpr char hexDigitsLowerCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	constexpr char hexDigitsUpperCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	extern char texBuf[128];

	bool isValidBaseNum(uint8_t base, const char* str, const char* str_end = 0);

	constexpr uint8_t numStrDigitsNeeded(uint64_t num, uint8_t base) {
		uint8_t cnt = 0;
		while (num) {
			num /= base;
			cnt++;
		}
		return std::max(cnt, (uint8_t)1);
	}

	template<bool upperCase = false>
	void uIntToHexBuf(uint64_t num, char* buf, uint8_t digits = -1, char pad = '0') {
		if (digits == (decltype(digits))-1)
			digits = numStrDigitsNeeded(num,16);

		char* bufPtr = buf + digits;
		while (digits--) {
			if (num) {
				*--bufPtr = (upperCase ? hexDigitsUpperCase : hexDigitsLowerCase)[num & 0xF];
				num >>= 4;
			}
			else {
				*--bufPtr = pad;
			}
		}
	}
	template<bool upperCase = false>
	std::string uIntToHexStr(uint64_t num, uint8_t digits = -1, char pad = '0') {
		if (digits == (decltype(digits))-1)
			digits = numStrDigitsNeeded(num,16);
#if !STRINGUTILS_STD_COMPLIANT
		std::string s(digits,' ');
		uIntToHexBuf<upperCase>(num, &s[0], digits, pad);
		return s;
#else
		char* buf = new char[digits + 1];
		uIntToHexBuf<upperCase>(num, buf, digits, pad);
		buf[digits] = 0;
		std::string s(buf);
		delete[] buf;
		return s;
#endif
	}
	void uIntToHexBufCase(uint64_t num, char* buf, bool upperCase, uint8_t digits = -1, char pad = '0');
	

	void uIntToBuf(uint64_t num, char* buf, uint8_t base = 10, bool upperCase = false, uint8_t digits = -1, char pad = ' ');
	std::string uIntToStr(uint64_t num, uint8_t base = 10, bool upperCase = false, uint8_t digits = -1, char pad = ' ');


	template<typename T = uint64_t>
	constexpr T numBaseStrToUInt(uint8_t base, const char* str, const char* strEnd = nullptr){
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		uint64_t num = 0;
		const char* strPtr = str;
		while (strPtr != strEnd) {
			const char c = *strPtr++;
			uint8_t cNum = -1;
			if (c >= '0' && c <= '9')
				cNum = c - '0';
			else {
				if (c >= 'A' && c <= 'Z')
					cNum = c - 'A' + 10;
				else if (c >= 'a' && c <= 'z')
					cNum = c - 'a' + 10;
				else
					return -1;
			}
			num *= base;
			num |= cNum;
		}
		return num;
	}
	template<uint8_t base, typename T = uint64_t>
	constexpr T numBaseStrToUIntT(const char* str, const char* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		T num = 0;
		const char* strPtr = str;
		while (strPtr != strEnd) {
			const char c = *strPtr++;
			uint8_t cNum = (uint8_t)-1;
			if (c >= '0' && c <= '9')
				cNum = c - '0';
			else if (c == ' ')
				cNum = 0;
			else
				if constexpr (base > 10) {
					if (c >= 'A' && c <= 'Z')
						cNum = c - 'A' + 10;
					else if (c >= 'a' && c <= 'z')
						cNum = c - 'a' + 10;
					else
						return (T)-1;
				}
			num *= base;
			num += cNum;
		}
		return num;
	}

	template<typename T = uint64_t>
	constexpr T binStrToUInt(const char* str, const char* strEnd = nullptr){
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		T out = 0;
		for (const char* cPtr = str; cPtr < strEnd; cPtr++) {
			out <<= 1;
			if (*cPtr == '1')
				out |= 1;
		}
		return out;
	}
	template<typename T = uint64_t>
	constexpr T hexStrToUInt(const char* str, const char* strEnd = nullptr){
		return numBaseStrToUIntT<16,T>(str, strEnd);
	}
	template<typename T = uint64_t>
	constexpr T hexStrToUIntLen(const char* str, size_t len){
		return hexStrToUInt<T>(str, str + len);
	}

	template<typename T = uint64_t>
	constexpr T numStrToUInt(const char* str, const char* strEnd = nullptr){
		if(strEnd == nullptr)
			strEnd = str + std::strlen(str);
			
		// check if str at least 3 long and starts with '0'
		if (str + 2 < strEnd && str[0] == '0') {
			switch (str[1]) {
				case 'b':
					return binStrToUInt<T>(str+2, strEnd);
				case 'x':
					return hexStrToUInt<T>(str+2, strEnd);
				default:
					return numBaseStrToUIntT<8,T>(str+2, strEnd);
			}
		}

		return numBaseStrToUIntT<10,T>(str, strEnd);
	}

	void uIntToBinBuf(uint64_t num, uint8_t digits, char* buf); // buf has to be at least digits+1 big
	std::string uIntToBinStr(uint64_t num, uint8_t digits);

	// this function ignores any f/l suffixes, as the size of the float is explicitly stated through the bits
	uint64_t stof(const char* str, const char* str_end, uint8_t exponent_bits = 8, uint8_t fraction_bits = 23, bool atof_compadible = false);

	template<typename T>
	std::string vectorToStr(const std::vector<T>& vec) {
		return vectorToStr(vec, [](const T& val){
			return std::to_string(val);
		});
	}
	template<typename T, typename F>
	std::string vectorToStr(const std::vector<T>& vec, F toStrFunc) {
		std::string out = "[";
		for(size_t i = 0; i<vec.size(); i++) {
			out += toStrFunc(vec[i]);
			if(i+1 < vec.size())
				out += ", ";
		}
		out += "]";
		return out;
	}

	void byteSizeToBufSmall(char* buf, size_t buf_size, uint64_t bytes);

	std::vector<std::string> split(const std::string& s, const std::string& delimiter);

	void backup_wstr_to_str(char* dest, const wchar_t* src, size_t size);
	void backup_str_to_wstr(wchar_t* dest, const char* src, size_t size);

	/*
		IO
	*/

	std::string loadFileIntoString(const char* path);
	bool writeStringToFile(const std::string& str, const char* path);

	std::vector<uint8_t> loadFileIntoByteArray(const char* path);
	bool writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path);

	bool fileExists(const char* path);

	template<typename CHAR_TYPE, typename STR_TYPE>
	inline STR_TYPE getDirName(const CHAR_TYPE* path, const CHAR_TYPE* path_end = NULL) {
		if (path_end == 0)
			path_end = path + StringUtils::ustrlen(path);

		while(path+1 <= path_end && StringUtils::isWhitespace(*(path_end-1)))
			path_end--;

		if (*(path_end - 1) == '/' || *(path_end - 1) == '\\')
			return STR_TYPE(path, path_end);

		const CHAR_TYPE* lastSlash = findCharInStrFromBack<CHAR_TYPE>('/', path, path_end);
		const CHAR_TYPE* lastBSlash = findCharInStrFromBack<CHAR_TYPE>('\\', path, path_end);
		const CHAR_TYPE* lastDiv = std::max(lastSlash != nullptr ? lastSlash : 0, lastBSlash != nullptr ? lastBSlash : 0);

		if (lastDiv == nullptr)
			return STR_TYPE(path, path_end);

		return STR_TYPE(path, lastDiv);
	}
	inline std::string getDirName(const char* path, const char* path_end = NULL) {
		return getDirName<char, std::string>(path, path_end);
	}
	inline std::wstring getDirName(const wchar_t* path, const wchar_t* path_end = NULL) {
		return getDirName<wchar_t, std::wstring>(path, path_end);
	}

	template<typename CHAR_TYPE>
	constexpr const CHAR_TYPE* getFileName(const CHAR_TYPE* path, const CHAR_TYPE* path_end = NULL, bool acceptBackSlash = true){
		if (path_end == 0)
			path_end = path + StringUtils::ustrlen(path);

		while(path+1 <= path_end && (*(path_end-1) == (CHAR_TYPE)'/' || (acceptBackSlash && *(path_end-1) == (CHAR_TYPE)'\\')))
			path_end--;

		const CHAR_TYPE* lastSlash = StringUtils::findCharInStrFromBack((CHAR_TYPE)'/', path, path_end);
		const CHAR_TYPE* lastBSlash = acceptBackSlash ? StringUtils::findCharInStrFromBack((CHAR_TYPE)'\\', path, path_end) : nullptr;

		if (lastSlash == nullptr && lastBSlash == nullptr) {
			return path;
		}

		const CHAR_TYPE* lastDiv = std::max(lastSlash != nullptr ? lastSlash : 0, lastBSlash != nullptr ? lastBSlash : 0);

		return lastDiv + 1;
	}
	inline const char* getFileName(const std::string& str, bool acceptBackSlash = true) {
		return StringUtils::getFileName<char>(str.c_str(), str.c_str() + str.size(), acceptBackSlash);
	}
	inline const wchar_t* getFileName(const std::wstring& str, bool acceptBackSlash = true) {
		return StringUtils::getFileName<wchar_t>(str.c_str(), str.c_str() + str.size(), acceptBackSlash);
	}
	
	template<typename CHAR_TYPE>
	constexpr const CHAR_TYPE* getFileExtension(const CHAR_TYPE* path, const CHAR_TYPE* path_end = NULL){
		if (path_end == 0)
			path_end = path + StringUtils::ustrlen(path);

		const CHAR_TYPE* lastSlash = StringUtils::findCharInStrFromBack('/', path, path_end);
		const CHAR_TYPE* lastBSlash = StringUtils::findCharInStrFromBack('\\', path, path_end);
		const CHAR_TYPE* lastDiv = std::max(lastSlash != nullptr ? lastSlash : 0, lastBSlash != nullptr ? lastBSlash : 0);

		const CHAR_TYPE* lastDot = StringUtils::findCharInStrFromBack('.', path, path_end);

		if (lastDot && lastDot > lastDiv) {
			return lastDot + 1;
		}
		else {
			return path_end;
		}
	}

	/*
		Various Utility
	*/

	std::string formatTimestamp(const char* fmt, uint64_t time);
	void formatTimestampBuf(char* buf, size_t buf_size, const char* fmt, uint64_t time);

	std::vector<size_t> generateLineIndexArr(const char* str);

	std::vector<uint8_t> parseHexFileStr(const char* str, const char* str_end = 0);
}
#endif
