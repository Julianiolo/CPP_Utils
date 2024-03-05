#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "StringUtils.h"

#include <cstring>
#include <ctype.h>
#include <ctime>

#include <fstream>
#include <streambuf>
#include <exception>
#include <iostream>
#include <sstream>
#include <cmath>

#include "DataUtils.h"
#include "MathUtils.h"

#ifdef __EMSCRIPTEN__
	#include "emscripten.h"
#endif

char StringUtils::texBuf[128];

std::string StringUtils::replace(const char* str, const char* old, const char* val, const char* str_end) {
	if(str_end == NULL)
		str_end = str + std::strlen(str);

	DU_ASSERT(str_end >= str);

	const size_t strLen = str_end-str;
	const size_t oldLen = std::strlen(old);
	//const size_t valLen = std::strlen(val);

	DU_ASSERT(oldLen > 0);

	const std::string_view sw(str, strLen);

	size_t ptr = 0;
	std::string out;
	do {
		size_t off = sw.find(old, strLen-ptr);
		if (off == std::string_view::npos)
			break;

		out += sw.substr(ptr, off-ptr);
		out += val;

		ptr = off + oldLen;
	} while(ptr < strLen);

	if (ptr < strLen)
		out += sw.substr(ptr, strLen-ptr);

	return out;
}
std::string StringUtils::replace(const char* str, char old, char val, const char* str_end) {
	if(str_end == NULL)
		str_end = str + std::strlen(str);
	std::string s(str, str_end);
	replace_(&s[0], old, val, &s[s.size()]);
	return s;
}
void StringUtils::replace_(char* str, char old, char val, const char* str_end) {
	if(str_end == NULL)
		str_end = str + std::strlen(str);
	const size_t len = str_end - str;

	char* ptr = str;
	while (true) {
		char* occ = (char*)std::memchr(ptr, old, len);
		if (occ == NULL) break;
		*occ = val;
		ptr = occ;
	}
}

std::string StringUtils::paddLeft(const std::string& s, size_t length, char padd) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.begin(), length - out.size(), padd);
	return out;
}
std::string StringUtils::paddRight(const std::string& s, size_t length, char padd) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.end(), length - out.size(), padd);
	return out;
}

void StringUtils::LEllipsisBuf(char* buf, size_t bufSize, const char* s, size_t maxLen) {
	const size_t len = std::strlen(s);
	if(len <= maxLen) {
		std::strncpy(buf, s, bufSize);
		buf[bufSize-1] = '\0';
	}else{
		std::snprintf(buf, bufSize, "...%s", s+(len-maxLen));
	}
}
void StringUtils::REllipsisBuf(char* buf, size_t bufSize, const char* s, size_t maxLen) {
	const size_t len = std::strlen(s);
	if(len <= maxLen) {
		std::strncpy(buf, s, bufSize);
		buf[bufSize-1] = '\0';
	}else{
		if(bufSize<4) {
			std::strncpy(buf, "...", bufSize);
			if(bufSize>0)
				buf[bufSize-1] = 0;
			return;
		}

		const size_t l = std::min(maxLen-3, bufSize-4);
		strncpy(buf, s, l);
		strcpy(buf+l, "...");
	}
}


std::pair<const char*, const char*> StringUtils::stripString(const char* str, const char* str_end) {
	if (str_end == NULL)
		str_end = str + std::strlen(str);

	char c;
	while (str < str_end && ((c = *str) == ' ' || c == '\n' || c == '\r' || c == '\t'))
		str++;

	while (str_end > str+1 && ((c = *(str_end-1)) == ' ' || c == '\n' || c == '\r' || c == '\t'))
		str_end--;

	return {str,str_end};
}
std::string_view StringUtils::stripString_(const std::string_view& str) {
	auto res = stripString(str.data(), str.data() + str.size());
	return std::string_view(res.first, res.second-res.first);
}

// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> StringUtils::split(const std::string& s, const std::string& delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		std::string token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back (s.substr(pos_start));
	return res;
}

