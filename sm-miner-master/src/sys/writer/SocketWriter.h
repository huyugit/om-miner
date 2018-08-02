#ifndef SOCKETWRITER_H
#define SOCKETWRITER_H

#include "sys/writer/Writer.h"

class SocketWriter
        : public Writer
{
public:
    SocketWriter(int _fd);

    void printf(const char* format, ...);

private:
    int fd;
};

#endif // SOCKETWRITER_H
