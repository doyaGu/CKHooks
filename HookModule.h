#ifndef HOOKS_HOOKMODULE_H
#define HOOKS_HOOKMODULE_H

#include <cstdlib>
#include <cstring>

#include "VxMathDefines.h"
#include "XString.h"
#include "XNHashTable.h"

#include "Hooks.h"

class HookModule;

struct HookModuleInfo {
    XString name;
    XString filename;
    XString handlerName;
    uint32_t code = 0;
    uint32_t version = 0;

    void Reset() {
        name = "";
        filename = "";
        handlerName = "";
        code = 0;
        version = 0;
    }
};

enum HookModuleFlag {
    HMF_UNINITIALIZED = 0x00000000,
    HMF_INITIALIZED = 0x00000001,
    HMF_LOADED = 0x00000002,
    HMF_DISABLED = 0x00000004,
    HMF_TERMINATED = 0x00000008,
};

enum HookModuleCallbackIndex {
    HMCI_ONPREINIT = 0,
    HMCI_ONPOSTINIT = 1,
    HMCI_ONPREFINI = 2,
    HMCI_ONPOSTFINI = 3,
    HMCI_ONPRELOAD = 4,
    HMCI_ONPOSTLOAD = 5,
    HMCI_ONPREUNLOAD = 6,
    HMCI_ONPOSTUNLOAD = 7,
};

typedef int (*HookModuleCallback)(HookModule *hook, void *arg);

class HookModule {
    friend class HookLoader;
public:
    HookModule(const HookModule &rhs) = delete;
    HookModule &operator=(const HookModule &rhs) = delete;

    ~HookModule();

    void Init(const HookModuleInfo &info);
    void Fini();

    void AddCallback(int index, HookModuleCallback callback, void *arg);
    void RemoveCallback(int index, HookModuleCallback callback, void *arg);

    bool Load();
    bool Unload();

    void Enable();
    void Disable();

    int Handle(size_t action, size_t code, uintptr_t param1, uintptr_t param2);

    int Error(HookModuleErrorCode code, void *data1, void *data2);

    int Query(HookModuleQueryCode code, void *data1, void *data2);
    int Post(HookModulePostCode code, void *data1, void *data2);

    int Set(size_t code, void **pcb, void **parg);
    int Unset(size_t code, void **pcb, void **parg);

    void *SetData(void *data, int id = 0);
    void *GetData(int id = 0) const;

    const HookModuleInfo &GetInfo() const {
        return m_Info;
    }

    bool IsInitialized() const {
        return (m_Flag & HMF_INITIALIZED) != 0;
    }

    bool IsLoaded() const {
        return (m_Flag & HMF_LOADED) != 0;
    }

    bool IsDisabled() const {
        return (m_Flag & HMF_DISABLED) != 0;
    }

    bool IsTerminated() const {
        return (m_Flag & HMF_TERMINATED) != 0;
    }

private:
    struct Callback {
        HookModuleCallback callback;
        void *argument;

        bool operator==(const Callback &rhs) const {
            return callback == rhs.callback && argument == rhs.argument;
        }

        bool operator!=(const Callback &rhs) const {
            return !(rhs == *this);
        }
    };

    HookModule();

    int TriggerCallbacks(int index);

    HookModuleInfo m_Info;
    uint32_t m_Flag = HMF_UNINITIALIZED;
    INSTANCE_HANDLE *m_Instance = nullptr;
    HookHandlerFunction m_Handler = nullptr;
    XNHashTable<size_t, int> m_Data;
    XArray<Callback> m_Callbacks[8];
};

#endif /* HOOKS_HOOKMODULE_H */
