#include "MessageHook.h"

#include <TlHelp32.h>

#include "Utils.h"

static HMODULE GetSelfModuleHandle() {
    MEMORY_BASIC_INFORMATION mbi;
    return ((::VirtualQuery((LPVOID) &GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
            ? (HMODULE) mbi.AllocationBase : nullptr);
}

static DWORD GetThreadInfo(DWORD dwProcessID) {
    HANDLE hSnThread = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnThread == INVALID_HANDLE_VALUE)
        return 0;

    THREADENTRY32 threadEntry = {sizeof(THREADENTRY32)};
    ::Thread32First(hSnThread, &threadEntry);
    do {
        if (threadEntry.th32OwnerProcessID == dwProcessID)
            return threadEntry.th32ThreadID;
    } while (::Thread32Next(hSnThread, &threadEntry));
    return 1;
}

MessageHook::MessageHook() = default;

MessageHook::~MessageHook() {
    if (IsInstalled()) {
        Uninstall();
    }
}

static int ConvertFunctionsIndexToHookType(MessageHookFunctionsIndex index) {
    switch (index) {
        case MHFI_MSGFILTER:
            return WH_MSGFILTER;
        case MHFI_JOURNALRECORD:
            return WH_JOURNALRECORD;
        case MHFI_JOURNALPLAYBACK:
            return WH_JOURNALPLAYBACK;
        case MHFI_KEYBOARD:
            return WH_KEYBOARD;
        case MHFI_GETMESSAGE:
            return WH_GETMESSAGE;
        case MHFI_CALLWNDPROC:
            return WH_CALLWNDPROC;
        case MHFI_CBT:
            return WH_CBT;
        case MHFI_SYSMSGFILTER:
            return WH_SYSMSGFILTER;
        case MHFI_MOUSE:
            return WH_MOUSE;
        case MHFI_DEBUG:
            return WH_DEBUG;
        case MHFI_SHELL:
            return WH_SHELL;
        case MHFI_FOREGROUNDIDLE:
            return WH_FOREGROUNDIDLE;
        case MHFI_CALLWNDPROCRET:
            return WH_CALLWNDPROCRET;
        case MHFI_KEYBOARD_LL:
            return WH_KEYBOARD_LL;
        case MHFI_MOUSE_LL:
            return WH_MOUSE_LL;
    }
    return 0xFF;
}

bool MessageHook::Install(int index, HOOKPROC proc) {
    int type = ConvertFunctionsIndexToHookType(static_cast<MessageHookFunctionsIndex>(index));
    if (type == 0xFF || !proc)
        return false;

    if (IsInstalled()) {
        utils::OutputDebugA("%s: Message Hook is already installed.", __FUNCTION__);
        return true;
    }

    HMODULE currentModule = GetSelfModuleHandle();
    DWORD currentThreadID = GetThreadInfo(::GetCurrentProcessId());

    m_HookProc = proc;
    m_Type = type;
    m_hHook = ::SetWindowsHookEx(m_Type, m_HookProc, currentModule, currentThreadID);
    if (!m_hHook) {
        utils::OutputDebugA("%s: Failed to install Message Hook.", __FUNCTION__);
        return false;
    }

    return true;
}

bool MessageHook::Uninstall() {
    if (!IsInstalled()) {
        utils::OutputDebugA("%s: Message Hook is not installed.", __FUNCTION__);
        return false;
    }

    if (::UnhookWindowsHookEx(m_hHook) != TRUE) {
        utils::OutputDebugA("%s: Failed to uninstall Message Hook.", __FUNCTION__);
        return false;
    }

    m_HookProc = nullptr;
    m_Type = 0;
    m_hHook = nullptr;

    return true;
}