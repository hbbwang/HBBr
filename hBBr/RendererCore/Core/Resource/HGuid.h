#pragma once

#include "HString.h"
#include "ThirdParty/crossguid/Include/crossguid/guid.hpp"

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib ,"crossguid-dgb.lib")
#else
#pragma comment(lib ,"crossguid.lib")
#endif
#endif

#define HGUID xg::Guid

inline HGUID CreateGUID()
{
	return xg::newGuid();
}

inline bool IsGUIDValid(HGUID guid)
{
	return guid.isValid();
}

inline std::string GUIDToStdString(const HGUID& guid)
{
	return guid.str();
}

inline HString GUIDToString(const HGUID& guid)
{
	return guid.str().c_str();
}

inline bool StringToGUID(const char* guidString, HGUID* guid)
{
	*guid = HGUID(guidString);
	return guid->isValid();
}

inline HString CreateGUIDAndString(HGUID& emptyGuid)
{
	emptyGuid = xg::newGuid();
	return emptyGuid.str().c_str();
}

//#include <string>
//#include <stdio.h>
//
//#ifdef _WIN32
//#include <objbase.h>
//#else
//#include <uuid/uuid.h>
//#endif
//
//#include "HString.h"
//
//struct HGUID
//{
//    unsigned long Data1;
//    uint16_t Data2;
//    uint16_t Data3;
//    uint8_t Data4[8];
//
//    HGUID() {}
//
//    HGUID(const char* guidStr)
//    {
//        sscanf_s(guidStr,
//            "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
//            &Data1, &Data2, &Data3,
//            &Data4[0], &Data4[1], &Data4[2], &Data4[3],
//            &Data4[4], &Data4[5], &Data4[6], &Data4[7]);
//    }
//
//    bool operator==(const HGUID& id) const {
//        return id.Data1 == Data1 && id.Data2 == Data2 && id.Data3 == Data3
//            && id.Data4[0] == Data4[0] && id.Data4[1] == Data4[1] && id.Data4[2] == Data4[2] && id.Data4[3] == Data4[3]
//            && id.Data4[4] == Data4[4] && id.Data4[5] == Data4[5] && id.Data4[6] == Data4[6] && id.Data4[7] == Data4[7];
//    }
//};
//
//HGUID CreateGUID();
//
//inline HString GUIDToString(const HGUID& guid)
//{
//    char buf[64] = { 0 };
//#ifdef __GNUC__
//    snprintf(
//#else // MSVC
//    _snprintf_s(
//#endif
//        buf,
//        sizeof(buf),
//        "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
//        guid.Data1, guid.Data2, guid.Data3,
//        guid.Data4[0], guid.Data4[1],
//        guid.Data4[2], guid.Data4[3],
//        guid.Data4[4], guid.Data4[5],
//        guid.Data4[6], guid.Data4[7]);
//    return std::string(buf).c_str();
//}
//
//inline bool StringToGUID(const char* guidString, HGUID* guid)
//{
//    return sscanf_s(guidString,
//        "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
//        &guid->Data1, &guid->Data2, &guid->Data3,
//        &guid->Data4[0], &guid->Data4[1], &guid->Data4[2], &guid->Data4[3],
//        &guid->Data4[4], &guid->Data4[5], &guid->Data4[6], &guid->Data4[7]) == 11;
//}
//
//inline HString CreateGUIDString()
//{
//    return GUIDToString(CreateGUID());
//}
//
//namespace std
//{
//    struct GUIDEqualKey
//    {
//        bool operator() (const HGUID& s, const HGUID& q) const noexcept
//        {
//            return s == q;
//        }
//    };
//    template<>
//    struct hash<HGUID>
//    {
//        size_t operator() (const HGUID& s) const noexcept
//        {      
//            std::hash<std::string> hasher;
//            return hasher(GUIDToString(s).c_str());
//        }
//    };
//};
