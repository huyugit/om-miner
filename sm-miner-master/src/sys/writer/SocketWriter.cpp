#include "SocketWriter.h"

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

#include "base/VarArgs.h"
#include "except/ApplicationException.h"


SocketWriter::SocketWriter(int _fd)
    : fd(_fd)
{}

void SocketWriter::printf(const char *format, ...)
{
    char buffer[1024];

    VarArgs arglist;
    VA_START(arglist, format);
    int len = ::vsnprintf(buffer, sizeof(buffer), format, arglist);

    if (len < 0) return;
    len = ::strlen(buffer);

    char *p = buffer;
    while (len > 0)
    {
        const int bytesSent = ::send(fd, p, len, MSG_EOR|MSG_NOSIGNAL);
        if (bytesSent <= 0)
        {
            throw ApplicationException("SocketWriter: write error %d!", bytesSent);
        }

        p += bytesSent;
        len -= bytesSent;
    }
}
