#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <vector>
#include <iterator>

template<typename T>
class RingBuffer{
private:
    std::vector<T> data;
    size_t ptr = 0;
    size_t len = 0;
public:
    class Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        RingBuffer& rb;
        size_t ind;
    public:

        Iterator(RingBuffer& rb, size_t ind) : rb(rb), ind(ind) {

        }

        reference operator*() const {
            return rb.get(ind);
        }
        pointer operator->() {
            return &rb.get(ind);
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

        bool operator== (const Iterator& b) {
            return rb == b.rb && ind == b.ind;
        }
        bool operator!= (const Iterator& b) {
            return !(*this == b);
        }
    };

    RingBuffer(size_t size_) : data(size_){

    }

    void initTo(const T& val){
        for(size_t i = 0; i<data.size();i++){
            data[i] = val;
        }
    }

    T& get(size_t ind){
        if(ind > len)
            throw std::runtime_error("ringbuffer index out of bounds");
        size_t ind_ = (ptr+data.size()-len+ind)%data.size();
        return data[ind_];
    }
    const T& get(size_t ind) const{
        if(ind > len)
            throw std::runtime_error("ringbuffer index out of bounds");
        size_t ind_ = (ptr+data.size()-len+ind)%data.size();
        return data[ind_];
    }

    void add(const T& t){
        if(ptr >= data.size())
            throw std::runtime_error("ptr out of bounds");
        data[ptr++] = t;
        ptr %= data.size();
        len = std::min(len+1,data.size());
    }

    size_t size(){
        return len;
    }
    size_t sizeMax(){
        return data.size();
    }

    void clear() {
        len = 0;
        ptr = 0;
    }

    T& last() {
        return get(size() - 1);
    }

    bool operator==(const RingBuffer& other) {
        if (len != other.len || data.size() != other.data.size())
            return false;
        for (size_t i = 0; i < len; i++) {
            if (!(get(i) == other.get(i)))
                return false;
        }
        return true;
    }

    Iterator begin() {
        return Iterator(*this, 0);
    }
    Iterator end() {
        return Iterator(*this, len);
    }
};

#endif