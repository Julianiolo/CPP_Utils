#ifndef __MATHUTILS_H__
#define __MATHUTILS_H__

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
}

#endif