int StringUtils::strcasecmp(const char* a, const char* b, const char* a_end, const char* b_end) {
	if (a_end && b_end && (a_end - a != b_end - b)) {
		return a_end - a > b_end - b ? 1 : -1;
	}

	while (true) {
		if (a == a_end || b == b_end) {
			if(a_end == 0 && *a == 0)
				a_end = a;
			if(b_end == 0 && *b == 0)
				b_end = b;

			if (a == a_end && b == b_end)
				return 0;

			return b == b_end ? 1 : -1;
		}

		char ac = *a;
		char bc = *b;

		if (ac >= 'A' && ac <= 'Z')
			ac += 'a' - 'A';
		if (bc >= 'A' && bc <= 'Z')
			bc += 'a' - 'A';

		if (ac != bc || !ac) {
			return ac - bc;
		}

		a++;
		b++;
	}

	return 0;
}

const char* StringUtils::strcasestr(const char* str, const char* search, const char* str_end, const char* search_end) {
	if (str_end == NULL)
		str_end = str + std::strlen(str);
	if (search_end == NULL)
		search_end = search + std::strlen(search);

	size_t search_len = search_end - search;

	if (search_len == 0)
		return str;

	for (const char* s = str; s < str_end-(search_len-1); s++) {
		if (strcasecmp(s, search, s+search_len, search_end) == 0) {
			return s;
		}
	}
	return NULL;
}

char* StringUtils::ustrncpy(char* dst, const char* src, size_t len) {
	return std::strncpy(dst, src, len);
}
wchar_t* StringUtils::ustrncpy(wchar_t* dst, const wchar_t* src, size_t len) {
	return std::wcsncpy(dst, src, len);
}

char* StringUtils::ustrncat(char* dst, const char* src, size_t len) {
	return std::strncat(dst, src, len);
}
wchar_t* StringUtils::ustrncat(wchar_t* dst, const wchar_t* src, size_t len) {
	return std::wcsncat(dst, src, len);
}

