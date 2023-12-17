#ifndef HOOKS_HOOKLOADER_H
#define HOOKS_HOOKLOADER_H

#include <cstdlib>
#include <cstring>

#include "VxMathDefines.h"
#include "XString.h"
#include "XArray.h"
#include "XNHashTable.h"

#include "HookModule.h"

enum HookLoaderFlag {
    HLF_UNINITIALIZED = 0x00000000,
    HLF_INITIALIZED = 0x00000001,
    HLF_LOADED = 0x00000002,
};

class HookLoader {
public:
    static HookLoader &GetInstance();

    HookLoader(const HookLoader &rhs) = delete;
    HookLoader &operator=(const HookLoader &rhs) = delete;

    ~HookLoader();

    bool Init(const char *filename);
    void Fini();

    void LoadHooks();
    void UnloadHooks();
    void IterateHooks(HookModuleCallback callback, void *arg);

    int GetHookCount();
    HookModule *GetHookByIndex(int index);
    HookModule *GetHookByName(const char *name);

    void *SetData(void *data, int id = 0);
    void *GetData(int id = 0) const;

    bool IsInitialized() const {
        return (m_Flag & HLF_INITIALIZED) != 0;
    }

    bool IsLoaded() const {
        return (m_Flag & HLF_LOADED) != 0;
    }

private:
    HookLoader();

    bool LoadJson(char *data, size_t len);

    HookModule *CreateHook(const HookModuleInfo &info);
    void DestroyHook(HookModule *hook);

    static int OnPreHookFini(HookModule *hook, void *arg);

    int m_Flag = HLF_UNINITIALIZED;
    XArray<HookModule *> m_Hooks;
    XArray<XString> m_Paths;
    XNHashTable<HookModule *, XString> m_HookMap;
    XNHashTable<size_t, int> m_Data;
};

#endif /* HOOKS_HOOKLOADER_H */
