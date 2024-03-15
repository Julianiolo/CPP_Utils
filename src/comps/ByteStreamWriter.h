#ifndef __CPP_UTILS_BYTESTREAMWRITER_H__
#define __CPP_UTILS_BYTESTREAMWRITER_H__

#include <cstdint>
#include <cstddef> // size_t

class ByteStreamWriter {
public:
    uint8_t* data;
    size_t off;
    size_t dataLen;
    bool lsbFirst;

    ByteStreamWriter(uint8_t* data, size_t dataLen, bool lsbFirst = false);

    void write(uint8_t value);
    void write(int8_t value);
    void write(uint16_t value);
    void write(int16_t value);
    void write(uint32_t value);
    void write(int32_t value);
    void write(uint64_t value);
    void write(int64_t value);

    void writeBytes(const uint8_t* buf, size_t bufLen);

private:
    template<typename T>
    void writeInt(T value);
};

#endif