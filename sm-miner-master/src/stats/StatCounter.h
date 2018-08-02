#ifndef STATCOUNTER_H
#define STATCOUNTER_H

#include <stdint.h>


template <class T>
class StatCounterT
{
public:
    StatCounterT()
        : value(0), valuePrev(0)
    {}

    inline operator T() const {
        return value;
    }
    inline T& operator=(const T& v) {
        value = v;
        return value;
    }

    inline T get(bool useTotal = true) const {
        return useTotal ? value : getDiff();
    }

    inline T getPrev() const {
        return valuePrev;
    }

    inline T getDiff() const {
        return value - valuePrev;
    }

    inline void save() {
        valuePrev = value;
    }

    inline void clear() {
        value = 0;
        valuePrev = 0;
    }

    inline StatCounterT<T>& operator+=(const T& v) {
        value += v;
        return *this;
    }
    inline StatCounterT<T>& operator+=(const StatCounterT<T>& v) {
        value += v.get();
        valuePrev += v.getPrev();
        return *this;
    }

    inline StatCounterT<T>& operator++() {
        value++;
        return *this;
    }
    inline StatCounterT<T> operator++(int) {
        StatCounterT<T> tmp(*this);
        operator++(); // pre-increment
        return tmp;
    }

private:
    T value, valuePrev;
};


typedef StatCounterT<uint32_t> StatCounter32;
typedef StatCounterT<uint64_t> StatCounter64;


#endif // STATCOUNTER_H
