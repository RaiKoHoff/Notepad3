#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define SI_SUPPORT_IOSTREAMS
#include <windows.h>
#include "SimpleIni.h"

#include <cassert>
#include <climits>
#include <sstream>

static int failNthArrayAllocation;

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
    if (failNthArrayAllocation && --failNthArrayAllocation == 0) {
        return nullptr;
    }
    try {
        return ::operator new[](size);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

class RejectingConverter : public SI_ConvertW<wchar_t> {
public:
    using SI_ConvertW<wchar_t>::SI_ConvertW;
    bool ConvertToStore(const wchar_t*, char*, size_t) const { return false; }
};

int wmain(int argc, wchar_t* argv[])
{
    CSimpleIniW ini(true, false, true);
    std::string input = "; file comment\r\n\r\n; section comment\r\n[s]\r\n; key comment\r\n"
        "number=42 ;" + std::string(200, 'a') +
        "\r\nhex=0x2a#note\r\nwide=4294967296 ;note\r\nfloat=1.5 #note\r\n"
        "bad=42x\r\nflag=2\r\noverflow=9999999999999999999999999\r\n"
        "[t]\r\nkey=value\r\n";
    assert(ini.LoadData(input.data(), input.size()) == SI_Error::SI_OK);
    assert(ini.GetLongValue(L"s", L"number", -1) == 42);
    assert(ini.GetLongValue(L"s", L"hex", -1) == 42);
    assert(ini.GetLongValue(L"s", L"bad", -1) == -1);
    assert(ini.GetLongLongValue(L"s", L"wide", -1) == 4294967296LL);
    assert(ini.GetDoubleValue(L"s", L"float", -1) == 1.5);
    assert(ini.GetBoolValue(L"s", L"flag", false));
    assert(ini.SetLongLongValue(L"s", L"wide", LLONG_MIN) == SI_Error::SI_UPDATED);
    assert(ini.GetLongLongValue(L"s", L"wide", 0) == LLONG_MIN);
    assert(ini.GetLongValue(L"s", L"overflow", -1) == -1);
    assert(ini.GetLongLongValue(L"s", L"overflow", -1) == -1);
    ini.SetValue(L"s", L"nondigit", L"# only a comment");
    assert(ini.GetLongLongValue(L"s", L"nondigit", -1) == -1);
    assert(ini.GetDoubleValue(L"s", L"nondigit", -1) == -1);

    std::string saved;
    assert(ini.Save(saved, true) == SI_Error::SI_OK);
    assert(saved.starts_with("\xEF\xBB\xBF; file comment\r\n\r\n; section comment\r\n[s]\r\n; key comment\r\n"));
    assert(saved.ends_with("\r\n[t]\r\nkey = value\r\n"));
    assert(saved.find("\r\n\r\n[t]") == std::string::npos);

    for (bool bigEndian : {false, true}) {
        std::string encoded = bigEndian ? "\xFE\xFF" : "\xFF\xFE";
        for (wchar_t ch : std::wstring(L"[s]\nname=caf\u00e9 \U0001F600\n")) {
            encoded.push_back(static_cast<char>(bigEndian ? ch >> 8 : ch));
            encoded.push_back(static_cast<char>(bigEndian ? ch : ch >> 8));
        }
        std::string original = encoded;
        CSimpleIniW decoded;
        assert(decoded.LoadData(encoded.data(), encoded.size()) == SI_Error::SI_OK);
        assert(std::wstring(decoded.GetValue(L"s", L"name", L"")) == L"caf\u00e9 \U0001F600");
        assert(encoded == original);
        assert(decoded.LoadData("\xff\xfeX", 3) == SI_Error::SI_FAIL);
        std::istringstream stream(original);
        CSimpleIniW streamed;
        assert(streamed.LoadData(stream) == SI_Error::SI_OK);
        assert(std::wstring(streamed.GetValue(L"s", L"name", L"")) == L"caf\u00e9 \U0001F600");
    }

    assert(argc == 2);
    const std::wstring file = std::wstring(argv[1]) + L"\\settings.ini";
    HANDLE handle = CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
        nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    assert(handle != INVALID_HANDLE_VALUE);
    OVERLAPPED lock{};
    assert(LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &lock));
    LARGE_INTEGER size{};
    size.QuadPart = 4096;
    assert(SetFilePointerEx(handle, size, nullptr, FILE_BEGIN) && SetEndOfFile(handle));
    assert(ini.SaveFile(handle, true) == SI_Error::SI_OK);
    assert(GetFileSizeEx(handle, &size) && size.QuadPart == static_cast<LONGLONG>(saved.size()));
    CSimpleIniTempl<wchar_t, SI_NoCase<wchar_t>, RejectingConverter> cannotSave(true);
    assert(cannotSave.SetValue(L"s", L"k", L"v") == SI_Error::SI_INSERTED);
    assert(cannotSave.SaveFile(handle, true) == SI_Error::SI_FAIL);
    assert(GetFileSizeEx(handle, &size) && size.QuadPart == static_cast<LONGLONG>(saved.size()));
    assert(SetFilePointerEx(handle, {}, nullptr, FILE_BEGIN));
    CSimpleIniW loaded;
    assert(loaded.LoadFile(handle) == SI_Error::SI_OK);
    assert(loaded.GetLongLongValue(L"s", L"wide", 0) == LLONG_MIN);
    assert(UnlockFileEx(handle, 0, MAXDWORD, MAXDWORD, &lock));
    assert(CloseHandle(handle));

    handle = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(handle != INVALID_HANDLE_VALUE);
    assert(ini.SaveFile(handle) == SI_Error::SI_FILE);
    assert(CloseHandle(handle));
    assert(DeleteFileW(file.c_str()));

    const char withNul[] = "[s]\nx=1\0y=2";
    assert(loaded.LoadData(withNul, sizeof(withNul) - 1) == SI_Error::SI_FAIL);
    CSimpleIniW multiline(true, false, true);
    const wchar_t* text = L"one\nEND_OF_TEXT\ntwo";
    assert(multiline.SetValue(L"s", L"m", text) == SI_Error::SI_INSERTED);
    std::string serialized;
    assert(multiline.Save(serialized) == SI_Error::SI_OK);
    CSimpleIniW parsed(true, false, true);
    assert(parsed.LoadData(serialized.data(), serialized.size()) == SI_Error::SI_OK);
    assert(std::wstring(parsed.GetValue(L"s", L"m", L"")) == text);

    CSimpleIniW incremental(true);
    assert(incremental.LoadData("[s]\nx=old\ny=old") == SI_Error::SI_OK);
    failNthArrayAllocation = 3; // Parser buffer, then one value copy per update.
    assert(incremental.LoadData("[s]\nx=new\ny=new") == SI_Error::SI_NOMEM);
    assert(failNthArrayAllocation == 0);
    assert(std::wstring(incremental.GetValue(L"s", L"x")) == L"new");
    assert(std::wstring(incremental.GetValue(L"s", L"y")) == L"old");
}
