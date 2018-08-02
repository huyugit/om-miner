/*******************************************************************************
*  file    : array.hpp
*  created : 11.01.2013
*  author  : Slyshyk Oleksiy (alex312@meta.ua)
*******************************************************************************/

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstdint>

template<class T, uint32_t SIZE = 256>
class Array
{
public:
    enum {max_size = SIZE};
    typedef T  value_type;
    typedef T& reference;
    typedef const T const_value;
    typedef const T& const_ref;
public:
    Array() : size_(0) {}
    inline reference  operator[](int idx)       {return buf_[idx];}
    inline const_ref  operator[](int idx) const {return buf_[idx];}
    inline value_type at        (int idx) const {return buf_[idx];}
    inline uint32_t   capacity  (void) const {return SIZE;}
    inline uint32_t   size      ()     const {return size_;}
    const T*          data      (void)       {return &buf_[0];}
    void              init      (const T& v)
    {
        for(int i = 0; i < SIZE; ++i)
            buf_[i] = v;
    }
    void push_back(const T& value) {if(size_ < capacity()) buf_[size_++] = value; }
    inline reference back()        {return buf_[size_ - 1];}
    inline const_ref back()  const {return buf_[size_ - 1];}
    inline reference front()       {return buf_[0];}
    inline const_ref front() const {return buf_[0];}
    void             clear()       {size_ = 0;}
    uint32_t         erase(uint32_t pos);
    uint32_t         erase(uint32_t from, uint32_t to);

private:
    T buf_[SIZE];
    uint32_t size_;
};

template<class T, uint32_t SIZE>
uint32_t Array<T,SIZE>::erase(uint32_t pos)
{
    if(pos >= size_)
        return capacity();
    for(uint32_t i = pos+1; i < size_ ; ++i)
        buf_[i-1] = buf_[i];
    size_ --;
    return pos;
}
template<class T, uint32_t SIZE>
uint32_t Array<T,SIZE>::erase(uint32_t from, uint32_t to)
{
    if(to < from)
        return capacity();
    if(to > size_)
        return capacity();
    for(uint32_t i = from+1; i < size_ ; ++i)
        buf_[i-1] = buf_[i];
    size_ -= to-from ;

    return from;
}


template<class T, uint32_t SIZE = 256>
class RingBuffer
{
public:
    typedef T  value_type;
    typedef T& reference;
    typedef const T const_value;
    typedef const T& const_ref;
public:
    RingBuffer():data_idx_(0),data_cnt_(0){}

    void clear()
    {
        data_idx_ = 0;
        data_cnt_ = 0;
    }

    void push_back(const T& x)
    {
        buf_[data_idx_] = x;

        data_idx_ += 1;
        if(data_idx_ == SIZE)
            data_idx_ = 0;
        if(data_cnt_ < SIZE)
            data_cnt_ += 1;
    }
    reference front()
    {
        if(data_idx_)
            return buf_[data_idx_ - 1];
        else
            return buf_[SIZE - 1];
    }
    reference back ()
    {
        if(data_idx_ >= data_cnt_)
            return buf_[data_idx_ - data_cnt_];
        else
            return buf_[SIZE - data_cnt_ + data_idx_];
    }
    T pop_front()
    {
        if( !is_empty() )
            {
                data_cnt_--;
                if(data_idx_)
                    data_idx_--;
                else
                    data_idx_ = SIZE - 1;
            }
        return buf_[data_idx_];
    }
    T pop_back()
    {
        T res = back();
        if( !is_empty() )
            data_cnt_--;
        return res;
    }

    reference  operator[](uint32_t idx)       ;
    const_ref  operator[](uint32_t idx) const ;

    const T* data()
    {
        return &buf_[0];
    }

    inline bool is_empty() const {return (data_cnt_ == 0);}
    inline bool is_full () const {return (data_cnt_ == SIZE);}
    inline int  capasity() const {return SIZE;}
    inline uint32_t  size    () const {return data_cnt_;}
    inline int  data_idx() const {return data_idx_;}
private:
    T buf_[SIZE];
    uint32_t data_idx_;
    uint32_t data_cnt_;
};

