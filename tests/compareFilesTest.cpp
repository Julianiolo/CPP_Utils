#include <cstdio>

#include "../src/SystemUtils.h"

int main() {
    const char *file1 = "floatTest.cpp";
    const char *file2 = "floa1tTest.cpp";

    printf("%d\n", SystemUtils::compareFiles(file1, file2));
}