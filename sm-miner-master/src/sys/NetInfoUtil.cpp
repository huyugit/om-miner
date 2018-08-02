/*
 * Defines a set of network utility functions.
 */

#include "NetInfoUtil.h"

#include "except/SystemException.h"

#include "sys/FileDescriptor.h"

#include <assert.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>


namespace util
{

const struct in_addr getIpAddress(const char* ifname)
{
    assert(ifname != nullptr);

    FileDescriptor sock(::socket(AF_INET, SOCK_DGRAM, 0));
    if (!sock)
    {
        static const in_addr noAddress = {};
        return noAddress;
    }

    struct ifreq ifr = {};
    ifr.ifr_addr.sa_family = AF_INET;
    ::strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    ::ioctl(sock.getDescriptor(), SIOCGIFADDR, &ifr);
    sock.close();

    return (reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr))->sin_addr;
}

const IpAddressStr ipAddressToString(const struct in_addr& ipAddress)
{
    return ::inet_ntoa(ipAddress);
}

const MacAddress getMacAddress(const char* ifname)
{
    assert(ifname != nullptr);
    
    MacAddress macAddress = {};
    
    FileDescriptor sock(::socket(AF_INET, SOCK_DGRAM, 0));
    if (!sock)
        return macAddress;
    
    struct ifreq ifr = {};
    ifr.ifr_addr.sa_family = AF_INET;
    ::strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    
    ::ioctl(sock.getDescriptor(), SIOCGIFHWADDR, &ifr);
    sock.close();
    
    ::memcpy(macAddress.data, ifr.ifr_hwaddr.sa_data, sizeof(macAddress.data));
    return macAddress;
}

const MacAddressStr macAddressToString(const MacAddress& macAddress)
{
    MacAddressStr str;
    
    for (size_t i = 0; i < sizeof(macAddress.data); ++i)
    {
        if (!str.isEmpty())
            str.print(":");
    
        str.printf("%02x", macAddress.data[i]);
    }
    
    return str;
}

}  // end of namespace util
