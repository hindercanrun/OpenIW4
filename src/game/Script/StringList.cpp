#include "game/defs.hpp"
#include "StringList.hpp"
#include "../Com/Com.hpp"

#include "utils/memory/memory.hpp"

//THUNK : 0x00455AD0
uint32_t SL_GetStringOfSize(const char* str, uint32_t user, uint32_t len, int type)
{
    return memory::call<uint32_t(const char*, uint32_t, uint32_t, int)>(0x455AD0)(str, user, len, type);
}

//DONE : 0x004A2EA0
uint32_t SL_GetString_(const char* str, uint32_t user, int type)
{
    const int len = I_strlen(str);
    return SL_GetStringOfSize(str, user, len + 1, type);
}

//DONE : Inlined
RefString* GetRefString(uint32_t stringValue)
{
    uint32_t pos = 0xC * stringValue;
    return reinterpret_cast<RefString*>(&scrMemTreePub->mt_buffer[pos]);
}

//DONE : 0x0040E990
char* SL_ConvertToString(uint32_t stringValue)
{
    if (!stringValue)
        return NULL;

    return GetRefString(stringValue)->str;
}

//Inlined
//Moved from Memory.cpp
const char* CopyString(const char* in)
{
    //CopyString takes 2 more arguments, but they are never used for some reason
    uint32_t stringValue = SL_GetString_(in, 0, 22);
    return SL_ConvertToString(stringValue);
}
