#ifndef __STREAMUTILS_H__
#define __STREAMUTILS_H__

#include <iostream>
#include <streambuf>
#include <vector>
#include <type_traits>
#include <cinttypes>
#include <cstring>

namespace StreamUtils {
    struct imem_streambuf : std::streambuf {
        imem_streambuf(const char* data, size_t size);
    };
    
    struct omem_streambuf : std::streambuf {
    public:
        std::vector<char> buf;
        
        std::streamsize xsputn(const char_type* s, std::streamsize n);
        int_type overflow(int_type c);
    };

    template<typename T>
    void write(std::ostream& stream, const T& val){
        if constexpr(
            std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t> || 
            std::is_same_v<T,uint16_t> || std::is_same_v<T,int16_t> || 
            std::is_same_v<T,uint32_t> || std::is_same_v<T,int32_t> || 
            std::is_same_v<T,uint64_t> || std::is_same_v<T,int64_t> || 
            std::is_same_v<T,bool>
        ){
            constexpr size_t size = sizeof(T);
            for(size_t i = size-1; i>0; i--) {
                char v = (uint8_t)(val>>(i*8));
                stream.write(&v,1);
            }

            char v = (uint8_t)val;
            stream.write(&v,1);
        }else if constexpr(std::is_same_v<T,float> || std::is_same_v<T,double>) {
            constexpr size_t size = sizeof(T);
            uint64_t val_;
            if constexpr(size == 4) {
                val_ = *(uint32_t*)&val;
            }else if constexpr(size == 8) {
                val_ = *(uint64_t*)&val;
            }else{
                abort();
            }

            for(ptrdiff_t i = size-1; i>=0; i--) {
                char v = (uint8_t)(val_>>(i*8));
                stream.write(&v,1);
            }
        }else if constexpr(std::is_same_v<T,std::string> || std::is_same_v<T,const char*> || std::is_same_v<T,char*>) {
            const char* s;
            if constexpr(std::is_same_v<T,std::string>) {
                s = val.c_str();
            }else{
                s = val;
            }

            size_t len = std::strlen(s);
            stream.write(s,len+1);
        }else{
            static_assert(sizeof(T) == 0, "no behaviour defined for this type");
        }
    }

    template<typename T>
    void read(std::istream& stream, T* val){
        if constexpr(
            std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t> || 
            std::is_same_v<T,uint16_t> || std::is_same_v<T,int16_t> || 
            std::is_same_v<T,uint32_t> || std::is_same_v<T,int32_t> || 
            std::is_same_v<T,uint64_t> || std::is_same_v<T,int64_t> || 
            std::is_same_v<T,bool>
        ){
            constexpr size_t size = sizeof(T);

            T out = 0;
            for (size_t i = size-1; i>0; i--) {
                char v;
                stream.read(&v,1);
                out |= ((T)(uint8_t)v)<<(i*8);
            }

            char v;
            stream.read(&v,1);
            *val = (uint8_t)v;
            if constexpr(size > 1) {
                *val |= out;
            }
        }else if constexpr(std::is_same_v<T,float> || std::is_same_v<T,double>) {
            constexpr size_t size = sizeof(T);
            uint64_t out = 0;
            for(ptrdiff_t i = size-1; i>=0; i--) {
                char v;
                stream.read(&v,1);
                out |= ((uint64_t)(uint8_t)v)<<(i*8);
            }
            if constexpr(size == 4) {
                uint32_t v = (uint32_t)out;
                *val = *(float*)&v;
            }else if constexpr(size == 8) {
                *val = *(double*)&out;
            }else{
                static_assert(sizeof(T) == 0, "no behaviour defined for this size of float");
            }
        }else if constexpr(std::is_same_v<T,std::string>) {
            std::string s;
            char c;
            while(true) {
                stream.read(&c,1);
                if(c == 0)
                    break;
                s += c;
            };
            *val = std::move(s);
        }else{
            static_assert(sizeof(T) == 0, "no behaviour defined for this type");
        }
    }
}

#endif