void StringUtils::byteSizeToBufSmall(char* buf, size_t buf_size, uint64_t bytes) {
	if(buf_size == 0) return;

	constexpr const char* prefixes[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"};
	size_t prefix = 0;
	
	uint64_t div_factor = 1;
	{
		uint64_t num_ = bytes;
		while(num_ >= 1000) {
			num_ /= 1000;
			div_factor *= 1000;
			prefix++;
		}
	}

	if(prefix >= DU_ARRAYSIZE(prefixes)){
		prefix = 0;
		div_factor = 1;
	}

	snprintf(buf, buf_size, "%" CU_PRIuSIZE "%sB", MathUtils::ceil_div(bytes, div_factor), prefixes[prefix]);
}


void StringUtils::backup_wstr_to_str(char* dest, const wchar_t* src, size_t size) {
	for (size_t i = 0; i < size; i++) {
		wchar_t c = src[i];
		if (c > 127)
			c = '?';
		dest[i] = (char)c;
	}
}
void StringUtils::backup_str_to_wstr(wchar_t* dest, const char* src, size_t size) {
	for (size_t i = 0; i < size; i++) {
		uint8_t c = (uint8_t)src[i];
		if (c > 127)
			c = '?';
		dest[i] = (wchar_t)c;
	}
}


bool StringUtils::isValidBaseNum(uint8_t base, const char* str, const char* str_end) {
	if (str_end == 0)
		str_end = str + std::strlen(str);
	size_t len = str_end - str;

	for (size_t i = 0; i < len; i++) {
		char c = str[i];

		if (c >= '0' && c <= '9') {
			uint8_t val = c - '0';
			if (val >= base) return false;
		}
		else {
			if (c >= 'A' && c <= 'Z')
				c += 'a' - 'A';

			if (c >= 'a' && c <= 'z') {
				uint8_t val = 10 + (c - 'a');
				if (val >= base) return false;
			}
			else {
				return false;
			}
		}
	}

	return true;
}

void StringUtils::uIntToHexBufCase(uint64_t num, char* buf, bool upperCase, uint8_t digits, char pad) {
	if(!upperCase) {
		StringUtils::uIntToHexBuf<false>(num, buf, digits, pad);
	}
	else {
		StringUtils::uIntToHexBuf<true>(num, buf, digits, pad);
	}	
}

void StringUtils::uIntToBuf(uint64_t num, char* buf, uint8_t base, bool upperCase, uint8_t digits, char pad) {
	if (digits == (decltype(digits))-1)
		digits = numStrDigitsNeeded(num,base);
	char* bufPtr = buf + digits;
	while (digits--) {
		if (num) {
			uint8_t cNum = num % base;
			char c = '#';
			if (cNum <= 9)
				c = '0' + cNum;
			else if (cNum <= 36) {
				c = (upperCase ? 'A' : 'a') + (cNum - 10);
			}
			*--bufPtr = c;
			num /= base;
		}
		else {
			*--bufPtr = pad;
		}
	}
}

std::string StringUtils::uIntToStr(uint64_t num, uint8_t base, bool upperCase, uint8_t digits, char pad) {
	if (digits == (decltype(digits))-1)
		digits = numStrDigitsNeeded(num,base);
	std::string s(digits, ' ');
	uIntToBuf(num,&s[0], base, upperCase, digits, pad);
	return s;
}

void StringUtils::uIntToBinBuf(uint64_t num, uint8_t digits, char* buf) {
	char* bufPtr = buf + digits;
	for (uint8_t i = 0; i < digits; i++) {
		*--bufPtr = num & 1 ? '1' : '0';
		num >>= 1;
	}
}
std::string StringUtils::uIntToBinStr(uint64_t num, uint8_t digits) {
	std::string s(digits, ' ');
	uIntToBinBuf(num, digits, &s[0]);
	return s;
}



std::string StringUtils::loadFileIntoString(const char* path) {
#ifdef __EMSCRIPTEN__
	if(StringUtils::findCharInStr(':',path) != nullptr) {
		void* data = nullptr;
		int size = 0;
		int error = 0;
		emscripten_wget_data(path, &data, &size, &error);
		std::string res((size_t)size, ' ');
		if(size > 0)
			std::memcpy(&res[0], data, size);
		return res;
	}
#endif

	std::ifstream t(path, std::ios::binary);
	t.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	
	if(t.fail()){
		throw std::runtime_error(StringUtils::format("could not open the file \"%s\"",path));
	}

#if 0
	t.seekg(0, std::ios::end);
	std::streampos size = t.tellg();
	t.seekg(0, std::ios::beg);

	std::string fileStr((size_t)size, ' ');

	if(size > 0)
		t.read(&fileStr[0], size);
	t.close();
	return fileStr;
#else
	std::stringstream str_strm;
	str_strm << t.rdbuf();
	return str_strm.str();
#endif
}

bool StringUtils::writeStringToFile(const std::string& str, const char* path) {
	std::ofstream out(path);
	out << str;
	out.close();
	return true;
}

std::vector<uint8_t> StringUtils::loadFileIntoByteArray(const char* path) {
#ifdef __EMSCRIPTEN__
	if(StringUtils::findCharInStr(':',path) != nullptr) {
		void* data = nullptr;
		int size = 0;
		int error = 0;
		emscripten_wget_data(path, &data, &size, &error);
		std::vector<uint8_t> res((size_t)size);
		if(size > 0)
			std::memcpy(&res[0], data, size);
		return res;
	}
#endif
	std::ifstream t(path, std::ios::binary);

	if(!t.is_open()){
		throw std::runtime_error(StringUtils::format("could not open the file \"%s\"",path));
	}

	std::vector<uint8_t> byteArr;

	t.seekg(0, std::ios::end);
	uint64_t size = t.tellg();
	t.seekg(0, std::ios::beg);
	byteArr.resize((size_t)size);

	if(size > 0)
		t.read((char*) &byteArr[0], size);

	if (!t.good())
		throw std::runtime_error("stream error");

	t.close();

	return byteArr;
}

bool StringUtils::writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path) {
	std::ofstream out(path, std::ios::binary);
	out.write((const char*)data, dataLen);
	out.close();
	return true;
}

bool StringUtils::fileExists(const char* path) {
	std::ifstream file(path);
	return file.good();
}

