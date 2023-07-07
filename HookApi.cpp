#include "Hooks.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <MinHook.h>

static HookApiErrorCode StatusToErrorCode(int status) {
    switch (status) {
        case MH_UNKNOWN:
            return HAEC_UNKNOWN;
        case MH_OK:
            return HAEC_OK;
        case MH_ERROR_ALREADY_CREATED:
            return HAEC_ALREADY_CREATED;
        case MH_ERROR_NOT_CREATED:
            return HAEC_NOT_CREATED;
        case MH_ERROR_ENABLED:
            return HAEC_ENABLED;
        case MH_ERROR_DISABLED:
            return HAEC_DISABLED;
        case MH_ERROR_NOT_EXECUTABLE:
            return HAEC_NOT_EXECUTABLE;
        case MH_ERROR_UNSUPPORTED_FUNCTION:
            return HAEC_UNSUPPORTED_FUNCTION;
        case MH_ERROR_MEMORY_ALLOC:
            return HAEC_MEMORY_ALLOC;
        case MH_ERROR_MEMORY_PROTECT:
            return HAEC_MEMORY_PROTECT;
        case MH_ERROR_MODULE_NOT_FOUND:
            return HAEC_MODULE_NOT_FOUND;
        case MH_ERROR_FUNCTION_NOT_FOUND:
            return HAEC_FUNCTION_NOT_FOUND;
        default:
            break;
    }
    return HAEC_UNKNOWN;
}

HookApiErrorCode HookApiCreateHook(void *target, void *detour, void **originalPtr) {
    return StatusToErrorCode(MH_CreateHook(target, detour, originalPtr));
}

HookApiErrorCode HookApiCreateHookApi(const char *modulePath, const char *procName, void *detour, void **originalPtr, void **targetPtr) {
    HMODULE hModule;

    if (modulePath) {
        int size = ::MultiByteToWideChar(CP_UTF8, 0, modulePath, -1, nullptr, 0);
        if (size == 0)
            return HAEC_MODULE_NOT_FOUND;

        auto ws = new wchar_t[size];
        ::MultiByteToWideChar(CP_UTF8, 0, modulePath, -1, ws, size);

        hModule = ::GetModuleHandleW(ws);
        delete[] ws;
        if (!hModule)
            return HAEC_MODULE_NOT_FOUND;
    } else {
        hModule = GetModuleHandle(nullptr);
    }

    void *target = reinterpret_cast<void *>(::GetProcAddress(hModule, procName));
    if (!target)
        return HAEC_FUNCTION_NOT_FOUND;

    if (targetPtr)
        *targetPtr = target;

    return StatusToErrorCode(MH_CreateHook(target, detour, originalPtr));
}

HookApiErrorCode HookApiRemoveHook(void *target) {
    return StatusToErrorCode(MH_RemoveHook(target));
}

HookApiErrorCode HookApiEnableHook(void *target) {
    return StatusToErrorCode(MH_EnableHook(target));
}

HookApiErrorCode HookApiDisableHook(void *target) {
    return StatusToErrorCode(MH_DisableHook(target));
}

HookApiErrorCode HookApiQueueEnableHook(void *target) {
    return StatusToErrorCode(MH_QueueEnableHook(target));
}

HookApiErrorCode HookApiQueueDisableHook(void *target) {
    return StatusToErrorCode(MH_QueueDisableHook(target));
}

HookApiErrorCode HookApiApplyQueued() {
    return StatusToErrorCode(MH_ApplyQueued());
}

HookApi g_HookApi = {
    HookApiCreateHook,
    HookApiCreateHookApi,
    HookApiRemoveHook,
    HookApiEnableHook,
    HookApiDisableHook,
    HookApiQueueEnableHook,
    HookApiQueueDisableHook,
    HookApiApplyQueued,
};
