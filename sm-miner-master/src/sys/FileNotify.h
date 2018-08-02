#ifndef FILE_NOTIFY_H
#define FILE_NOTIFY_H
/*
 * Contains FileNotify class declaration.
 */

#include "base/NonCopyable.h"
#include "base/StringBuffer.h"
#include "except/SystemException.h"
#include "sys/FileDescriptor.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#include <sys/inotify.h>


// Class to process file system notifications watching
// for a given file write/delete events.
//
class FileNotify
    : private NonCopyable  // Prevent copy and assignment.
{
// Public type declarations.
public:
    // Notification event types.
    struct NotifyEvent
    {
        enum Type
        {
            NOTHING = 0,        // No event happened.
            FILE_CHANGED,       // Watched file has been changed.
            FILE_DELETED        // Watched file has been deleted.
        };
    };

// Construction.
public:
    // Constructs the object.
    explicit FileNotify(const char* filePath)
        : m_notify()
        , m_fileName()
        , m_fileDir()
        , m_watchDescriptor(-1)
    {
        assert(filePath != nullptr);
        
        m_notify.attach(::inotify_init1(IN_NONBLOCK));
        if (!m_notify)
            throw SystemException(errno, "inotify_init1() failed");
        
        char tmpPath[PATH_MAX];
        ::strncpy(tmpPath, filePath, sizeof(tmpPath));
        tmpPath[sizeof(tmpPath) - 1] = 0;
        
        m_fileName = ::basename(tmpPath);
        m_fileDir = ::dirname(tmpPath);
        
        m_watchDescriptor = ::inotify_add_watch(m_notify.getDescriptor(), m_fileDir.cdata(),
            IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF);
        if (m_watchDescriptor < 0)
            throw SystemException(errno, "Unable to watch \"%s\" (the directory may not exist)", m_fileDir.cdata());
    }
    
// Public interface.
public:
    // Returns raw inotify instance descriptor.
    inline int getDescriptor()  { return m_notify.getDescriptor(); }
    
    // Returns name/directory of the file to observe.
    const String getFileName() const throw()  { return m_fileName.str(); }
    const String getFileDir()  const throw()  { return m_fileDir.str();  }
    
    // Read notification events from queue and return them combined
    // into a single value.
    NotifyEvent::Type readEvents()
    {
        // Some systems cannot read integer variables if they are not
        // properly aligned. On other systems, incorrect alignment may
        // decrease performance. Hence, the buffer used for reading from
        // the inotify file descriptor should have the same alignment as
        // struct inotify_event.

        static const size_t c_bufSize = 4096;
        char buf[c_bufSize] __attribute__((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event = nullptr;

        NotifyEvent::Type notifyEvent = NotifyEvent::NOTHING;

        for (;;)
        {
            const ssize_t result = m_notify.read(buf, sizeof(buf));
            if (result < 0 && errno != EAGAIN)
                throw SystemException(errno, "Error reading data");

            // If the nonblocking read() found no events to read, then
            // it returns -1 with errno set to EAGAIN. In that case,
            // exit the loop.
            if (result <= 0)
                break;

            const size_t len = static_cast<size_t>(result);
            assert(len <= sizeof(buf));

            // Loop over all events in the buffer.
            // As a result, notifyEvent variable will contain the most recent event 
            for (const char* ptr = buf; ptr < buf + len;
                ptr += sizeof(struct inotify_event) + event->len)
            {
                event = reinterpret_cast<const struct inotify_event*>(ptr);
                if (event->wd != m_watchDescriptor)
                    continue;

                if (event->mask & IN_DELETE_SELF)
                    throw SystemException("Watched directory has been deleted.");
                else if (event->mask & IN_ISDIR)
                    continue;  // Not interested in directory events.
                else if (event->len == 0 || m_fileName.str() != event->name)
                    continue;  // Not related to the watched file.

                if (event->mask & IN_CLOSE_WRITE)
                    notifyEvent = NotifyEvent::FILE_CHANGED;
                if (event->mask & IN_DELETE)
                    notifyEvent = NotifyEvent::FILE_DELETED;
            }
        }
        
        return notifyEvent;
    }

// Member variables.
private:
    // Holds file descriptor associated with inotify event queue.
    FileDescriptor m_notify;
    
    // A name of file to observe.
    StringBuffer<256> m_fileName;
    
    // A directory of file to observe.
    StringBuffer<PATH_MAX> m_fileDir;
    
    // Inotify watch descriptor.
    int m_watchDescriptor;
};

#endif  // FILE_NOTIFY_H
