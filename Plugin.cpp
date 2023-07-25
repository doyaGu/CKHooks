#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "HookLoader.h"
#include "HookManager.h"
#include "FileHook.h"
#include "Logger.h"
#include "Utils.h"

#include <MinHook.h>

extern HookApi g_HookApi;

void RegisterHookCallbacks(HookModule *hook, int functions) {
    assert(hook != nullptr);

    if (functions == 0) return;

    HookLoader &loader = HookLoader::GetInstance();
    auto context = reinterpret_cast<CKContext *>(loader.GetData(0x110));
    if (!context) return;

    HookManager *man = HookManager::GetManager(context);

#define HOOK_SET_CALLBACK(Name, Type) \
    do { \
        void *func = nullptr; \
        void *arg = nullptr; \
        if ((functions & CKHF_##Name) != 0 && \
            hook->Set(CKHFI_##Name, &func, &arg) == HMR_OK && func != nullptr) { \
            man->Add##Name##CallBack(reinterpret_cast<Type>(func), arg); \
        } \
    } while (0)

    HOOK_SET_CALLBACK(OnSequenceToBeDeleted, CK_DELETECALLBACK);
    HOOK_SET_CALLBACK(OnSequenceDeleted, CK_DELETECALLBACK);
    HOOK_SET_CALLBACK(PreProcess, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PostProcess, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PreClearAll, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PostClearAll, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnCKInit, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnCKEnd, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnCKPlay, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnCKPause, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PreLoad, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PreSave, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PreLaunchScene, CK_LAUNCHSCENECALLBACK);
    HOOK_SET_CALLBACK(PostLaunchScene, CK_LAUNCHSCENECALLBACK);
    HOOK_SET_CALLBACK(OnCKReset, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PostLoad, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(PostSave, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnCKPostReset, CK_PROCESSCALLBACK);
    HOOK_SET_CALLBACK(OnSequenceRemovedFromScene, CK_SCENECALLBACK);
    HOOK_SET_CALLBACK(OnPreCopy, CK_COPYCALLBACK);
    HOOK_SET_CALLBACK(OnPostCopy, CK_COPYCALLBACK);
    HOOK_SET_CALLBACK(OnPreRender, CK_RENDERCALLBACK);
    HOOK_SET_CALLBACK(OnPostRender, CK_RENDERCALLBACK);
    HOOK_SET_CALLBACK(OnPostSpriteRender, CK_RENDERCALLBACK);

#undef HOOK_SET_CALLBACK
}

void UnregisterHookCallbacks(HookModule *hook, int functions) {
    assert(hook != nullptr);

    if (functions == 0) return;

    HookLoader &loader = HookLoader::GetInstance();
    auto context = reinterpret_cast<CKContext *>(loader.GetData(0x110));
    if (!context) return;

    HookManager *man = HookManager::GetManager(context);

#define HOOK_UNSET_CALLBACK(Name, Type) \
    do { \
        void *func = nullptr; \
        void *arg = nullptr; \
        if ((functions & CKHF_##Name) != 0 && \
            hook->Unset(CKHFI_##Name, &func, &arg) == HMR_OK && func != nullptr) { \
            man->Remove##Name##CallBack(reinterpret_cast<Type>(func), arg); \
        } \
    } while (0)

    HOOK_UNSET_CALLBACK(OnSequenceToBeDeleted, CK_DELETECALLBACK);
    HOOK_UNSET_CALLBACK(OnSequenceDeleted, CK_DELETECALLBACK);
    HOOK_UNSET_CALLBACK(PreProcess, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PostProcess, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PreClearAll, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PostClearAll, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnCKInit, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnCKEnd, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnCKPlay, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnCKPause, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PreLoad, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PreSave, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PreLaunchScene, CK_LAUNCHSCENECALLBACK);
    HOOK_UNSET_CALLBACK(PostLaunchScene, CK_LAUNCHSCENECALLBACK);
    HOOK_UNSET_CALLBACK(OnCKReset, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PostLoad, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(PostSave, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnCKPostReset, CK_PROCESSCALLBACK);
    HOOK_UNSET_CALLBACK(OnSequenceRemovedFromScene, CK_SCENECALLBACK);
    HOOK_UNSET_CALLBACK(OnPreCopy, CK_COPYCALLBACK);
    HOOK_UNSET_CALLBACK(OnPostCopy, CK_COPYCALLBACK);
    HOOK_UNSET_CALLBACK(OnPreRender, CK_RENDERCALLBACK);
    HOOK_UNSET_CALLBACK(OnPostRender, CK_RENDERCALLBACK);
    HOOK_UNSET_CALLBACK(OnPostSpriteRender, CK_RENDERCALLBACK);

#undef HOOK_UNSET_CALLBACK
}

bool InitHookLoader() {
    HookLoader &loader = HookLoader::GetInstance();

    if (loader.IsInitialized()) return false;

    {
        auto hModule = reinterpret_cast<HMODULE>(loader.GetData(0x100));
        assert(hModule != nullptr);
        char buffer[MAX_PATH] = {0};
        ::GetModuleFileNameA(hModule, buffer, sizeof(buffer) / sizeof(char) - 1);

        XString logName = utils::RemoveExtension(buffer, "*") + ".log";
#ifdef NDEBUG
        Logger::Get().Init(logName.CStr());
#else
        Logger::Get().Init(logName.CStr(), LOG_LEVEL_TRACE);
#endif

        XString jsonName = utils::RemoveExtension(buffer, "*") + ".json";
        if (!loader.Init(jsonName.CStr()))
            return false;
    }

    const int count = loader.GetHookCount();
    for (int i = 0; i < count; ++i) {
        HookModule *hook = loader.GetHookByIndex(i);
        hook->AddCallback(HMCI_ONPRELOAD, [](HookModule *hook, void *arg) {
            int abi = HOOKS_ABI_VERSION;
            if (hook->Query(HMQC_ABI, nullptr, reinterpret_cast<void *>(&abi)) != HMR_OK ||
                abi > HOOKS_ABI_VERSION) {
                return 1;
            }

            int code = 0;
            if (hook->Query(HMQC_CODE, nullptr, reinterpret_cast<void *>(&code)) != HMR_OK ||
                code != hook->GetInfo().code) {
                return 1;
            }

            int version = 0;
            if (hook->Query(HMQC_VERSION, nullptr, reinterpret_cast<void *>(&version)) != HMR_OK ||
                version != hook->GetInfo().version) {
                return 1;
            }

            return 0;
        }, nullptr);
    }

    return true;
}

CKERROR InitInstance(CKContext *context) {
    auto *man = new HookManager(context);

    HookLoader &loader = HookLoader::GetInstance();
    loader.SetData(context, 0x110);

    loader.IterateHooks([](HookModule *hook, void *arg) -> int {
        auto *context = reinterpret_cast<CKContext *>(arg);

        hook->Post(HMPC_CKCONTEXT, nullptr, reinterpret_cast<void *>(context));
        hook->Post(HMPC_WINDOW, nullptr, reinterpret_cast<void *>(context->GetMainWindow()));

        int ckFuncs;
        if (hook->Query(HMQC_CK2, nullptr, reinterpret_cast<void *>(&ckFuncs)) == HMR_OK) {
            RegisterHookCallbacks(hook, ckFuncs);
        }

        hook->AddCallback(HMCI_ONPREUNLOAD, [](HookModule *hook, void *arg) {
            int ckFuncs;
            if (hook->Query(HMQC_CK2, nullptr, reinterpret_cast<void *>(&ckFuncs)) == HMR_OK) {
                UnregisterHookCallbacks(hook, ckFuncs);
            }

            return 0;
        }, nullptr);

        return (!hook->IsDisabled()) ? HMR_OK : HMR_TERMINATE;
    }, context);

    man->AddOnCKInitCallBack([](void *arg) {
        auto *context = reinterpret_cast<CKContext *>(arg);
        HookManager *man = HookManager::GetManager(context);
        man->AddOnCKEndCallBack([](void *) {
            HookLoader::GetInstance().SetData(nullptr, 0x110);
        }, nullptr);
    }, context);

    return CK_OK;
}

// In fact, this function will never be called.
CKERROR ExitInstance(CKContext *context) {
    HookManager *man = HookManager::GetManager(context);
    delete man;

    return CK_OK;
}

CKPluginInfo g_PluginInfo[2];

PLUGIN_EXPORT int CKGetPluginInfoCount() { return 2; }

PLUGIN_EXPORT CKPluginInfo *CKGetPluginInfo(int Index) {
    g_PluginInfo[0].m_Author = "Kakuty";
    g_PluginInfo[0].m_Description = "Building blocks for hooking";
    g_PluginInfo[0].m_Extension = "";
    g_PluginInfo[0].m_Type = CKPLUGIN_BEHAVIOR_DLL;
    g_PluginInfo[0].m_Version = 0x000001;
    g_PluginInfo[0].m_InitInstanceFct = nullptr;
    g_PluginInfo[0].m_ExitInstanceFct = nullptr;
    g_PluginInfo[0].m_GUID = CKGUID(0x3a086b4d, 0x2f4a4f01);
    g_PluginInfo[0].m_Summary = "Building blocks for hooking";

    g_PluginInfo[1].m_Author = "Kakuty";
    g_PluginInfo[1].m_Description = "Hook Manager";
    g_PluginInfo[1].m_Extension = "";
    g_PluginInfo[1].m_Type = CKPLUGIN_MANAGER_DLL;
    g_PluginInfo[1].m_Version = 0x000001;
    g_PluginInfo[1].m_InitInstanceFct = InitInstance;
    g_PluginInfo[1].m_ExitInstanceFct = ExitInstance;
    g_PluginInfo[1].m_GUID = HOOKMANAGER_GUID;
    g_PluginInfo[1].m_Summary = "Virtools Hook Manager";

    return &g_PluginInfo[Index];
}

PLUGIN_EXPORT void RegisterBehaviorDeclarations(XObjectDeclarationArray *reg);

void RegisterBehaviorDeclarations(XObjectDeclarationArray *reg) {
    HookLoader &loader = HookLoader::GetInstance();
    if (InitHookLoader()) {
        loader.IterateHooks([](HookModule *hook, void *arg) -> int {
            hook->AddCallback(HMCI_ONPRELOAD, [](HookModule *hook, void *arg) {
                HookApiIndex apiIndex = HAI_HOOK;
                hook->Post(HMPC_API, &apiIndex, reinterpret_cast<void *>(&g_HookApi));
                return 0;
            }, nullptr);
            return HMR_OK;
        }, nullptr);

        loader.LoadHooks();
    }

    RegisterBehavior(reg, FillBehaviorHookBlockDecl);
}

bool HookCreateCKBehaviorPrototypeRuntime() {
    HMODULE handle = ::GetModuleHandleA("CK2.dll");
    LPVOID lpCreateCKBehaviorPrototypeRunTimeProc =
        (LPVOID) ::GetProcAddress(handle, "?CreateCKBehaviorPrototypeRunTime@@YAPAVCKBehaviorPrototype@@PAD@Z");
    LPVOID lpCreateCKBehaviorPrototypeProc =
        (LPVOID) ::GetProcAddress(handle, "?CreateCKBehaviorPrototype@@YAPAVCKBehaviorPrototype@@PAD@Z");
    if (MH_CreateHook(lpCreateCKBehaviorPrototypeRunTimeProc, lpCreateCKBehaviorPrototypeProc, nullptr) != MH_OK ||
        MH_EnableHook(lpCreateCKBehaviorPrototypeRunTimeProc) != MH_OK) {
        return false;
    }
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            HookLoader::GetInstance().SetData(hModule, 0x100);
            if (MH_Initialize() != MH_OK) {
                utils::OutputDebugA("Fatal: Unable to initialize MinHook.\n");
                return FALSE;
            }
            if (!HookCreateCKBehaviorPrototypeRuntime()) {
                utils::OutputDebugA("Fatal: Unable to hook CreateCKBehaviorPrototypeRuntime.n");
                return FALSE;
            }
            if (!CP_HOOK_CLASS_NAME(CKFile)::InitHooks(&g_HookApi)) {
                utils::OutputDebugA("Fatal: Unable to hook CKFile.\n");
                return FALSE;
            }
            break;
        case DLL_PROCESS_DETACH:
            HookLoader::GetInstance().SetData(nullptr, 0x100);
            CP_HOOK_CLASS_NAME(CKFile)::ShutdownHooks(&g_HookApi);
            if (MH_Uninitialize() != MH_OK) {
                utils::OutputDebugA("Fatal: Unable to uninitialize MinHook.\n");
                return FALSE;
            }
            break;
        default:
            break;
    }
    return TRUE;
}
