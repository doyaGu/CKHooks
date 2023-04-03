#include "Hooks.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "CKContext.h"

int g_ModuleCode = 1000;
int g_ModuleVersion = 1;
CKContext *g_CKContext = nullptr;
HWND g_Window = nullptr;
void *g_Handle = nullptr;

static int OnError(HookModuleErrorCode code, void *data1, void *data2) {
    return HMR_OK;
}

static int OnQuery(HookModuleQueryCode code, void *data1, void *data2) {
    switch (code) {
        case HMQC_ABI:
            *reinterpret_cast<int *>(data2) = HOOKS_ABI_VERSION;
            break;
        case HMQC_CODE:
            *reinterpret_cast<int *>(data2) = g_ModuleCode;
            break;
        case HMQC_VERSION:
            *reinterpret_cast<int *>(data2) = g_ModuleVersion;
            break;
        case HMQC_CK2:
            *reinterpret_cast<int *>(data2) =
                CKHF_OnSequenceToBeDeleted |
                CKHF_OnSequenceDeleted |
                CKHF_PreProcess |
                CKHF_PostProcess |
                CKHF_PreClearAll |
                CKHF_PostClearAll |
                CKHF_OnCKInit |
                CKHF_OnCKEnd |
                CKHF_OnCKPlay |
                CKHF_OnCKPause |
                CKHF_PreLoad |
                CKHF_PreSave |
                CKHF_PreLaunchScene |
                CKHF_PostLaunchScene |
                CKHF_OnCKReset |
                CKHF_PostLoad |
                CKHF_PostSave |
                CKHF_OnCKPostReset |
                CKHF_OnSequenceAddedToScene |
                CKHF_OnSequenceRemovedFromScene |
                CKHF_OnPreCopy |
                CKHF_OnPostCopy |
                CKHF_OnPreRender |
                CKHF_OnPostRender |
                CKHF_OnPostSpriteRender;
            break;
        case HMQC_MSGHOOK:
            *reinterpret_cast<int *>(data2) = 0;
//                MHF_MSGFILTER |
//                MHF_JOURNALRECORD |
//                MHF_JOURNALPLAYBACK |
//                MHF_KEYBOARD |
//                MHF_GETMESSAGE |
//                MHF_CALLWNDPROC |
//                MHF_CBT |
//                MHF_SYSMSGFILTER |
//                MHF_MOUSE |
//                MHF_DEBUG |
//                MHF_SHELL |
//                MHF_FOREGROUNDIDLE |
//                MHF_CALLWNDPROCRET |
//                MHF_KEYBOARD_LL |
//                MHF_MOUSE_LL;
            break;
        default:
            return HMR_SKIP;
    }

    return HMR_OK;
}

static int OnPost(HookModulePostCode code, void *data1, void *data2) {
    switch (code) {
        case HMPC_CKCONTEXT:
            g_CKContext = (CKContext *) data2;
            break;
        case HMPC_WINDOW:
            g_Window = (HWND) data2;
            break;
        default:
            return HMR_SKIP;
    }
    return HMR_OK;
}

static int OnLoad(size_t code, void *handle) {
    g_Handle = reinterpret_cast<void *>(handle);
    return HMR_OK;
}

static int OnUnload(size_t code, void * /* handle */) {

    return HMR_OK;
}

CKERROR PreClearAll(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PreClearAll");
    return CK_OK;
}

CKERROR PostClearAll(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PostClearAll");
    return CK_OK;
}

CKERROR PreProcess(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PreProcess");
    return CK_OK;
}

CKERROR PostProcess(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PostProcess");
    return CK_OK;
}

CKERROR OnSequenceAddedToScene(CKScene *scn, CK_ID *objids, int count, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"SequenceAddedToScene");
    return CK_OK;
}

CKERROR OnSequenceRemovedFromScene(CKScene *scn, CK_ID *objids, int count, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"SequenceRemovedFromScene");
    return CK_OK;
}

CKERROR PreLaunchScene(CKScene *OldScene, CKScene *NewScene, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PreLaunchScene");
    return CK_OK;
}

CKERROR PostLaunchScene(CKScene *OldScene, CKScene *NewScene, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PostLaunchScene");
    return CK_OK;
}

CKERROR OnCKInit(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKInit");
    return CK_OK;
}

CKERROR OnCKEnd(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKEnd");
    return CK_OK;
}

CKERROR OnCKReset(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKReset");
    return CK_OK;
}

