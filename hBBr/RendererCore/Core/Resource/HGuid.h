#pragma once

#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif

struct HUUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];

    bool operator==(const HUUID& id) const {
        return id.Data1 == Data1 && id.Data2 == Data2 && id.Data3 == Data3
            && id.Data4[0] == Data4[0] && id.Data4[1] == Data4[1] && id.Data4[2] == Data4[2] && id.Data4[3] == Data4[3]
            && id.Data4[4] == Data4[4] && id.Data4[5] == Data4[5] && id.Data4[6] == Data4[6] && id.Data4[7] == Data4[7];
    }
};

HUUID CreateUUID();

inline std::string UUIDToString(const HUUID& guid)
{
    char buf[64] = { 0 };
#ifdef __GNUC__
    snprintf(
#else // MSVC
    _snprintf_s(
#endif
        buf,
        sizeof(buf),
        "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
    return std::string(buf);
}

inline bool StringToUUID(const char* guidString, UUID* guid)
{
    return sscanf_s(guidString,
        "{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}",
        &guid->Data1, &guid->Data2, &guid->Data3,
        &guid->Data4[0], &guid->Data4[1], &guid->Data4[2], &guid->Data4[3],
        &guid->Data4[4], &guid->Data4[5], &guid->Data4[6], &guid->Data4[7]) == 11;
}

inline std::string CreateUUIDString()
{
    return UUIDToString(CreateUUID());
}

namespace std
{
    struct UUIDEqualKey
    {
        bool operator() (const HUUID& s, const HUUID& q) const noexcept
        {
            return s == q;
        }
    };
    template<>
    struct hash<HUUID>
    {
        size_t operator() (const HUUID& s) const noexcept
        {      
            std::hash<std::string> hasher;
            return hasher(UUIDToString(s));
        }
    };
};
