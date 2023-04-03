#ifndef HOOKS_MESSAGEHOOK_H
#define HOOKS_MESSAGEHOOK_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "XArray.h"

#include "Hooks.h"

class MessageHook {
public:
    MessageHook();

    MessageHook(const MessageHook &rhs) = delete;
    MessageHook &operator=(const MessageHook &rhs) = delete;

    ~MessageHook();

    bool Install(int index, HOOKPROC proc);
    bool Uninstall();

    bool IsInstalled() const {
        return m_hHook != nullptr;
    }

    HHOOK GetHandle() { return m_hHook; }

private:
    int m_Type = 0;
    HOOKPROC m_HookProc = nullptr;
    HHOOK m_hHook = nullptr;
};

#endif /* HOOKS_MESSAGEHOOK_H */