std::vector<std::pair<size_t,std::string>> StringUtils::findStrings(const uint8_t* data, size_t dataLen, size_t minLen) {
	std::vector<std::pair<size_t,std::string>> out;
	size_t runStart = 0;
	size_t runCnt = 0;
	for (size_t i = 0; i < dataLen; i++) {
		if (isprint(data[i])) {
			if (runCnt == 0)
				runStart = i;
			runCnt++;
		}
		else {
			if (data[i] == 0 && runCnt >= minLen) {
				out.push_back({ runStart, std::string((const char*)data + runStart, (const char*)data + runStart + runCnt) });
			}
			runCnt = 0;
		}
	}
	return out;
}

/*
	stof[Ex]:
	input: string, [end of string], [number of exponent bits], [number of fraction bits], [whether it should do exactly the same as atof or be more strict]
	output: a 64bit integer which stores n bits of output (n = 1+num_ExpBits+num_FracBits) right aligned
	
	standard values for exponent num and fraction num are:
						exp     frac
	float  (float32):     8       23      + 1 = 32
	double (float64):    11       52      + 1 = 64
*/
uint64_t StringUtils::stof(const char* str, const char* str_end, uint8_t exponent_bits, uint8_t fraction_bits, bool atof_compatible) {
	if (str_end == NULL)
		str_end = str + strlen(str);

	const uint64_t exponent_bias = (1 << exponent_bits) / 2 - 1;

	const char* str_stripped = str;
	const char* str_end_stripped = str_end;

	uint64_t sign = 0, exponent = exponent_bias, fraction = 0;

	while (isspace((int)*str_stripped))
		str_stripped++;

	if (str_stripped >= str_end) { // return NaN/0 if string is just empty (or just whitespace)
		if (atof_compatible)
			goto stof_zero;
		else
			goto stof_nan;
	} 
		

	while (str_end_stripped > str_stripped && isspace((int)*(str_end_stripped-1)))
		str_end_stripped--;

	if(str_end_stripped-str_stripped >= 3){
		const char* check_end = atof_compatible ? str_stripped+3 : str_end_stripped; // if atof_compatible we only check first 3 characters, else everything

		if (strcasecmp(str_stripped, "inf", check_end, 0) == 0 || strcasecmp(str_stripped, "infinity", str_end_stripped, 0) == 0) {
			goto stof_inf;
		}

		if (strcasecmp(str_stripped, "nan", check_end, 0) == 0) {
			goto stof_nan;
		}
	}

	

	{
		const char* str_w = str_stripped;
		const char* str_end_w = str_end_stripped;

		if (*str_w == '+') {
			sign = 0;
			str_w++;
		}
		else if (*str_w == '-') {
			sign = 1;
			str_w++;
		}

		uint64_t digits = 0, decimals = 0;
		int exponent_dec = 0;
		size_t decimals_seq_len = 0;

		// extract digits
		{
			bool skip = false;
			const char* digitSeqEnd = str_w;
			while (true) {
				if (digitSeqEnd >= str_end_stripped) {
					skip = true;
					break;
				}

				if (!isdigit((int)*digitSeqEnd))
					break;

				digitSeqEnd++;
			}

			size_t digitSeqLen = digitSeqEnd - str_w;
			if (digitSeqLen > 0) {
				uint64_t n = 0;
				for (size_t i = 0; i < digitSeqLen; i++) {
					uint64_t n_ = n * 10;

					if (n_ / 10 != n || n_ + (str_w[i] - '0') < n_) { // overflow
						// rounding should not be necessary, because the greatest precision we can have is 63 bit
						exponent_dec += (int)digitSeqLen - (int)i;
						break;
					}

					n = n_ + (str_w[i] - '0');
				}
				digits = n;
			}
			str_w = digitSeqEnd;

			if(skip)
				goto stof_calc;
		}

		// at this point str_w is guaranteed to be < str_end_stripped

		// extract decimals
		if (*str_w == '.') {
			str_w++;

			bool skip = false;

			const char* decimals_seq_end = str_w;
			while (true) {
				if (decimals_seq_end >= str_end_stripped) { // check if at end of string
					skip = true;
					break;
				}

				if (!isdigit((int)*decimals_seq_end))
					break;

				decimals_seq_end++;
			}

			decimals_seq_len = decimals_seq_end - str_w;
			if (decimals_seq_len > 0) {
				uint64_t n = 0;
				for (size_t i = 0; i < decimals_seq_len; i++) {
					uint64_t n_ = n * 10;

					if (n_*10 / 100 != n) { // check for overflow in the next round
						if (str_w[i] - '0' >= 5)
							n++; // round
						decimals_seq_len = i;
						break;
					}

					n = n_ + (str_w[i] - '0');
				}
				decimals = n;
			}

			str_w = decimals_seq_end;

			if(skip)
				goto stof_calc;
		}

		// at this point str_w is guaranteed to be < str_end_stripped

		// extract exponent
		if (*str_w == 'e' || *str_w == 'E') {
			str_w++;

			if (str_w >= str_end_stripped) // illegal
				goto stof_nan;

			bool exp_is_neg = false;

			if (*str_w == '+') {
				exp_is_neg = false;
				str_w++;
			}
			else if (*str_w == '-') {
				exp_is_neg = true;
				str_w++;
			}

			const char* exponent_seq_end = str_w;
			while (true) {
				if (exponent_seq_end >= str_end_stripped) // check if at end of string
					break;

				if (!isdigit((int)*exponent_seq_end))
					break;

				exponent_seq_end++;
			}

			if (exponent_seq_end > str_w) {
				int expOff = 0;
				for (const char* s = str_w; s < exponent_seq_end; s++) { // convert dec string to int
					expOff = expOff * 10 + (*s - '0');
				}
				exponent_dec += exp_is_neg ? -expOff : expOff;
			}

			str_w = exponent_seq_end;
		}


		if (str_w < str_end_w) {
			if (*str_w == 'f' || *str_w == 'F' || *str_w == 'l' || *str_w == 'L')
				str_w++;
		}

		if (!atof_compatible && str_w != str_end_w) // check if there is still something left in the string (illegal, if not atof_compatible)
			goto stof_nan;

stof_calc:
		{
			int exp = 0;

			uint8_t digits_HBS = 0;
			{ // get the highest bit set
				uint64_t x = digits;
				while (x >>= 1) digits_HBS++;
			}


			uint64_t decimals_bin = 0; // binary representation of fraction, but left aligned (msb set = 0.5)
			uint8_t decimals_bin_ind = 0; // basically the length
			if (decimals != 0) {
				if (digits == 0) {
					digits = decimals;
					decimals = 0;
					exponent_dec -= (int)decimals_seq_len;
				}
				else {
					if(digits_HBS < fraction_bits) {
						uint64_t decimals_cpy = decimals;
						uint8_t decimalsBinIndFirst1 = (uint8_t)-1;

						uint64_t decimals_upper_bound = 1;
						for (size_t i = 0; i < decimals_seq_len; i++) {
							uint64_t n = decimals_upper_bound * 10;
							if (n / 10 != decimals_upper_bound) {
								goto stof_zero;
							}
							decimals_upper_bound = n;
						}

						while (decimals_cpy > 0) {
							decimals_cpy <<= 1;
							if (decimals_cpy >= decimals_upper_bound) {
								decimals_cpy -= decimals_upper_bound;
								decimals_bin |= (uint64_t)1 << (64 - decimals_bin_ind - 1);

								if (decimalsBinIndFirst1 == (uint8_t)-1)
									decimalsBinIndFirst1 = decimals_bin_ind;
							}
							decimals_bin_ind++;

							if (decimalsBinIndFirst1 != (uint8_t)-1 && decimals_bin_ind - decimalsBinIndFirst1 > fraction_bits - digits_HBS + 1) // check if enough bits were read
								break;
						}
					}
				}
			}
			
			exp -= decimals_bin_ind;

			uint64_t combined = (digits << decimals_bin_ind) | (decimals_bin >> (64-decimals_bin_ind)); // right aligned (normal)

			if (combined == 0)
				goto stof_zero;
			else if (exponent_dec != 0) {
				while (combined && !(combined & 1)) {
					combined >>= 1;
					exp++;
				}

				exp += exponent_dec; // get the prime factor 2 out of the base 10

				if (exponent_dec > 0) {
					for (int i = 0; i < exponent_dec; i++) {
						uint64_t n;
						while (true) {
							n = combined * 5;
							if (n / 5 != combined) { // overflow detected
								combined >>= 1;
								exp++;
								continue;
							}
							break;
						}
						combined = n;
					}
					while (combined && !(combined & 1)) {
						combined >>= 1;
						exp++;
					}
				}
				else {
					// we left align the current mantissa to get maximum precision when dividing
					while (combined && !(combined & ((uint64_t)1 << 63))) { // left align
						combined <<= 1;
						exp--;
					}
					int cnt = -exponent_dec;
					while(cnt>0){

						// we are dividing in big steps to improve precision
						uint64_t div_by;
						if(cnt < 27){
							div_by = 1;
							for(int i = 0; i < cnt; i++){
								div_by *= 5;
							}
							cnt = 0;
						}else{
							div_by = 7450580596923828125ull; // =5^27
							cnt -= 27;
						}

						uint64_t n = combined / div_by;
						uint64_t residual = combined % div_by;

						uint8_t amt_free_digits_n = 0;
						if(n == 0){
							amt_free_digits_n = 64;
						}else{
							while(!(n & ((uint64_t)1 << 63))){
								n <<= 1;
								amt_free_digits_n++;
							}
						}

						combined = n;

						const uint8_t bitoff = amt_free_digits_n-1;
						for(uint8_t i = 0; i<amt_free_digits_n && residual > 0; i++) {
							residual <<= 1;
							if(residual >= div_by){
								residual -= div_by;
								combined |= (uint64_t)1<<(bitoff-i);
							}
						}

						exp -= amt_free_digits_n;
					}
					while (combined && !(combined & 1)) { // right align again
						combined >>= 1;
						exp++;
					}
				}
			}

			{
				uint8_t comb_HBS = 0;
				{ // get the highest bit set
					uint64_t x = combined;
					while (x >>= 1) comb_HBS++;
				}

				exp += comb_HBS;

				uint8_t shift_off_special = 0;
				if((int64_t)exponent_bias + exp <= 0) { // special case: for effective exponent == 0 we need to shift one less
					shift_off_special = 1;
				}

				uint64_t comb_fraction = combined;
				if ((comb_HBS+shift_off_special) <= fraction_bits) { // shift to fit into fraction of set bitlength
					comb_fraction <<= fraction_bits - comb_HBS - shift_off_special;
				}
				else {
					uint8_t shft_amt = comb_HBS - fraction_bits;
					comb_fraction >>= shft_amt - 1 + shift_off_special; // shift right by one less

					uint8_t round_up = comb_fraction & 1; // get last bit for rounding
					comb_fraction >>= 1; // shift right by the remaining 1
					// now comb_fraction should be < (1<<fraction_bits)
					comb_fraction += round_up;
					
					if(comb_fraction > (((uint64_t)1<<(fraction_bits+1))-1)){ // check if rounding caused overflow
						comb_fraction >>= 1;
						exp++;
					}
				}

				fraction = comb_fraction;
				if(!shift_off_special)
					fraction &= (((uint64_t)1<<fraction_bits)-1); // shear off the highest bit set, since thats always set (by the standard)
			}
			

			if ((int64_t)exponent_bias + exp < 0){ // exponent too small (for regular use) => we enter special exponent=0 case
				exponent = 0;

				if((int64_t)exponent_bias + exp < -fraction_bits) {
					goto stof_zero;
				}else{
					uint8_t shft_amt = (uint8_t)(-((int64_t)exponent_bias + exp)); // garanteed to be > 0

					uint8_t round_up = (fraction&((uint64_t)1<<(shft_amt-1)))!=0;
					fraction >>= shft_amt;
					fraction += round_up; // round up cant cause overflow
				}

				
			} else if (exponent_bias + exp >= ((uint64_t)1 << exponent_bits)){ // exponent too big
				goto stof_inf;
			} else {
				exponent = exponent_bias + exp;
			}
			
			goto stof_end;
		}
	}

stof_inf:
	exponent = ((uint64_t)1 << exponent_bits) - 1; // all bits set
	fraction = 0;
	goto stof_end;

stof_nan:
	exponent = ((uint64_t)1 << exponent_bits) - 1; // all bits set
	fraction = (uint64_t)1 << (fraction_bits-1);
	goto stof_end;

stof_zero:
	exponent = 0;
	fraction = 0;
	goto stof_end;

stof_end:
	return (sign << (exponent_bits + fraction_bits)) | (exponent << fraction_bits) | fraction;
}

