#include <shlobj.h>

extern "C" {
#include "PathLib.h"

bool TestKnownFolderCpp(HPATHL path)
{
    return Path_GetKnownFolder(FOLDERID_Profile, path);
}
}
