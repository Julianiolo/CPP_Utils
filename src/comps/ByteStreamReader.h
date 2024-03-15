#ifndef __CPP_UTILS_BYTESTREAMWRITER_H__
#define __CPP_UTILS_BYTESTREAMWRITER_H__

#include <cstdint>
#include <string_view>
#include <stdexcept>

class ByteStreamReader {
public:
    class NoDataLeftException : public std::runtime_error {
    public:
        size_t off;
        size_t getAmt;
        size_t dataLen;
        NoDataLeftException(size_t off, size_t getAmt, size_t dataLen);
    };

    const uint8_t* data;
    size_t dataLen;
    bool lsbFirst;

    size_t off;

    ByteStreamReader(const uint8_t* data, size_t dataLen, bool isLsbFirst = true, size_t startOff = 0);

    void setIsLsbFirst(bool isLsbFirst);
    void setLen(size_t len);

    uint64_t getInt(size_t numBytes);
    uint8_t getByte(bool advance = true);
    uint8_t getByteAt(size_t off) const;
    std::string_view getBytes(size_t amt);
    void read(uint8_t* dest, size_t amt);
    std::string_view readStr(char term = 0, bool stopOnEnd = false);

    void advance(size_t amt);
    void goTo(size_t offset);
    size_t getOff() const;

    bool canReadAmt(size_t amt) const;
    bool hasLeft() const;
};

#endif