std::string StringUtils::formatTimestamp(const char* fmt, uint64_t time) {
	time_t time_ = time;
	auto* tm = std::localtime(&time_);
	if (tm == nullptr) return "ERR localtime";
	char buf[256];
	size_t ret = std::strftime(buf, sizeof(buf), fmt, tm);
	if(ret == 0) return "ERR strftime";
	return buf;
}

std::vector<size_t> StringUtils::generateLineIndexArr(const char* str) {
	std::vector<size_t> lines;

	const char* ptr = str;
	lines.push_back(0);
	while (*ptr) {
		char c = *ptr;
		if (c == '\n') {
			size_t line = (ptr - str) + 1;
			lines.push_back(line);
		}
		ptr++;
	}

	return lines;
}


std::string StringUtils::addThousandsSeperator(const std::string& str, const char* seperator) {
	return addThousandsSeperator(str.c_str(), str.c_str() + str.size(), seperator);
}

std::string StringUtils::addThousandsSeperator(const char* str, const char* str_end, const char* seperator) {
	if (str_end == nullptr)
		str_end = str + std::strlen(str);

	size_t len = str_end - str;
	if (len == 0) return "";

	size_t sep_len = std::strlen(seperator);
	if (sep_len == 0) return str;

	size_t num_seps = (len-1) / 3;
	std::string out(len + num_seps * sep_len , 'A');

	for (size_t i = 0; ; i++) {
		size_t sub_l = std::min((size_t)3, len - i * 3);
		std::memcpy(&out[0]+(out.size()-(i * (3 + sep_len))-sub_l), str_end - (i*3 + sub_l), sub_l); // copy over 3 chars of str

		if (i >= num_seps)
			break;

		std::memcpy(&out[0]+(out.size()-(i * (3 + sep_len))-3-1), seperator, sep_len); // copy over seperator
	}

	return out;
}

