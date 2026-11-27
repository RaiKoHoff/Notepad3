#define CINTERFACE
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#ifdef __cplusplus
extern "C" {
#endif
DWORD_PTR WINAPI TestDlapiGetFileInfo(LPCWSTR path, DWORD attributes, SHFILEINFOW* info, UINT size, UINT flags);
HRESULT WINAPI TestDlapiBindToParent(PCIDLIST_ABSOLUTE pidl, REFIID iid, void** parent, PCUITEMID_CHILD* child);
BOOL WINAPI TestDlapiPostMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#ifdef __cplusplus
}
#endif

#define SHGetFileInfoW TestDlapiGetFileInfo
#define SHBindToParent TestDlapiBindToParent
#undef PostMessage
#define PostMessage TestDlapiPostMessage
