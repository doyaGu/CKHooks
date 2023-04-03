#include "HookModule.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "Utils.h"

HookModule::~HookModule() {
    Fini();
}

HookModule::HookModule() = default;

void HookModule::Init(const HookModuleInfo &info) {
    if (IsInitialized()) return;

    TriggerCallbacks(HMCI_ONPREINIT);

    m_Info = info;
    m_Flag |= HMF_INITIALIZED;

    TriggerCallbacks(HMCI_ONPOSTINIT);
}

void HookModule::Fini() {
    if (!IsInitialized()) return;

    if (IsLoaded())
        Unload();

    TriggerCallbacks(HMCI_ONPREFINI);

    m_Info.Reset();
    m_Instance = nullptr;
    m_Flag &= ~HMF_INITIALIZED;

    TriggerCallbacks(HMCI_ONPOSTFINI);
}

void HookModule::AddCallback(int index, HookModuleCallback callback, void *arg) {
    if (!(index > 0 && index < 8 && callback != nullptr))
        return;

    Callback cb = {callback, arg};
    Callback *prev = m_Callbacks[index].Find(cb);
    if (prev == m_Callbacks[index].End()) {
        m_Callbacks[index].PushBack(cb);
    } else {
        m_Callbacks[index].Move(prev, m_Callbacks[index].End());
    }
}

void HookModule::RemoveCallback(int index, HookModuleCallback callback, void *arg) {
    if (!(index > 0 && index < 8 && callback != nullptr))
        return;

    Callback cb = {callback, arg};
    m_Callbacks[index].Remove(cb);
}

bool HookModule::Load() {
    if (!IsInitialized()) return false;

    if (IsLoaded()) {
        utils::OutputDebugA("%s: Hook \"%s\" is already loaded.", __FUNCTION__, m_Info.name.CStr());
        return true;
    }

    m_Instance = reinterpret_cast<INSTANCE_HANDLE *>(::LoadLibraryEx(m_Info.filename.CStr(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH));
    if (!m_Instance) {
        utils::OutputDebugA("%s: Failed to load Hook \"%s\".", __FUNCTION__, m_Info.name.CStr());
        return false;
    }

    m_Handler = reinterpret_cast<HookHandlerFunction>(::GetProcAddress((HMODULE) m_Instance, m_Info.handlerName.CStr()));
    if (!m_Handler) {
        utils::OutputDebugA("%s: Hook \"%s\" does not export handler function \"%s\".", __FUNCTION__, m_Info.name.CStr(), m_Info.handlerName.CStr());
        return false;
    }

    if (TriggerCallbacks(HMCI_ONPRELOAD) != 0)
        return false;

    if (Handle(HMA_LOAD, GetInfo().code, 0, reinterpret_cast<uintptr_t>(this)) != HMR_OK) {
        utils::OutputDebugA("%s: Hook \"%s\" handler can not execute loading successfully.", __FUNCTION__, m_Info.name.CStr());
        return false;
    }

    m_Flag |= HMF_LOADED;

    if (TriggerCallbacks(HMCI_ONPOSTLOAD) != 0)
        return false;

    return true;
}

bool HookModule::Unload() {
    if (!IsLoaded()) return false;

    if (TriggerCallbacks(HMCI_ONPREUNLOAD) != 0)
        return false;

    if (Handle(HMA_UNLOAD, GetInfo().code, 0, reinterpret_cast<uintptr_t>(this)) != HMR_OK) {
        utils::OutputDebugA("%s: Hook \"%s\" handler can not execute unloading successfully.", __FUNCTION__, m_Info.name.CStr());
        return false;
    }

    if (m_Instance) {
        ::FreeLibrary(reinterpret_cast<HMODULE>(m_Instance));
        m_Instance = nullptr;
    }

    m_Flag &= ~HMF_LOADED;

    if (TriggerCallbacks(HMCI_ONPOSTUNLOAD) != 0)
        return false;

    return true;
}

void HookModule::Enable() {
    m_Flag &= ~HMF_DISABLED;
}

void HookModule::Disable() {
    m_Flag |= HMF_DISABLED;
}

int HookModule::Handle(size_t action, size_t code, uintptr_t param1, uintptr_t param2) {
    if (!m_Handler) return HMR_NOTLOADED;
    if (IsDisabled()) return HMR_DISABLED;

    auto ret = static_cast<HookModuleReturn>(m_Handler(action, code, param1, param2));
    switch (ret) {
        case HMR_FREEZE:
            Disable();
        case HMR_REVIVE:
            Enable();
            break;
        default:
            break;
    }

    return ret;
}

int HookModule::Error(HookModuleErrorCode code, void *data1, void *data2) {
    return Handle(HMA_ERROR, code, reinterpret_cast<uintptr_t>(data1), reinterpret_cast<uintptr_t>(data2));
}

int HookModule::Query(HookModuleQueryCode code, void *data1, void *data2) {
    return Handle(HMA_QUERY, code, reinterpret_cast<uintptr_t>(data1), reinterpret_cast<uintptr_t>(data2));
}

int HookModule::Post(HookModulePostCode code, void *data1, void *data2) {
    return Handle(HMA_POST, code, reinterpret_cast<uintptr_t>(data1), reinterpret_cast<uintptr_t>(data2));
}

int HookModule::Set(size_t code, void **pcb, void **parg) {
    return Handle(HMA_SET, code, reinterpret_cast<uintptr_t>(pcb), reinterpret_cast<uintptr_t>(parg));
}

int HookModule::Unset(size_t code, void **pcb, void **parg) {
    return Handle(HMA_UNSET, code, reinterpret_cast<uintptr_t>(pcb), reinterpret_cast<uintptr_t>(parg));
}

void *HookModule::SetData(void *data, int id) {
    size_t old = 0;
    m_Data.LookUp(id, old);
    m_Data[id] = reinterpret_cast<size_t>(data);
    return reinterpret_cast<void *>(old);
}

void *HookModule::GetData(int id) const {
    size_t data = 0;
    if (!m_Data.LookUp(id, data))
        return nullptr;
    return reinterpret_cast<void *>(data);
}

int HookModule::TriggerCallbacks(int index) {
    int ret;
    Callback *cb = m_Callbacks[index].Begin();
    while (cb != m_Callbacks[index].End()) {
        if ((ret = (cb->callback)(this, cb->argument)) != 0)
            return ret;
        ++cb;
    }

    return 0;
}