void StringUtils::addThousandsSeperatorBuf(char* buf, size_t size, uint64_t num, const char* seperator) {
	char numBuf[256];
	std::snprintf(numBuf, sizeof(numBuf), "%" PRIu64, num);

	size_t num_len = std::strlen(numBuf);

	size_t sep_len = std::strlen(seperator);
	if (sep_len == 0) {
		if (size >= num_len+1) {
			strcpy(buf, numBuf);
		}
		else {
			goto exit_err;
		}
	}

	if ((num_len / 3) * (3+sep_len) + 1 > size) {
		goto exit_err;
	}

	{
		size_t in_off = 0;
		size_t out_off = 0;
		if (num_len % 3 != 0) {
			size_t num_cpy = num_len % 3;
			std::memcpy(buf, numBuf, num_cpy);
			in_off += num_cpy;
			out_off += num_cpy;

			if (num_len > 3) {
				std::memcpy(buf + out_off, seperator, sep_len);
				out_off += sep_len;
			}
		}
		
		while (in_off < num_len) {
			std::memcpy(buf + out_off, numBuf + in_off, 3);
			in_off += 3;
			out_off += 3;

			if (in_off < num_len) {
				std::memcpy(buf + out_off, seperator, sep_len);
				out_off += sep_len;
			}
		}
		buf[out_off] = 0;
	}

	return;
	
exit_err:
	if (size >= 4) {
		std::strcpy(buf, "ERR");
	}
	else {
		buf[0] = 0;
	}
}

