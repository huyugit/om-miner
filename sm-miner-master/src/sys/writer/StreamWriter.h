#ifndef STREAMWRITER_H
#define STREAMWRITER_H

#include <cstdio>
#include "sys/writer/Writer.h"

class StreamWriter
        : public Writer
{
public:
    StreamWriter(FILE *_stream);

    void printf(const char* format, ...);

private:
    FILE *stream;
};

#endif // STREAMWRITER_H
