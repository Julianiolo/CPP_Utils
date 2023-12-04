#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

#include <cstdint>
#include <functional>

namespace FileUtils {
    bool checkHardlinkedTogether(const char* path1, const char* path2);

    // for unix
    char fileTypeModeToLetter(uint32_t mode);
    void fileModeToStr(char* buf, uint32_t mode); // buffer needs to be AT LEAST 11 bytes big

    bool compareFiles(const char* path1, const char* path2, char* buf1=NULL, char* buf2=NULL, size_t bufSize=0, std::function<bool(uint64_t,uint64_t)> callB=NULL);

    class MappedFile {
    private:
        char* data;
    public:
        MappedFile(const char* path);
    };
};

#endif