int StringUtils::getDecimalsForPrecision(double v, uint8_t n_precision) {
	if (v == 0)
		return n_precision;

	const double l = std::log10(v);
	const int off = std::min((int)std::floor(l), 0);

	return -(off - (n_precision-1));
}


std::vector<uint8_t> StringUtils::parseHexFileStr(const char* str, const char* str_end) {
	if (str_end == nullptr){
		str_end = str + std::strlen(str);
	}
	size_t strl = str_end-str;

	// sanity check (check for non ascii characters)
	for (size_t i = 0; i < strl; i++) {
		unsigned char c = (unsigned char)str[i];
		if (c == 0 || c > 127) {
			throw std::runtime_error(StringUtils::format("Couldn't load from Hex, because it contained a non ASCII character (0x%02x at %" CU_PRIuSIZE ")", c, i));
		}
	}

	DataUtils::ReadByteStream stream((const uint8_t*)str, strl);

	std::vector<uint8_t> res;

	while (stream.hasLeft()) {
		stream.advance(1); // skip :

		uint8_t checksum = 0;

		uint8_t byteCount;
		{
			std::string_view byteCountStr = stream.getBytes(2);
			if (!StringUtils::isValidBaseNum(16, byteCountStr.data(), byteCountStr.data() + 2))
				throw std::runtime_error(
					StringUtils::format("byte count at %" CU_PRIuSIZE " contains non hex values: \"%s\"", 
						stream.getOff()-2, std::string(byteCountStr).c_str())
				);
			byteCount = StringUtils::hexStrToUIntLen<uint8_t>(byteCountStr.data(), 2);
			checksum += byteCount;
		}
		
		uint16_t addr;
		{
			std::string_view addrStr = stream.getBytes(4);
			if (!StringUtils::isValidBaseNum(16, addrStr.data(), addrStr.data() + 4))
				throw std::runtime_error(
					StringUtils::format("addr at %" CU_PRIuSIZE " contains non hex values: \"%s\"", 
						stream.getOff()-4, std::string(addrStr).c_str())
				);
			uint8_t addrH = StringUtils::hexStrToUIntLen<uint8_t>(addrStr.data(), 2);
			uint8_t addrL = StringUtils::hexStrToUIntLen<uint8_t>(addrStr.data()+2, 2);
			checksum += addrH + addrL;
			addr = (uint16_t)addrH << 8 | addrL;
		}
		
		uint8_t type;
		{
			std::string_view typeStr = stream.getBytes(2);
			if (!StringUtils::isValidBaseNum(16, typeStr.data(), typeStr.data() + 2))
				throw std::runtime_error(
					StringUtils::format("type at %" CU_PRIuSIZE " contains non hex values: \"%s\"", 
						stream.getOff()-2, std::string(typeStr).c_str())
				);
			type = StringUtils::hexStrToUIntLen<uint8_t>(typeStr.data(), 2);
			checksum += type;
		}

		std::string_view byteStr = stream.getBytes((size_t)byteCount * 2);
		if (!StringUtils::isValidBaseNum(16, byteStr.data(), byteStr.data() + (size_t)byteCount*2))
			throw std::runtime_error(
				StringUtils::format("data at %" CU_PRIuSIZE " contains non hex values: \"%s\"", 
					stream.getOff()-(size_t)byteCount*2, std::string(byteStr).c_str())
			);
		for (uint8_t i = 0; i < byteCount; i++) {
			uint8_t val = StringUtils::hexStrToUIntLen<uint8_t>(byteStr.data() + i * 2, 2);
			checksum += val;
			res.push_back(val);
		}

		uint8_t checkVal;
		{
			std::string_view checkStr = stream.getBytes(2);
			if (!StringUtils::isValidBaseNum(16, checkStr.data(), checkStr.data() + 2))
				throw std::runtime_error(
					StringUtils::format("checksum at %" CU_PRIuSIZE " contains non hex values: \"%s\"", 
						stream.getOff()-2, std::string(checkStr).c_str())
				);
			checkVal = StringUtils::hexStrToUIntLen<uint8_t>(checkStr.data(), 2);
			checksum += checkVal;
		}
		
		if (checksum != 0)
			throw std::runtime_error(StringUtils::format("checksum doesnt match (%u) at %" CU_PRIuSIZE, (uint32_t)checksum, stream.getOff()));

		while (stream.hasLeft()) {
			char c = stream.getByte(false);
			if (c != '\n' && c != '\r') break;
			stream.advance(1);
		}
	}

	return res;
}