#include "IWNet.hpp"

#include <utils/memory/memory.hpp>

//DONE : 0x0043BC70
bool IWNet_DNSResolved()
{
    return dnsLookupSucceeded;
}