CKERROR OnCKPostReset(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKPostReset");
    return CK_OK;
}

CKERROR OnCKPause(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKPause");
    return CK_OK;
}

CKERROR OnCKPlay(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnCKPlay");
    return CK_OK;
}

CKERROR OnSequenceToBeDeleted(CK_ID *objids, int count, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"SequenceToBeDeleted");
    return CK_OK;
}
CKERROR OnSequenceDeleted(CK_ID *objids, int count, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"SequenceDeleted");
    return CK_OK;
}

CKERROR PreLoad(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PreLoad");
    return CK_OK;
}

CKERROR PostLoad(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PostLoad");
    return CK_OK;
}

CKERROR PreSave(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PreSave");
    return CK_OK;
}

CKERROR PostSave(void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"PostSave");
    return CK_OK;
}

CKERROR OnPreCopy(CKDependenciesContext *context, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnPreCopy");
    return CK_OK;
}

CKERROR OnPostCopy(CKDependenciesContext *context, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnPostCopy");
    return CK_OK;
}

CKERROR OnPreRender(CKRenderContext *dev, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnPreRender");
    return CK_OK;
}

CKERROR OnPostRender(CKRenderContext *dev, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnPostRender");
    return CK_OK;
}

CKERROR OnPostSpriteRender(CKRenderContext *dev, void *arg) {
    g_CKContext->OutputToConsole((CKSTRING)"OnPostSpriteRender");
    return CK_OK;
}

static int OnSet(size_t code, void **pcb, void **parg) {
    switch (code) {
        case CKHFI_OnSequenceToBeDeleted:
            *pcb = reinterpret_cast<void*>(OnSequenceToBeDeleted);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceDeleted:
            *pcb = reinterpret_cast<void*>(OnSequenceDeleted);
            *parg = nullptr;
            break;
        case CKHFI_PreProcess:
            *pcb = reinterpret_cast<void*>(PreProcess);
            *parg = nullptr;
            break;
        case CKHFI_PostProcess:
            *pcb = reinterpret_cast<void*>(PostProcess);
            *parg = nullptr;
            break;
        case CKHFI_PreClearAll:
            *pcb = reinterpret_cast<void*>(PreClearAll);
            *parg = nullptr;
            break;
        case CKHFI_PostClearAll:
            *pcb = reinterpret_cast<void*>(PostClearAll);
            *parg = nullptr;
            break;
//        case CKHFI_OnCKInit:
//            *pcb = reinterpret_cast<void*>(OnCKInit);
//            *parg = nullptr;
//            break;
//        case CKHFI_OnCKEnd:
//            *pcb = reinterpret_cast<void*>(OnCKEnd);
//            *parg = nullptr;
//            break;
        case CKHFI_OnCKPlay:
            *pcb = reinterpret_cast<void*>(OnCKPlay);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPause:
            *pcb = reinterpret_cast<void*>(OnCKPause);
            *parg = nullptr;
            break;
        case CKHFI_PreLoad:
            *pcb = reinterpret_cast<void*>(PreLoad);
            *parg = nullptr;
            break;
        case CKHFI_PreSave:
            *pcb = reinterpret_cast<void*>(PreSave);
            *parg = nullptr;
            break;
        case CKHFI_PreLaunchScene:
            *pcb = reinterpret_cast<void*>(PreLaunchScene);
            *parg = nullptr;
            break;
        case CKHFI_PostLaunchScene:
            *pcb = reinterpret_cast<void*>(PostLaunchScene);
            *parg = nullptr;
            break;
        case CKHFI_OnCKReset:
            *pcb = reinterpret_cast<void*>(OnCKReset);
            *parg = nullptr;
            break;
        case CKHFI_PostLoad:
            *pcb = reinterpret_cast<void*>(PostLoad);
            *parg = nullptr;
            break;
        case CKHFI_PostSave:
            *pcb = reinterpret_cast<void*>(PostSave);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPostReset:
            *pcb = reinterpret_cast<void*>(OnCKPostReset);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceAddedToScene:
            *pcb = reinterpret_cast<void*>(OnSequenceAddedToScene);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceRemovedFromScene:
            *pcb = reinterpret_cast<void*>(OnSequenceRemovedFromScene);
            *parg = nullptr;
            break;
        case CKHFI_OnPreCopy:
            *pcb = reinterpret_cast<void*>(OnPreCopy);
            *parg = nullptr;
            break;
        case CKHFI_OnPostCopy:
            *pcb = reinterpret_cast<void*>(OnPostCopy);
            *parg = nullptr;
            break;
        case CKHFI_OnPreRender:
            *pcb = reinterpret_cast<void*>(OnPreRender);
            *parg = nullptr;
            break;
        case CKHFI_OnPostRender:
            *pcb = reinterpret_cast<void*>(OnPostRender);
            *parg = nullptr;
            break;
        case CKHFI_OnPostSpriteRender:
            *pcb = reinterpret_cast<void*>(OnPostSpriteRender);
            *parg = nullptr;
            break;
        default:
            break;
    }
    return HMR_OK;
}

