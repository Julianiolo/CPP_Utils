#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#define STRINGUTILS_STD_COMPLIANT 1

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>

namespace StringUtils {
	/*
		General String functions
	*/

	std::string paddLeft(const std::string& s, int paddedLength, char paddWith);
	std::string paddRight(const std::string& s, int paddedLength, char paddWith);

	std::pair<const char*, const char*> stripString(const char* str, const char* str_end = 0);
	std::string_view stripString_(const std::string_view& str);

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

	constexpr const char* findCharInStr(char c, const char* str, const char* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);
		for (const char* ptr = str; ptr < strEnd; ptr++) {
			if (*ptr == c)
				return ptr;
		}
		return nullptr;
	}
	constexpr const char* findCharInStrFromBack(char c, const char* str, const char* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);
		for (const char* ptr = strEnd-1; ptr >= str; ptr--) {
			if (*ptr == c)
				return ptr;
		}
		return nullptr;
	}
	std::vector<std::pair<size_t,std::string>> findStrings(const uint8_t* data, size_t dataLen, size_t minLen = 1);

	int strcasecmp(const char* a, const char* b, const char* a_end = NULL, const char* b_end = NULL);

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
		std::string s(digits,' ');
		uIntToHexBuf<upperCase>(num, &s[0], digits, pad);
		return s;
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
			uint8_t cNum = -1;
			if (c >= '0' && c <= '9')
				cNum = c - '0';
			else if (c == ' ')
				cNum = 0;
			else
				if(base > 10) {
					if (c >= 'A' && c <= 'Z')
						cNum = c - 'A' + 10;
					else if (c >= 'a' && c <= 'z')
						cNum = c - 'a' + 10;
					else
						return -1;
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
		// str at least 3 long
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

	void uIntToBinBuf(uint64_t num, uint8_t digits, char* buf);
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

	/*
		IO
	*/

	std::string loadFileIntoString(const char* path);
	bool writeStringToFile(const std::string& str, const char* path);

	std::vector<uint8_t> loadFileIntoByteArray(const char* path);
	bool writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path);

	bool fileExists(const char* path);

	std::string getDirName(const char* path, const char* path_end = NULL);
	constexpr const char* getFileName(const char* path, const char* path_end = NULL){
		if (path_end == 0)
			path_end = path + strlen(path);

		while(path+1 <= path_end && (*(path_end-1) == '/' || *(path_end-1) == '\\'))
			path_end--;

		const char* lastSlash = findCharInStrFromBack('/', path, path_end);
		const char* lastBSlash = findCharInStrFromBack('\\', path, path_end);
		const char* lastDiv = std::max(lastSlash != nullptr ? lastSlash : 0, lastBSlash != nullptr ? lastBSlash : 0);

		return lastDiv + 1;
	}
	constexpr const char* getFileExtension(const char* path, const char* path_end = NULL){
		if (path_end == 0)
			path_end = path + strlen(path);

		const char* lastSlash = findCharInStrFromBack('/', path, path_end);
		const char* lastBSlash = findCharInStrFromBack('\\', path, path_end);
		const char* lastDiv = std::max(lastSlash != nullptr ? lastSlash : 0, lastBSlash != nullptr ? lastBSlash : 0);

		const char* lastDot = findCharInStrFromBack('.', path, path_end);

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

	std::vector<size_t> generateLineIndexArr(const char* str);

	std::string addThousandsSeperator(const char* str, const char* str_end = 0, const char* seperator = ",");

	std::vector<uint8_t> parseHexFileStr(const char* str, const char* str_end = 0);
}
#endif

/*

uint8_t HexDigitToInt(char digit);
uint32_t HexStrToInt(const char* str);

*/