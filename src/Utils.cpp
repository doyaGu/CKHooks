#include "Utils.h"

#include <direct.h>
#include <sys/stat.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <strsafe.h>
#include <Psapi.h>

#include "XArray.h"

namespace utils {
    XString GetCurrentDir() {
        char buf[MAX_PATH];
        getcwd(buf, MAX_PATH);
        return buf;
    }

    XString RemoveDirectoryName(const XString &path) {
        const char *const lastSep = FindLastPathSeparator(path);
        return lastSep ? XString(lastSep + 1) : path;
    }

    XString RemoveFileName(const XString &path) {
        const char *const lastSep = FindLastPathSeparator(path);
        if (!lastSep) return ".\\";
        return XString(path.CStr(), (lastSep + 1 - path.CStr()));
    }

    XString RemoveExtension(const XString &path, const XString &ext) {
        if (ext == "*") {
            if (path.RFind('.') != XString::NOTFOUND)
                return path.Substring(0, path.RFind('.'));
        } else {
            const XString de = XString(".") + ext;
            if (StringIEndsWith(path, de)) {
                return path.Substring(0, path.Length() - de.Length());
            }
        }
        return path;
    }

    XString JoinPaths(const XString &path1, const XString &path2) {
        if (path1.Length() == 0) return path2;
        return RemoveTrailingPathSeparator(path1) + "\\" + path2;
    }

    XString MakeFileName(const XString &dir, const XString &name, const XString &ext) {
        return JoinPaths(dir, name + "." + ext);
    }

    bool IsFileExist(const XString &file) {
        if (file.Length() != 0) {
            struct stat fstat = {0};
            memset(&fstat, 0, sizeof(struct stat));
            return stat(file.CStr(), &fstat) == 0 && (fstat.st_mode & S_IFREG);
        }
        return false;
    }

    bool IsDirectoryExist(const XString &dir) {
        if (dir.Length() != 0) {
            struct stat fstat = {0};
            memset(&fstat, 0, sizeof(struct stat));
            return stat(dir.CStr(), &fstat) == 0 && (fstat.st_mode & S_IFDIR);
        }
        return false;
    }

    bool IsAbsolutePath(const XString &path) {
        if (path.Length() == 0) return false;
        if (path.Length() < 2 || !isalpha(path[0]) || path[1] != ':')
            return false;
        return true;
    }

    XString GetAbsolutePath(const XString &path, bool trailing) {
        if (path.Length() == 0 || IsAbsolutePath(path))
            return path;

        XArray<char> buf(MAX_PATH);
        ::GetCurrentDirectoryA(MAX_PATH, buf.Begin());
        XString absPath(buf.Begin());
        absPath << '\\';
        absPath << path;

        if (HasTrailingPathSeparator(absPath) && !trailing) {
            return absPath.Substring(0, absPath.Length() - 1);
        } else if (!HasTrailingPathSeparator(absPath) && trailing) {
            return absPath << '\\';
        } else {
            return absPath;
        }
    }

    bool CreateDir(const XString &dir) {
        int ret = _mkdir(dir.CStr());
        if (ret == -1)
            return IsDirectoryExist(dir);
        return true;
    }

    bool RemoveDir(const XString &dir) {
        if (IsDirectoryExist(dir)) {
            return ::RemoveDirectoryA(dir.CStr()) == TRUE;
        }
        return false;
    }

    const char *FindLastPathSeparator(const XString &path) {
        const char *const lastSep = strrchr(path.CStr(), '\\');
        const char *const lastAltSep = strrchr(path.CStr(), '/');
        return (lastAltSep && (!lastSep || lastAltSep > lastSep)) ? lastAltSep : lastSep;
    }

    bool HasTrailingPathSeparator(const XString &path) {
        return path.Length() != 0 && StringEndsWith(path, "\\");
    }

    XString RemoveTrailingPathSeparator(const XString &path) {
        return HasTrailingPathSeparator(path) ? path.Substring(0, path.Length() - 1) : path;
    }

    void NormalizePath(XString &path) {
        char *pch = path.Str();
        for (int i = 0; i < path.Length(); ++i) {
            char ch = path[i];
            if (!(ch == '\\' || ch == '/')) {
                *(pch++) = ch;
            } else if (pch == path.Str() || *(pch - 1) != '\\') {
                *(pch++) = '\\';
            }
        }
        path.Resize(pch - path.Str());
    }

    void OutputDebugA(const char *format, ...) {
        char buf[4096] = {0};

        va_list args;
        va_start(args, format);
        ::StringCchVPrintfA(buf, ARRAYSIZE(buf) - 1, format, args);
        va_end(args);

        buf[ARRAYSIZE(buf) - 1] = '\0';
        ::OutputDebugStringA(buf);
    }

    void OutputDebugW(const wchar_t *format, ...) {
        wchar_t buf[4096] = {0};

        va_list args;
        va_start(args, format);
        ::StringCchVPrintfW(buf, ARRAYSIZE(buf) - 1, format, args);
        va_end(args);

        buf[ARRAYSIZE(buf) - 1] = L'\0';
        ::OutputDebugStringW(buf);
    }

    bool StringEndsWith(const XString &str1, const XString &str2) {
        if (str1.Length() >= str2.Length()) {
            const char *str = str1.CStr() + (str1.Length() - str2.Length());
            return strncmp(str, str2.CStr(), str2.Length()) == 0;
        }
        return false;
    }

    bool StringIEndsWith(const XString &str1, const XString &str2) {
        if (str1.Length() >= str2.Length()) {
            const char *str = str1.CStr() + (str1.Length() - str2.Length());
            return strnicmp(str, str2.CStr(), str2.Length()) == 0;
        }
        return false;
    }


    void *GetSelfModuleHandle() {
        MEMORY_BASIC_INFORMATION mbi;
        return ((::VirtualQuery((LPVOID) &GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
                ? (HMODULE) mbi.AllocationBase : nullptr);
    }

    void *GetModuleBaseAddress(const char *modulePath) {
        if (!modulePath)
            return nullptr;

        int size = ::MultiByteToWideChar(CP_UTF8, 0, modulePath, -1, nullptr, 0);
        if (size == 0)
            return nullptr;

        auto ws = new wchar_t[size];
        ::MultiByteToWideChar(CP_UTF8, 0, modulePath, -1, ws, size);

        HMODULE hModule = ::GetModuleHandleW(ws);
        delete[] ws;
        if (!hModule)
            return nullptr;

        MODULEINFO moduleInfo;
        ::GetModuleInformation(::GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

        return moduleInfo.lpBaseOfDll;
    }
}
