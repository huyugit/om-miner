#ifndef WRITER_H
#define WRITER_H

class Writer
{
public:
    virtual void printf(const char* format, ...) = 0;
};

#endif // WRITER_H
