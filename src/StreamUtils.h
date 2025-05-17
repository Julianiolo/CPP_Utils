#ifndef __STREAMUTILS_H__
#define __STREAMUTILS_H__

#include <iostream>
#include <streambuf>
#include <vector>
#include <type_traits>
#include <cinttypes>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace StreamUtils {
    template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    void write(std::ostream& stream, const T& val) {
        constexpr size_t size = sizeof(T);
        for(size_t i = size-1; i>0; i--) {
            char v = (uint8_t)(val>>(i*8));
            stream.write(&v,1);
        }

        char v = (uint8_t)val;
        stream.write(&v,1);
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void write(std::ostream& stream, const T& val) {
        constexpr size_t size = sizeof(T);
        uint64_t val_;
        if constexpr(size == 4) {
            val_ = *(uint32_t*)&val;
        }else if constexpr(size == 8) {
            val_ = *(uint64_t*)&val;
        }else{
            static_assert(sizeof(T) == -1, "no behaviour defined for this size of float");
        }

        for(ptrdiff_t i = size-1; i>=0; i--) {
            char v = (uint8_t)(val_>>(i*8));
            stream.write(&v,1);
        }
    }
    
    void write(std::ostream& stream, const char* val) {
        const size_t len = std::strlen(val);
        StreamUtils::write(stream, (uint64_t)len);
        stream.write(val, len);
    }
    template<typename CharT>
    void write(std::ostream& stream, const std::basic_string<CharT>& val) {
        StreamUtils::write(stream, (uint64_t)val.size());
        stream.write(val.data(), val.size()*sizeof(CharT));
    }
    template<typename T, typename Alloc>
    void write(std::ostream& stream, const std::vector<T, Alloc>& val) {
        StreamUtils::write(stream, (uint64_t)val.size());
        stream.write(val.data(), val.size() * sizeof(*val.data()));
    }
    template<typename T, typename Hash, typename Pred, typename Alloc>
    void write(std::ostream& stream, const std::unordered_set<T, Hash, Pred, Alloc>& val) {
        StreamUtils::write(stream, (uint64_t)val.size());
        for(const auto& v : val)
            StreamUtils::write(stream, v);
    }
    template<typename TK, typename TV, typename Hash, typename Pred, typename Alloc>
    void write(std::ostream& stream, const std::unordered_map<TK, TV, Hash, Pred, Alloc>& val) {
        StreamUtils::write(stream, (uint64_t)val.size());
        for(const auto& [k, v] : val) {
            StreamUtils::write(stream, k);
            StreamUtils::write(stream, v);
        }
    }

    template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    void read(std::istream& stream, T* val){
        constexpr size_t size = sizeof(T);

        T out = 0;
        if constexpr (size > 1) {
            for (size_t i = size-1; i>0; i--) {
                char v;
                stream.read(&v,1);
                out |= ((T)(uint8_t)v)<<(i*8);
            }
        }

        char v;
        stream.read(&v,1);
        *val = (uint8_t)v;
        if constexpr(size > 1) {
            *val |= out;
        }
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void read(std::istream& stream, T* val){
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
    }

    template<typename CharT>
    void read(std::istream& stream, std::basic_string<CharT>* val) {
        uint64_t len;
        StreamUtils::read(stream, &len);
        
        std::basic_string<CharT> s;
        s.resize(len);
        stream.read(s.data(), len*sizeof(CharT));

        *val = std::move(s);
    }

    template<typename T, typename Alloc>
    void read(std::istream& stream, std::vector<T, Alloc>* val) {
        uint64_t len;
        StreamUtils::read(stream, &len);
        stream.read(val.data(), val.size() * sizeof(*val.data()));
    }

    template<typename T, typename Hash, typename Pred, typename Alloc>
    void read(std::istream& stream, std::unordered_set<T, Hash, Pred, Alloc>* val) {
        uint64_t len;
        StreamUtils::read(stream, &len);
        val->reserve(len);
        for(size_t i = 0; i<len; i++) {
            T t;
            StreamUtils::read(stream, &t);
            val->insert(std::move(t));
        }
    }
    template<typename TK, typename TV, typename Hash, typename Pred, typename Alloc>
    void read(std::istream& stream, std::unordered_map<TK, TV, Hash, Pred, Alloc>* val) {
        uint64_t len;
        StreamUtils::read(stream, &len);
        val->reserve(len);
        for(size_t i = 0; i<len; i++) {
            TK tk;
            StreamUtils::read(stream, &tk);
            TV tv;
            StreamUtils::read(stream, &tv);

            val->insert(std::make_pair(std::move(tk), std::move(tv)));
        }
    }
    
    template<typename CharT = char>
    std::basic_string<CharT> read_str(std::istream& stream) {
        std::basic_string<CharT> s;
        std::getline(stream, s, '\0');
        return s;
    }
    template<typename CharT = char>
    void read_str_buf(std::istream& stream, CharT* buf, size_t buf_len) {
        stream.getline(buf, buf_len, '\0');
    }
}

#endif