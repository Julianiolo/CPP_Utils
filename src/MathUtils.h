#ifndef __MATHUTILS_H__
#define __MATHUTILS_H__

#include <cstdint>
#include <chrono>
#include <cmath>

namespace MathUtils {
	template<typename T>
	T clamp(const T& val, const T& min, const T& max) {
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;
	}

	template<typename T>
	T sq(const T& a) {
		return a*a;
	}

	template<typename T, typename OutT, uint8_t n_exp_bits, uint8_t n_man_bits>
	constexpr OutT _ToIEEE754(T f) {
		constexpr OutT exponent_bias = ((OutT)1 << n_exp_bits) / 2 - 1;

		bool is_negative = std::signbit(f);
		if(is_negative) f = -f;
		
		if (f == 0) {
			OutT res = 0;
			if(is_negative)
				res |= (OutT)1 << (n_exp_bits+n_man_bits);
			return res;
		}

		if(std::isinf(f)) {
			OutT exp = ((OutT)1 << n_exp_bits) - 1;
			return ((OutT)(is_negative ? 1 : 0) << (n_exp_bits+n_man_bits)) | (exp << n_man_bits) | 0;
		}

		if(std::isnan(f)) {
			OutT exp = (1 << n_exp_bits) - 1;
			OutT mantissa = (OutT)1 << (n_man_bits-1); // TODO maybe
			return ((OutT)(is_negative ? 1 : 0) << (n_exp_bits+n_man_bits)) | (exp << n_man_bits) | mantissa;
		}

		int exp;
		T frac = std::frexp(f, &exp);

		OutT mantissa = 0;

		frac *= 2;
		frac -= 1;
		exp--;

		#if 0
		for (size_t i = 0; i<n_man_bits && frac != 0; i++) {
			frac *= 2;
			if(frac >= 1) {
				frac -= 1;
				mantissa |= (OutT)1 << (n_man_bits-1-i);
			}
		}
		#else
		mantissa = (OutT)(frac * ((OutT)1 << n_man_bits)) & (((OutT)1<<n_man_bits)-1);
		#endif

		OutT biased_exp;
		
		if(exp < 0 && (OutT)-exp >= exponent_bias) {
			biased_exp = 0;
			if((OutT)-exp > exponent_bias + n_man_bits - 1) {
				mantissa = 1;
			}else{
				mantissa |= (OutT)1 << n_man_bits;
				mantissa >>= ((OutT)-exp) - exponent_bias + 1;
			}
		}else{
			biased_exp = exp + exponent_bias;
		}

		return ((OutT)(is_negative ? 1: 0) << (n_exp_bits+n_man_bits)) | (biased_exp << n_man_bits) | mantissa;
	}

	// both with sign bit being msb
	constexpr uint32_t floatToIEEE754(float f) { // brute force proven (except for nan values)
		return _ToIEEE754<float,uint32_t,8,23>(f);
	} 
	constexpr uint64_t doubleToIEEE754(double f) { // works in the same way as floatToIEEE754, should also be correct
		return _ToIEEE754<double,uint64_t,11,52>(f);
	}

	template<typename T, typename OutT, uint8_t n_exp_bits, uint8_t n_man_bits>
	constexpr OutT IEEE754To_(T fi) {
		constexpr T exponent_bias = ((T)1 << n_exp_bits) / 2 - 1;

		bool is_negative = !!(fi & ((T)1 << (n_exp_bits + n_man_bits)));

		T exponent_biased = (fi & (((T)1 << (n_exp_bits + n_man_bits))-1)) >> n_man_bits;
		T mantissa = fi & (((T)1 << n_man_bits) - 1);

		int exponent = exponent_biased - exponent_bias;

		if(exponent_biased == ((T)1 << n_exp_bits)-1) {
			OutT res;
			if(mantissa == 0) { // infinity
				res = std::numeric_limits<OutT>::infinity();
			}else{
				res = NAN;
			}
			if(is_negative) res = -res;
			return res;
		}

		OutT res;
		if(exponent_biased == 0) {
			if(mantissa == 0) {
				return is_negative ? (OutT)-0.0 : (OutT)0.0;
			}else{
				res = (OutT)mantissa / ((T)1 << n_man_bits);
				res *= 2;
			}
		}else {
			res = (OutT)(mantissa | ((T)1 << n_man_bits)) / ((T)1 << n_man_bits);
		}
		//res *= std::pow(2, exponent);

		// if(exponent != 0) {
		// 	if(exponent > 0) {
		// 		for(size_t i = 0; i<exponent; i++) {
		// 			res *= 2;
		// 		}
		// 	}else{
		// 		for(size_t i = 0; i<-exponent; i++) {
		// 			res /= 2;
		// 		}
		// 	}
		// }
		if(exponent != 0) {
			if(exponent > 0) {
				for(size_t i = 0; i+31<(uint32_t)exponent; i+=31) {
					res *= (T)1 << 31;
				}
				res *= (T)1 << (exponent % 31);
			}else{
				for(size_t i = 0; i+31<(uint32_t)-exponent; i+=31) {
					res /= (T)1 << 31;
				}
				res /= (T)1 << (-exponent % 31);
			}
		}

		if(res == 0) res = std::numeric_limits<OutT>::denorm_min();

		if(is_negative) res = -res;
		return res;
	}

	constexpr float IEEE754ToFloat(uint32_t fi) { // brute force proven (except for nan values)
		return IEEE754To_<uint32_t,float,8,23>(fi);
	}
	constexpr double IEEE754ToDouble(uint64_t fi) { // works in the same way as floatToIEEE754, should also be correct
		return IEEE754To_<uint64_t,double,11,52>(fi);
	}

	template<typename T1, typename T2>
	double durationToSeconds(std::chrono::duration<T1,T2> duration) {
		double t = (double)std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        t += (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000) / 1e+9;
		return t;
	}
}

#endif