#include "GlobalDefine.h"
#include <string.h>

ESkinMode getSkinModeFromStr(const char* pName)
{
    for (unsigned int x = 0; x < ESkinModeCount; x++)
    {
        if (strcmp(ESkinModeString[x], pName) == 0)
        {
            return static_cast<ESkinMode>(x);
        }
    }
    return ESkinMode::Absolute;
}
