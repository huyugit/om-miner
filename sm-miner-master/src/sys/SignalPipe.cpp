/*
 * Contains SignalPipe class definition.
 */

#include "SignalPipe.h"

#include <sys/socket.h>


void SignalPipe::signal()
{
    // Notification character sent through the signal pipe.
    // Doesn't really matter what the actual value is.
    static const char c_threadNotifyCommand = '!';

    // Send the notification character to the other pipe endpoint.
    m_socketPair.getSock1().write(&c_threadNotifyCommand, 1);
}

void SignalPipe::consumeNotificationData()
{
    // To consume all data from the control pipe buffer, recv() function is used
    // instead of read(), since the latter blocks if there is no data available.

    char command = 0;
    while (::recv(getDescriptor(), &command, 1, MSG_DONTWAIT) > 0)
    {
    }
}