static int OnUnset(size_t code, void **pcb, void **parg) {
    switch (code) {
        case CKHFI_OnSequenceToBeDeleted:
            *pcb = reinterpret_cast<void*>(OnSequenceToBeDeleted);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceDeleted:
            *pcb = reinterpret_cast<void*>(OnSequenceDeleted);
            *parg = nullptr;
            break;
        case CKHFI_PreProcess:
            *pcb = reinterpret_cast<void*>(PreProcess);
            *parg = nullptr;
            break;
        case CKHFI_PostProcess:
            *pcb = reinterpret_cast<void*>(PostProcess);
            *parg = nullptr;
            break;
        case CKHFI_PreClearAll:
            *pcb = reinterpret_cast<void*>(PreClearAll);
            *parg = nullptr;
            break;
        case CKHFI_PostClearAll:
            *pcb = reinterpret_cast<void*>(PostClearAll);
            *parg = nullptr;
            break;
//        case CKHFI_OnCKInit:
//            *pcb = reinterpret_cast<void*>(OnCKInit);
//            *parg = nullptr;
//            break;
//        case CKHFI_OnCKEnd:
//            *pcb = reinterpret_cast<void*>(OnCKEnd);
//            *parg = nullptr;
//            break;
        case CKHFI_OnCKPlay:
            *pcb = reinterpret_cast<void*>(OnCKPlay);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPause:
            *pcb = reinterpret_cast<void*>(OnCKPause);
            *parg = nullptr;
            break;
        case CKHFI_PreLoad:
            *pcb = reinterpret_cast<void*>(PreLoad);
            *parg = nullptr;
            break;
        case CKHFI_PreSave:
            *pcb = reinterpret_cast<void*>(PreSave);
            *parg = nullptr;
            break;
        case CKHFI_PreLaunchScene:
            *pcb = reinterpret_cast<void*>(PreLaunchScene);
            *parg = nullptr;
            break;
        case CKHFI_PostLaunchScene:
            *pcb = reinterpret_cast<void*>(PostLaunchScene);
            *parg = nullptr;
            break;
        case CKHFI_OnCKReset:
            *pcb = reinterpret_cast<void*>(OnCKReset);
            *parg = nullptr;
            break;
        case CKHFI_PostLoad:
            *pcb = reinterpret_cast<void*>(PostLoad);
            *parg = nullptr;
            break;
        case CKHFI_PostSave:
            *pcb = reinterpret_cast<void*>(PostSave);
            *parg = nullptr;
            break;
        case CKHFI_OnCKPostReset:
            *pcb = reinterpret_cast<void*>(OnCKPostReset);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceAddedToScene:
            *pcb = reinterpret_cast<void*>(OnSequenceAddedToScene);
            *parg = nullptr;
            break;
        case CKHFI_OnSequenceRemovedFromScene:
            *pcb = reinterpret_cast<void*>(OnSequenceRemovedFromScene);
            *parg = nullptr;
            break;
        case CKHFI_OnPreCopy:
            *pcb = reinterpret_cast<void*>(OnPreCopy);
            *parg = nullptr;
            break;
        case CKHFI_OnPostCopy:
            *pcb = reinterpret_cast<void*>(OnPostCopy);
            *parg = nullptr;
            break;
        case CKHFI_OnPreRender:
            *pcb = reinterpret_cast<void*>(OnPreRender);
            *parg = nullptr;
            break;
        case CKHFI_OnPostRender:
            *pcb = reinterpret_cast<void*>(OnPostRender);
            *parg = nullptr;
            break;
        case CKHFI_OnPostSpriteRender:
            *pcb = reinterpret_cast<void*>(OnPostSpriteRender);
            *parg = nullptr;
            break;
        default:
            break;
    }
    return HMR_OK;
}

HOOKS_EXPORT int TestHandler(size_t action, size_t code, uintptr_t param1, uintptr_t param2) {
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