#include "Hooks.h"

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>

#include "CKAll.h"

CKContext *g_CKContext = nullptr;
FILE *g_File = nullptr;

struct CKGUIDHash {
    size_t operator()(const CKGUID &guid) const {
        return std::hash<CKDWORD>()(guid.d[0]) ^ std::hash<CKDWORD>()(guid.d[1]);
    }
};

struct CKGUIDEq {
    bool operator()(const CKGUID &guid1, const CKGUID &guid2) const {
        return guid1 == guid2;
    };
};

std::vector<std::string> g_PluginNames;
std::set<CKGUID> g_BehaviorGuids;
std::unordered_map<CKGUID, std::string, CKGUIDHash, CKGUIDEq> g_BehGuidToBehNameMap;
std::unordered_map<CKGUID, std::string, CKGUIDHash, CKGUIDEq> g_BehGuidToPluginMap;

static int OnError(HookModuleErrorCode code, void *data1, void *data2) {
    return HMR_OK;
}

static int OnQuery(HookModuleQueryCode code, void *data1, void *data2) {
    switch (code) {
        case HMQC_ABI:
            *reinterpret_cast<int *>(data2) = HOOKS_ABI_VERSION;
            break;
        case HMQC_CODE:
            *reinterpret_cast<int *>(data2) = 2000;
            break;
        case HMQC_VERSION:
            *reinterpret_cast<int *>(data2) = 1;
            break;
        case HMQC_CK2:
            *reinterpret_cast<int *>(data2) =
                CKHF_OnCKInit |
                CKHF_OnCKEnd |
                CKHF_PreLoad |
                CKHF_OnCKReset |
                CKHF_PostLoad |
                CKHF_OnCKPostReset;
            break;
        default:
            return HMR_SKIP;
    }

    return HMR_OK;
}

static int OnPost(HookModulePostCode code, void *data1, void *data2) {
    switch (code) {
        case HMPC_CKCONTEXT:
            g_CKContext = reinterpret_cast<CKContext *>(data2);
            break;
        default:
            return HMR_SKIP;
    }
    return HMR_OK;
}

static int OnLoad(size_t code, void * /* handle */) {
    g_File = fopen("BuildingBlocks.txt", "w");
    if (!g_File)
        return HMR_FAIL;

    return HMR_OK;
}

static int OnUnload(size_t code, void * /* handle */) {
    if (g_File)
        fclose(g_File);

    return HMR_OK;
}

CKERROR OnCKInit(void *arg) {
    return CK_OK;
}

CKERROR OnCKEnd(void *arg) {
    fputs("PluginName\t\t\t\t\t\tBehaviorName\t\t\t\t\t\t\tBehaviorGuid\n", g_File);

    std::map<std::string, std::vector<CKGUID>> behaviors;
    for (auto guid: g_BehaviorGuids) {
        behaviors[g_BehGuidToPluginMap[guid]].emplace_back(guid);
    }

    for (auto &pair: behaviors) {
        auto &guids = pair.second;
        for (auto &guid: guids) {
            fprintf(g_File, "%-30s\t%-36s\tCKGUID(%#lx,%#lx)\n", pair.first.c_str(), g_BehGuidToBehNameMap[guid].c_str(), guid.d1, guid.d2);
        }
    }

    return CK_OK;
}

CKERROR OnCKReset(void *arg) {
    return CK_OK;
}

CKERROR OnCKPostReset(void *arg) {
    CKPluginManager *pm = CKGetPluginManager();
    int pluginCount = pm->GetPluginCount(CKPLUGIN_BEHAVIOR_DLL);
    int pluginIdx;
    CKPluginEntry *pluginInfo = NULL;
    for (pluginIdx = 0; pluginIdx < pluginCount; ++pluginIdx) {
        pluginInfo = pm->GetPluginInfo(CKPLUGIN_BEHAVIOR_DLL, pluginIdx);
        CKPluginDll *pluginDll = pm->GetPluginDllInfo(pluginInfo->m_PluginDllIndex);

        XString name = pluginDll->m_DllFileName;
        int pos1 = name.RFind('\\') + 1;
        int pos2 = name.RFind('.');
        XString dll = name.Substring(pos1, pos2 - pos1);
        std::string pluginName(dll.CStr());

        g_PluginNames.emplace_back(pluginName);

        g_CKContext->OutputToConsoleEx((CKSTRING) "Plugin [%s]:", pluginName.c_str());

        XArray<CKGUID> &guids = pluginInfo->m_BehaviorsInfo->m_BehaviorsGUID;
        CKObjectDeclaration *od;

        for (CKGUID *it = guids.Begin(); it != guids.End(); ++it) {
            od = CKGetObjectDeclarationFromGuid(*it);
            g_BehGuidToPluginMap.emplace(od->GetGuid(), pluginName);
            g_BehGuidToBehNameMap.emplace(od->GetGuid(), od->GetName());

            g_CKContext->OutputToConsoleEx((CKSTRING) "\t\t%-36s\tCKGUID(%#lx,%#lx)", od->GetName(), od->GetGuid().d1, od->GetGuid().d2);
        }
    }

    return CK_OK;
}

