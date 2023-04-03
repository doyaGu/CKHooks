#ifndef HOOKS_UTILS_H
#define HOOKS_UTILS_H

#include "XString.h"

namespace utils {
    XString GetCurrentDir();

    XString RemoveDirectoryName(const XString &path);
    XString RemoveFileName(const XString &path);
    XString RemoveExtension(const XString &path, const XString &ext);

    XString JoinPaths(const XString &path1, const XString &path2);
    XString MakeFileName(const XString &dir, const XString &name, const XString &ext);

    bool IsFileExist(const XString &file);
    bool IsDirectoryExist(const XString &dir);

    bool IsAbsolutePath(const XString &path);
    XString GetAbsolutePath(const XString &path, bool trailing = false);

    bool CreateDir(const XString &dir);
    bool RemoveDir(const XString &dir);

    const char *FindLastPathSeparator(const XString &path);
    bool HasTrailingPathSeparator(const XString &path);
    XString RemoveTrailingPathSeparator(const XString &path);
    void NormalizePath(XString &path);

    void OutputDebugA(const char *format, ...);
    void OutputDebugW(const wchar_t *format, ...);

    wchar_t *AnsiToUtf16(const char *str);
    char *Utf16ToAnsi(const wchar_t *str);
}

#endif // HOOKS_UTILS_H