template<class T, uint32_t SIZE>
typename RingBuffer<T,SIZE>::reference RingBuffer<T,SIZE>::operator[](uint32_t idx)
{
    uint32_t real_idx ;
    if(data_idx_ >= data_cnt_)
        real_idx = data_idx_ - data_cnt_;
    else
        real_idx = SIZE - data_cnt_ + data_idx_;
    for(uint32_t i = 0; i < idx; ++i)
        {
            real_idx++;
            if(real_idx == SIZE)
                real_idx = 0;
        }
    return this->buf_[real_idx];
}
template<class T, uint32_t SIZE>
typename RingBuffer<T,SIZE>::const_ref RingBuffer<T,SIZE>::operator[](uint32_t idx) const
{
    uint32_t real_idx ;
    if(data_idx_ >= data_cnt_)
        real_idx = data_idx_ - data_cnt_;
    else
        real_idx = SIZE - data_cnt_ + data_idx_;
    for(int i = 0; i < idx; ++i)
        {
            real_idx++;
            if(real_idx == SIZE)
                real_idx = 0;
        }
    return this->buf_[real_idx];
}



template<class T, uint32_t DEFAULT_SIZE>
class RingBufferDyn
{
public:
    typedef T  value_type;
    typedef T& reference;
    typedef const T const_value;
    typedef const T& const_ref;
public:
    RingBufferDyn()
        : buf_ptr(buf_default),
          SIZE(sizeof(buf_default) / sizeof(buf_default[0])),
          data_idx_(0), data_cnt_(0)
    {}

    void setBuffer(T *ptr, uint32_t size)
    {
        clear();
        SIZE = size / sizeof(T);
        buf_ptr = ptr;
    }

    void clear()
    {
        data_idx_ = 0;
        data_cnt_ = 0;
    }

    void push_back(const T& x)
    {
        buf_ptr[data_idx_] = x;

        data_idx_ += 1;
        if(data_idx_ == SIZE)
            data_idx_ = 0;
        if(data_cnt_ < SIZE)
            data_cnt_ += 1;
    }
    reference front()
    {
        if(data_idx_)
            return buf_ptr[data_idx_ - 1];
        else
            return buf_ptr[SIZE - 1];
    }
    reference back ()
    {
        if(data_idx_ >= data_cnt_)
            return buf_ptr[data_idx_ - data_cnt_];
        else
            return buf_ptr[SIZE - data_cnt_ + data_idx_];
    }
    T pop_front()
    {
        if( !is_empty() )
            {
                data_cnt_--;
                if(data_idx_)
                    data_idx_--;
                else
                    data_idx_ = SIZE - 1;
            }
        return buf_ptr[data_idx_];
    }
    T pop_back()
    {
        T res = back();
        if( !is_empty() )
            data_cnt_--;
        return res;
    }

    reference operator[](uint32_t idx)
    {
        uint32_t real_idx ;
        if(data_idx_ >= data_cnt_)
            real_idx = data_idx_ - data_cnt_;
        else
            real_idx = SIZE - data_cnt_ + data_idx_;
        for(uint32_t i = 0; i < idx; ++i)
            {
                real_idx++;
                if(real_idx == SIZE)
                    real_idx = 0;
            }
        return this->buf_ptr[real_idx];
    }

    const T* data()
    {
        return &buf_ptr[0];
    }

    inline bool is_empty() const {return (data_cnt_ == 0);}
    inline bool is_full () const {return (data_cnt_ == SIZE);}
    inline int  capasity() const {return SIZE;}
    inline uint32_t  size    () const {return data_cnt_;}
    inline int  data_idx() const {return data_idx_;}
private:
    T buf_default[DEFAULT_SIZE];

    T* buf_ptr;
    uint32_t SIZE;

    uint32_t data_idx_;
    uint32_t data_cnt_;
};

#endif // ARRAY_HPP
