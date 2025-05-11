#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

#include <cstdint>
#include <functional>
#include <ios>
#include <optional>
#if __cplusplus >= 202002L
#include <version>
#endif
#if defined(__cpp_lib_filesystem)
#include <filesystem>
#endif

namespace FileUtils {
    bool checkHardlinkedTogether(const char* path1, const char* path2);

    // for unix
    char fileTypeModeToLetter(uint32_t mode);
    void fileModeToStr(char* buf, uint32_t mode); // buffer needs to be AT LEAST 11 bytes big

    struct CmpFileResult {
        bool are_same = false;
        uint64_t size_a = (uint64_t)-1;
        uint64_t size_b = (uint64_t)-1;
        uint64_t cmp_offset = (uint64_t)-1;
    };

    class CmpFileError : public std::ios_base::failure {
    public:
        enum class ErrSource : bool {
            A = false,
            B = true
        };
        ErrSource src_;
        CmpFileError(ErrSource src, const std::ios_base::failure& e);

        ErrSource src() const;
    };
    /*
        callB is of form (uint64_t has_read, uint64_t total_size) -> bool; should return true, if comparing should be stopped
    */
#if !defined(__cpp_lib_filesystem)
    #error "lel"
    using PathType = const char*;
#else
    using PathType = const std::filesystem::path&;
#endif
    std::optional<CmpFileResult> compareFiles(PathType pathA, PathType pathB, char* bufA=NULL, char* bufB=NULL, size_t bufSize=0, std::function<bool(uint64_t,uint64_t)> callB=NULL);

    class MappedFile {
    private:
        char* data;
    public:
        MappedFile(const char* path);
    };
};

#endif