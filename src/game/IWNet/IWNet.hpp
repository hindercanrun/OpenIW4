#pragma once

#include "../stdafx.hpp"

static bool dnsLookupSucceeded = *reinterpret_cast<bool*>(0x1BEAA1C);

bool IWNet_DNSResolved();
