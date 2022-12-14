#include "StringUtils.h"

#include <cstring>
#include <ctype.h>

#include <fstream>
#include <streambuf>

char StringUtils::texBuf[128];

void StringUtils::uIntToHexBufCase(uint64_t num, uint8_t digits, char* buf, bool upperCase) {
	if(!upperCase) {
		StringUtils::uIntToHexBuf<false>(num, digits, buf);
	}
	else {
		StringUtils::uIntToHexBuf<true>(num, digits, buf);
	}	
}

void StringUtils::uIntToNumBaseBuf(uint64_t num, uint8_t digits, char* buf, uint8_t base, bool upperCase) {
	char* bufPtr = buf + digits;
	while (digits--) {
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
}

std::string StringUtils::uIntToNumBaseStr(uint64_t num, uint8_t digits, uint8_t base, bool upperCase) {
	std::string s(digits, ' ');
	uIntToNumBaseBuf(num, digits, &s[0], base, upperCase);
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

std::string StringUtils::paddLeft(const std::string& s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.begin(), paddedLength - out.size(), paddWith);
	return out;
}
std::string StringUtils::paddRight(const std::string& s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.end(), paddedLength - out.size(), paddWith);
	return out;
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

std::string StringUtils::loadFileIntoString(const char* path, bool* success) {
	std::ifstream t(path, std::ios::binary);
	
	if(!t.is_open()){
		if (success)
			*success = false;
		return "";
	}

	t.seekg(0, std::ios::end);
	uint64_t size = t.tellg();
	t.seekg(0, std::ios::beg);
	std::string fileStr((size_t)size, ' ');

	t.read(&fileStr[0], size);
	t.close();
	if (success)
		*success = true;
	return fileStr;
}

bool StringUtils::writeStringToFile(const std::string& str, const char* path) {
	std::ofstream out(path);
	out << str;
	out.close();
	return true;
}

std::vector<uint8_t> StringUtils::loadFileIntoByteArray(const char* path, bool* success) {
	std::ifstream t(path, std::ios::binary);
	std::vector<uint8_t> byteArr;

	if(!t.is_open()){
		if (success)
			*success = false;
		return byteArr;
	}

	byteArr.clear();

	t.seekg(0, std::ios::end);
	uint64_t size = t.tellg();
	t.seekg(0, std::ios::beg);
	byteArr.resize((size_t)size);

	t.read((char*) & byteArr[0], size);
	t.close();

	if (success)
		*success = true;

	return byteArr;
}

bool StringUtils::writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path) {
	std::ofstream out(path, std::ios::binary);
	out.write((const char*)data, dataLen);
	out.close();
	return true;
}



size_t StringUtils::findCharInStr(char c, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);
	for (const char* ptr = str; ptr < strEnd; ptr++) {
		if (*ptr == c)
			return ptr - str;
	}
	return -1;
}
size_t StringUtils::findCharInStrFromBack(char c, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);
	for (const char* ptr = strEnd-1; ptr >= str; ptr--) {
		if (*ptr == c)
			return ptr - str;
	}
	return -1;
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

		if (ac != bc || !a) {
			return ac - bc;
		}

		a++;
		b++;
	}

	return 0;
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
						uint8_t decimalsBinIndFirst1 = -1;

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
					uint8_t shft_amt = -((int64_t)exponent_bias + exp); // garanteed to be > 0

					uint8_t round_up = (fraction&(1<<(shft_amt-1)))!=0;
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

uint8_t StringUtils::getLBS(uint64_t x) {
	if (x == 0) return 64;

	uint8_t ret = 0;
	while ((x & 1) == 0) {
		x >>= 1;
		ret++;
	}
	return ret;
}
uint8_t StringUtils::getHBS(uint64_t x) {
	uint8_t ret = 0;
	while (x >>= 1) ret++;
	return ret;
}


std::string StringUtils::getDirName(const char* path, const char* path_end) {
	if (path_end == 0)
		path_end = path + strlen(path);

	while(path+1 <= path_end && (*(path_end-1) == '/' || *(path_end-1) == '\\'))
		path_end--;

	size_t lastSlash = findCharInStrFromBack('/', path, path_end);
	size_t lastBSlash = findCharInStrFromBack('\\', path, path_end);
	size_t lastDiv = std::max(lastSlash != (size_t)-1 ? lastSlash : 0, lastBSlash != (size_t)-1 ? lastBSlash : 0);

	if(path+lastDiv+1 >= path_end)
		return "";

	return std::string(path + lastDiv + 1, path_end);
}
const char* StringUtils::getFileName(const char* path, const char* path_end) {
	if (path_end == 0)
		path_end = path + strlen(path);

	while(path+1 <= path_end && (*(path_end-1) == '/' || *(path_end-1) == '\\'))
		path_end--;

	size_t lastSlash = findCharInStrFromBack('/', path, path_end);
	size_t lastBSlash = findCharInStrFromBack('\\', path, path_end);
	size_t lastDiv = std::max(lastSlash != (size_t)-1 ? lastSlash : 0, lastBSlash != (size_t)-1 ? lastBSlash : 0);

	return path + lastDiv + 1;
}
const char* StringUtils::getFileExtension(const char* path, const char* path_end) {
	if (path_end == 0)
		path_end = path + strlen(path);

	size_t lastSlash = findCharInStrFromBack('/', path, path_end);
	size_t lastBSlash = findCharInStrFromBack('\\', path, path_end);
	size_t lastDiv = std::max(lastSlash != (size_t)-1 ? lastSlash : 0, lastBSlash != (size_t)-1 ? lastBSlash : 0);

	size_t lastDot = findCharInStrFromBack('.', path, path_end);

	if (lastDot > lastDiv) {
		return path + lastDot + 1;
	}
	else {
		return path_end;
	}
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