CKERROR PreLoad(void *arg) {
    return CK_OK;
}

CKERROR PostLoad(void *arg) {
    const XObjectPointerArray &behaviors = g_CKContext->GetObjectListByType(CKCID_BEHAVIOR, TRUE);
    for (XObjectPointerArray::Iterator it = behaviors.Begin(); it != behaviors.End(); ++it) {
        auto *beh = (CKBehavior *) (*it);
        g_BehaviorGuids.insert(beh->GetPrototypeGuid());
    }
    return CK_OK;
}

static int OnSet(size_t code, void **pcb, void **parg) {
    switch (code) {
        case CKHFI_OnCKInit:
            *pcb = reinterpret_cast<void *>(OnCKInit);
            *parg = nullptr;
            break;
        case CKHFI_OnCKEnd:
            *pcb = reinterpret_cast<void *>(OnCKEnd);
            *parg = nullptr;
            break;
        case CKHFI_PreLoad:
            *pcb = reinterpret_cast<void *>(PreLoad);
            *parg = nullptr;
            break;
        case CKHFI_OnCKReset:
            *pcb = reinterpret_cast<void *>(OnCKReset);
            *parg = nullptr;
            break;
        case CKHFI_PostLoad:
            *pcb = reinterpret_cast<void *>(PostLoad);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPostReset:
            *pcb = reinterpret_cast<void *>(OnCKPostReset);
            *parg = nullptr;
            break;
        default:
            break;
    }
    return HMR_OK;
}

static int OnUnset(size_t code, void **pcb, void **parg) {
    switch (code) {
        case CKHFI_OnCKInit:
            *pcb = reinterpret_cast<void *>(OnCKInit);
            *parg = nullptr;
            break;
        case CKHFI_OnCKEnd:
            *pcb = reinterpret_cast<void *>(OnCKEnd);
            *parg = nullptr;
            break;
        case CKHFI_PreLoad:
            *pcb = reinterpret_cast<void *>(PreLoad);
            *parg = nullptr;
            break;
        case CKHFI_OnCKReset:
            *pcb = reinterpret_cast<void *>(OnCKReset);
            *parg = nullptr;
            break;
        case CKHFI_PostLoad:
            *pcb = reinterpret_cast<void *>(PostLoad);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPostReset:
            *pcb = reinterpret_cast<void *>(OnCKPostReset);
            *parg = nullptr;
            break;
        default:
            break;
    }
    return HMR_OK;
}

HOOKS_EXPORT int Handler(size_t action, size_t code, uintptr_t param1, uintptr_t param2) {
    switch (action) {
        case HMA_ERROR:
            return OnError(static_cast<HookModuleErrorCode>(code), reinterpret_cast<void *>(param1), reinterpret_cast<void *>(param2));
        case HMA_QUERY:
            return OnQuery(static_cast<HookModuleQueryCode>(code), reinterpret_cast<void *>(param1), reinterpret_cast<void *>(param2));
        case HMA_POST:
            return OnPost(static_cast<HookModulePostCode>(code), reinterpret_cast<void *>(param1), reinterpret_cast<void *>(param2));
        case HMA_LOAD:
            return OnLoad(code, reinterpret_cast<void *>(param2));
        case HMA_UNLOAD:
            return OnUnload(code, reinterpret_cast<void *>(param2));
        case HMA_SET:
            return OnSet(code, reinterpret_cast<void **>(param1), reinterpret_cast<void **>(param2));
        case HMA_UNSET:
            return OnUnset(code, reinterpret_cast<void **>(param1), reinterpret_cast<void **>(param2));
        default:
            return HMR_SKIP;
    }
}