#ifndef NET_INFO_UTIL_H
#define NET_INFO_UTIL_H
/*
 * Declares a set of network utility functions.
 */

#include "base/StringBuffer.h"

#include <arpa/inet.h>


namespace util
{
    // Returns IP-address of the given interface name.
    const struct in_addr getIpAddress(const char* ifname = "eth0");
    
    // String buffer for IP-address textual representation.
    typedef StringBuffer<INET6_ADDRSTRLEN> IpAddressStr;
    
    // Returns IP-address of the given interface name.
    const IpAddressStr ipAddressToString(const struct in_addr& ipAddress);
    
    // Struct holding MAC address data.
    struct MacAddress
    {
        uint8_t data[6];
    };
    
    // Returns MAC-address of the given interface name.
    const MacAddress getMacAddress(const char* ifname = "eth0");
    
    // String buffer for MAC-address textual representation.
    typedef StringBuffer<18> MacAddressStr;
    
    const MacAddressStr macAddressToString(const MacAddress& macAddress);
};

#endif  // NET_INFO_UTIL_H
