#include "SingleInstanceApp.h"

#include <sys/file.h>
#include <errno.h>

#include "except/SystemException.h"
#include "sys/FileDescriptor.h"


void SingleInstanceApp::lock(const char* fileName)
{
    int fd = ::open(fileName, O_CREAT | O_RDWR, 0x0666);
    if (fd < 0) {
        throw SystemException(errno, "Cannot open pid file: %s", fileName);
    }

    int rc = flock(fd, LOCK_EX | LOCK_NB);
    if (rc)
    {
        const char* reason = "unexpected error";

        if (EWOULDBLOCK == errno)
        {
            reason = "another instance is running";
        }

        throw SystemException(errno, "Cannot lock pid file: %s (%s)",
                              fileName, reason);
    }
    else {
        // no file lock yet
    }

    ::printf("Single instance file locked: %s\n", fileName);
}
