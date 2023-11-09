#include <stdio.h>

#include "../src/DataUtils.h"
#include "../src/StringUtils.h"

int main() {
    DU_ASSERT(StringUtils::replace("AAxBBx", "x", "yy") == "AAyyBByy");
    DU_ASSERT(StringUtils::replace("AAxxBBx", "xx", "yy") == "AAyyBBx");
    DU_ASSERT(StringUtils::replace("AAyBBy", "x", "yy") == "AAyBBy");

    const std::string s = "AAxBB xxx "  "AAxxBBx xxx"  "AAyBBy xxx";
    DU_ASSERT(StringUtils::replace(s.c_str()+ 0, "x", "yy", s.c_str()+10) == "AAyyBByy");
    DU_ASSERT(StringUtils::replace(s.c_str()+10, "xx", "yy", s.c_str()+20) == "AAyyBBx");
    DU_ASSERT(StringUtils::replace(s.c_str()+20, "x", "yy", s.c_str()+30) == "AAyBBy");

    printf("tests completed\n");
    return 0;
}
