#include <iostream>
#include <cmath>
#include <cfloat>
#include <random>
#include <chrono>

#include "../src/MathUtils.h"
#include "../src/DataUtils.h"

using T = float;
using OutT = uint32_t;
#define TestFunc MathUtils::floatToIEEE754
#define TestFuncBack MathUtils::IEEE754ToFloat

void binToBuf(char* buf, size_t len, uint64_t num) {
	for(size_t i = 0; i < len; i++) {
		buf[len-1-i] = (num&1) ? '1' : '0';
		num >>= 1;
	}
	buf[len] = 0;
}

void printBin32(uint32_t num){
	char buf[128];
	binToBuf(buf, 32, num);
	printf("%s\n", buf);
}
void printBin64(uint64_t num){
	char buf[128];
	binToBuf(buf, 64, num);
	printf("%s\n", buf);
}

T vals[] = {
    0,
    -0,
    1,
    1.1f,
    0.1f,
    0.0000000001f,
    1e29f,
    134564.0f,
    1e20f,
    1e-20f,
    1e-25f,
    1e-45f,
    std::numeric_limits<T>::denorm_min(),
    -std::numeric_limits<T>::denorm_min(),
    std::numeric_limits<T>::max(),
    -std::numeric_limits<T>::max(),
    std::numeric_limits<T>::infinity(),
    -std::numeric_limits<T>::infinity(),
    NAN,
    (T)-NAN
};

void test(T val) {
    OutT vali = *(OutT*)&val;
    OutT valit = TestFunc(val);
    if(vali != valit) {
        printf("%.50f (%.10e):\n", val, val);
        if(sizeof(val) == 4) {
            printf("SEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM\n");
            printBin32(vali);
            printBin32(valit);
        }else {
            printf("SEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n");
            printBin64(vali);
            printBin64(valit);
        }
        

        OutT a = TestFunc(val);
        exit(1);
    }

    T back = TestFuncBack(vali);
    OutT backi = *(OutT*)&back;
    if(backi != vali) {
        printf("Back %.50f (%.10e) != %.50f (%.10e):\n", val, val, back, back);
        if(sizeof(val) == 4) {
            printf("SEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM\n");
            printBin32(vali);
            printBin32(backi);
        }else {
            printf("SEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n");
            printBin64(vali);
            printBin64(backi);
        }
        

        T a = TestFuncBack(vali);
        exit(1);
    }
}

void random_test(T from, T to) {
    static size_t cnt = 42;
    std::mt19937 eng(cnt++);
    std::uniform_real_distribution<> dist(from, to);
    for(size_t i = 0; i<100000000; i++) {
        test(dist(eng));
    }
}

int main() {
    for(size_t i = 0; i<DU_ARRAYSIZE(vals); i++) {
        test(vals[i]);
    }
    printf("edgecases done\n");
#if 0
    random_test(std::numeric_limits<T>::denorm_min(), std::numeric_limits<T>::max());
    random_test(std::numeric_limits<T>::denorm_min(), std::numeric_limits<T>::denorm_min()+1);
    random_test(std::numeric_limits<T>::denorm_min()+1, std::numeric_limits<T>::max());
    random_test(std::numeric_limits<T>::denorm_min(), std::numeric_limits<T>::denorm_min()*100);
    random_test(std::numeric_limits<T>::max()/100, std::numeric_limits<T>::max());

    random_test(-1, 1);
    random_test(-10, 10);
    random_test(-100, 100);
    random_test(-1000, 1000);
    random_test(-.1, .1);
    random_test(-.01, .01);
    random_test(-.001, .001);
    random_test(0, 1);
    random_test(-1, 0);
    printf("random done\n");
#endif
    auto start = std::chrono::high_resolution_clock::now();
    size_t cnt = 0;
    for(uint32_t s = 0; s <= 1; s++) {
        for(uint32_t e = 0; e < 0xFF; e++) {
            for(uint32_t m = 0; m < 1<<23; m++) {
                uint32_t v = s << (8 + 23) | e << 8 | m;
                float vf = *(float*)&v;
                test((T)vf);
                cnt++;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    double time = MathUtils::durationToSeconds(end - start);
    printf("took %fs\n", time);
    printf("%" DU_PRIuSIZE " runs => %es per run\n", cnt, time/cnt);
    return 0;
}