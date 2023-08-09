#include "HGuid.h"

HUUID CreateUUID()
{
    HUUID uuid;
#ifdef _WIN32
    GUID guid;
    CoCreateGuid(&guid);
    memcpy(&uuid, &guid, sizeof(HUUID));
#else
    uuid_generate(reinterpret_cast<unsigned char*>(&guid));
#endif
    return uuid;
}
