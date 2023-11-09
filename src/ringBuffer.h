#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <vector>
#include <iterator>
#include <stdexcept>

#include "StringUtils.h"
#include "DataUtils.h"

template<typename T>
class RingBuffer{
private:
    std::vector<T> data;
    size_t ptr = 0;
    size_t len = 0;
public:
    template <typename IT, typename VT>
    class Iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = VT;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        IT& rb;
        size_t ind;
    public:

        Iterator(IT& rb, size_t ind) : rb(rb), ind(ind) {

        }

        reference operator*() const {
            return rb.get(ind);
        }
        pointer operator->() {
            return &rb.get(ind);
        }
        reference operator[](difference_type d) {
            return rb.get(ind + d);
        }

        Iterator& operator++() {
            ind++;
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp = *this; 
            ++(*this); 
            return tmp; 
        }
        Iterator& operator--() {
            ind--;
            return *this;
        }
        Iterator operator--(int) {
            Iterator tmp = *this; 
            --(*this); 
            return tmp; 
        }
        Iterator& operator+=(difference_type d) {
            ind += d;
            return *this;
        }
        Iterator& operator-=(difference_type d) {
            ind -= d;
            return *this;
        }

        friend Iterator operator+(const Iterator& it, difference_type d) {
            return Iterator(it) += d;
        }
        friend Iterator operator+(difference_type d, const Iterator& it) {
            return Iterator(it) += d;
        }
        friend Iterator operator-(const Iterator& it, difference_type d) {
            return Iterator(it) -= d;
        }
        friend Iterator operator-(difference_type d, const Iterator& it) {
            return Iterator(it) -= d;
        }

        bool operator== (const Iterator& b) const {
            return rb == b.rb && ind == b.ind;
        }
        bool operator!= (const Iterator& b) const {
            return !(*this == b);
        }
    };

    RingBuffer(size_t size_) : data(size_) {

    }

    void initTo(const T& val) {
        for(size_t i = 0; i<data.size();i++){
            data[i] = val;
        }
    }

    T& get(size_t ind) {
        if(ind > len)
            throw std::runtime_error("ringbuffer index out of bounds");
        size_t ind_ = (ptr+data.size()-len+ind)%data.size();
        return data[ind_];
    }
    const T& get(size_t ind) const {
        if(ind > len)
            throw std::runtime_error("ringbuffer index out of bounds");
        size_t ind_ = (ptr+data.size()-len+ind)%data.size();
        return data[ind_];
    }

    void add(const T& t) {
        if(ptr >= data.size())
            throw std::runtime_error("ptr out of bounds");
        data[ptr++] = t;
        ptr %= data.size();
        len = std::min(len+1,data.size());
    }

    void pop_front(size_t amt = 1) {
        if(amt > size()) 
            throw std::runtime_error(StringUtils::format("requested to many pop_front %" DU_PRIuSIZE "/%" DU_PRIuSIZE, amt, size()));
        len -= amt;
    }

    size_t size() const {
        return len;
    }
    size_t sizeMax() const {
        return data.size();
    }

    void clear() {
        len = 0;
        ptr = 0;
    }

    T& last() {
        return get(size() - 1);
    }

    bool operator==(const RingBuffer& other) const {
        if (len != other.len || data.size() != other.data.size())
            return false;
        for (size_t i = 0; i < len; i++) {
            if (!(get(i) == other.get(i)))
                return false;
        }
        return true;
    }

    bool operator!=(const RingBuffer& other) const {
        return !(*this) == other;
    }

    Iterator<RingBuffer,T> begin() {
        return Iterator<RingBuffer,T>(*this, 0);
    }
    Iterator<RingBuffer,T> end() {
        return Iterator<RingBuffer,T>(*this, len);
    }
    Iterator<const RingBuffer,const T> begin() const {
        return Iterator<const RingBuffer, const T>(*this, 0);
    }
    Iterator<const RingBuffer,const T> end() const {
        return Iterator<const RingBuffer, const T>(*this, len);
    }
};

#endif