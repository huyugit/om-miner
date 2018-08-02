#ifndef DATA_NOISE_FILTER_H
#define DATA_NOISE_FILTER_H

#include <stddef.h>


template<class T, size_t SIZE = 5>
class DataNoiseFilter
{
public:
    DataNoiseFilter()
        : pos(0)
    {
        for (size_t i = 0; i < SIZE; i++)
            buffer[i] = T();
    }

    size_t getSize() const { return SIZE; }

    void push(T value)
    {
        buffer[pos] = value;
        pos++;
        if (pos >= SIZE) pos = 0;
    }

    T getAtMiddle()
    {
        memcpy(buf, buffer, sizeof(buffer));

        for (size_t i = 0; i < SIZE-1; i++)
            for (size_t j = i+1; j < SIZE; j++)
                if (buf[i] > buf[j]) {
                    T x=buf[i]; buf[i]=buf[j]; buf[j]=x;
                }

        return buf[SIZE / 2 + 1];
    }

private:
    T buffer[SIZE], buf[SIZE];
    size_t pos;
};

#endif // DATA_NOISE_FILTER_H
