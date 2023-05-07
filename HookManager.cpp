#include "HookManager.h"

#define HM_TRIGGER_CALLBACK(Name, Type) \
    do { \
        Callback *cb = m_##Name##Callbacks.Begin(); \
        while (cb != m_##Name##Callbacks.End()) { \
            ((Type)cb->callback)(cb->argument); \
            if (cb->temp) \
                cb = m_##Name##Callbacks.Remove(cb); \
            else \
                ++cb; \
        } \
    } while (0)

#define HM_TRIGGER_CALLBACK_ARGS(Name, Type, ...) \
    do { \
        Callback *cb = m_##Name##Callbacks.Begin(); \
        while (cb != m_##Name##Callbacks.End()) { \
            ((Type)cb->callback)(__VA_ARGS__, cb->argument); \
            if (cb->temp) \
                cb = m_##Name##Callbacks.Remove(cb); \
            else \
                ++cb; \
        } \
    } while (0)

HookManager::HookManager(CKContext *context) : CKBaseManager(context, HOOKMANAGER_GUID, "Hook Manager") {
    context->RegisterNewManager(this);
}

HookManager::~HookManager() = default;

CKERROR HookManager::PreClearAll() {
    HM_TRIGGER_CALLBACK(PreClearAll, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PostClearAll() {
    HM_TRIGGER_CALLBACK(PostClearAll, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PreProcess() {
    HM_TRIGGER_CALLBACK(PreProcess, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PostProcess() {
    HM_TRIGGER_CALLBACK(PostProcess, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::SequenceAddedToScene(CKScene *scn, CK_ID *objids, int count) {
    HM_TRIGGER_CALLBACK_ARGS(OnSequenceAddedToScene, CK_SCENECALLBACK, scn, objids, count);
    return CK_OK;
}

CKERROR HookManager::SequenceRemovedFromScene(CKScene *scn, CK_ID *objids, int count) {
    HM_TRIGGER_CALLBACK_ARGS(OnSequenceRemovedFromScene, CK_SCENECALLBACK, scn, objids, count);
    return CK_OK;
}

CKERROR HookManager::PreLaunchScene(CKScene *OldScene, CKScene *NewScene) {
    HM_TRIGGER_CALLBACK_ARGS(PreLaunchScene, CK_LAUNCHSCENECALLBACK, OldScene, NewScene);
    return CK_OK;
}

CKERROR HookManager::PostLaunchScene(CKScene *OldScene, CKScene *NewScene) {
    HM_TRIGGER_CALLBACK_ARGS(PostLaunchScene, CK_LAUNCHSCENECALLBACK, OldScene, NewScene);
    return CK_OK;
}

CKERROR HookManager::OnCKInit() {
    HM_TRIGGER_CALLBACK(OnCKInit, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnCKEnd() {
    HM_TRIGGER_CALLBACK(OnCKEnd, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnCKReset() {
    HM_TRIGGER_CALLBACK(OnCKReset, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnCKPostReset() {
    HM_TRIGGER_CALLBACK(OnCKPostReset, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnCKPause() {
    HM_TRIGGER_CALLBACK(OnCKPause, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnCKPlay() {
    HM_TRIGGER_CALLBACK(OnCKPlay, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::SequenceToBeDeleted(CK_ID *objids, int count) {
    HM_TRIGGER_CALLBACK_ARGS(OnSequenceToBeDeleted, CK_DELETECALLBACK, objids, count);
    return CK_OK;
}

CKERROR HookManager::SequenceDeleted(CK_ID *objids, int count) {
    HM_TRIGGER_CALLBACK_ARGS(OnSequenceDeleted, CK_DELETECALLBACK, objids, count);
    return CK_OK;
}

CKERROR HookManager::PreLoad() {
    HM_TRIGGER_CALLBACK(PreLoad, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PostLoad() {
    HM_TRIGGER_CALLBACK(PostLoad, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PreSave() {
    HM_TRIGGER_CALLBACK(PreSave, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::PostSave() {
    HM_TRIGGER_CALLBACK(PostSave, CK_PROCESSCALLBACK);
    return CK_OK;
}

CKERROR HookManager::OnPreCopy(CKDependenciesContext &context) {
    HM_TRIGGER_CALLBACK_ARGS(OnPreCopy, CK_COPYCALLBACK, &context);
    return CK_OK;
}

CKERROR HookManager::OnPostCopy(CKDependenciesContext &context) {
    HM_TRIGGER_CALLBACK_ARGS(OnPostCopy, CK_COPYCALLBACK, &context);
    return CK_OK;
}

CKERROR HookManager::OnPreRender(CKRenderContext *dev) {
    HM_TRIGGER_CALLBACK_ARGS(OnPreRender, CK_RENDERCALLBACK, dev);
    return CK_OK;
}

CKERROR HookManager::OnPostRender(CKRenderContext *dev) {
    HM_TRIGGER_CALLBACK_ARGS(OnPostRender, CK_RENDERCALLBACK, dev);
    return CK_OK;
}

CKERROR HookManager::OnPostSpriteRender(CKRenderContext *dev) {
    HM_TRIGGER_CALLBACK_ARGS(OnPostSpriteRender, CK_RENDERCALLBACK, dev);
    return CK_OK;
}

CKDWORD HookManager::GetValidFunctionsMask() {
    return CKMANAGER_FUNC_OnSequenceToBeDeleted |
           CKMANAGER_FUNC_OnSequenceDeleted |
           CKMANAGER_FUNC_PreProcess |
           CKMANAGER_FUNC_PostProcess |
           CKMANAGER_FUNC_PreClearAll |
           CKMANAGER_FUNC_PostClearAll |
           CKMANAGER_FUNC_OnCKInit |
           CKMANAGER_FUNC_OnCKEnd |
           CKMANAGER_FUNC_OnCKPlay |
           CKMANAGER_FUNC_OnCKPause |
           CKMANAGER_FUNC_PreLoad |
           CKMANAGER_FUNC_PreSave |
           CKMANAGER_FUNC_PreLaunchScene |
           CKMANAGER_FUNC_PostLaunchScene |
           CKMANAGER_FUNC_OnCKReset |
           CKMANAGER_FUNC_PostLoad |
           CKMANAGER_FUNC_PostSave |
           CKMANAGER_FUNC_OnCKPostReset |
           CKMANAGER_FUNC_OnSequenceAddedToScene |
           CKMANAGER_FUNC_OnSequenceRemovedFromScene |
           CKMANAGER_FUNC_OnPreCopy |
           CKMANAGER_FUNC_OnPostCopy |
           CKMANAGER_FUNC_OnPreRender |
           CKMANAGER_FUNC_OnPostRender |
           CKMANAGER_FUNC_OnPostSpriteRender;
}

#undef HM_TRIGGER_CALLBACK
#undef HM_TRIGGER_CALLBACK_ARGS

#define HM_ADD_CALLBACK(Name, Func, Arg, Temp) \
    do { \
        Callback cb = {(void *)Func, Arg, Temp}; \
        Callback *prev = m_##Name##Callbacks.Find(cb); \
        if (prev == m_##Name##Callbacks.End()) { \
            m_##Name##Callbacks.PushBack(cb); \
        } else { \
            m_##Name##Callbacks.Move(prev, m_##Name##Callbacks.End()); \
        } \
    } while (0)


#define HM_REMOVE_CALLBACK(Name, Func, Arg) \
    do { \
        Callback cb = {(void *)Func, Arg, FALSE}; \
        m_##Name##Callbacks.Remove(cb); \
    } while (0)

void HookManager::AddPreClearAllCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PreClearAll, func, arg, temp);
}

void HookManager::RemovePreClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PreClearAll, func, arg);
}

void HookManager::AddPostClearAllCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PostClearAll, func, arg, temp);
}

void HookManager::RemovePostClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PostClearAll, func, arg);
}

void HookManager::AddPreProcessCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PreProcess, func, arg, temp);
}

void HookManager::RemovePreProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PreProcess, func, arg);
}

void HookManager::AddPostProcessCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PostProcess, func, arg, temp);
}

void HookManager::RemovePostProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PostProcess, func, arg);
}


void HookManager::AddOnSequenceAddedToSceneCallBack(CK_SCENECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnSequenceAddedToScene, func, arg, temp);
}

void HookManager::RemoveOnSequenceAddedToSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnSequenceAddedToScene, func, arg);
}

void HookManager::AddOnSequenceRemovedFromSceneCallBack(CK_SCENECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnSequenceRemovedFromScene, func, arg, temp);
}

void HookManager::RemoveOnSequenceRemovedFromSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnSequenceRemovedFromScene, func, arg);
}

void HookManager::AddPreLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PreLaunchScene, func, arg, temp);
}

void HookManager::RemovePreLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PreLaunchScene, func, arg);
}

void HookManager::AddPostLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PostLaunchScene, func, arg, temp);
}

void HookManager::RemovePostLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PostLaunchScene, func, arg);
}

void HookManager::AddOnCKInitCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKInit, func, arg, temp);
}

void HookManager::RemoveOnCKInitCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKInit, func, arg);
}

void HookManager::AddOnCKEndCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKEnd, func, arg, temp);
}

void HookManager::RemoveOnCKEndCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKEnd, func, arg);
}

void HookManager::AddOnCKResetCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKReset, func, arg, temp);
}

void HookManager::RemoveOnCKResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKReset, func, arg);
}

void HookManager::AddOnCKPostResetCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKPostReset, func, arg, temp);
}

void HookManager::RemoveOnCKPostResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKPostReset, func, arg);
}

void HookManager::AddOnCKPauseCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKPause, func, arg, temp);
}

void HookManager::RemoveOnCKPauseCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKPause, func, arg);
}

void HookManager::AddOnCKPlayCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnCKPlay, func, arg, temp);
}

void HookManager::RemoveOnCKPlayCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnCKPlay, func, arg);
}

void HookManager::AddOnSequenceToBeDeletedCallBack(CK_DELETECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnSequenceToBeDeleted, func, arg, temp);
}

void HookManager::RemoveOnSequenceToBeDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnSequenceToBeDeleted, func, arg);
}

void HookManager::AddOnSequenceDeletedCallBack(CK_DELETECALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnSequenceDeleted, func, arg, temp);
}

void HookManager::RemoveOnSequenceDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnSequenceDeleted, func, arg);
}

void HookManager::AddPreLoadCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PreLoad, func, arg, temp);
}

void HookManager::RemovePreLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PreLoad, func, arg);
}

void HookManager::AddPostLoadCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PostLoad, func, arg, temp);
}

void HookManager::RemovePostLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PostLoad, func, arg);
}

void HookManager::AddPreSaveCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PreSave, func, arg, temp);
}

void HookManager::RemovePreSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PreSave, func, arg);
}

void HookManager::AddPostSaveCallBack(CK_PROCESSCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(PostSave, func, arg, temp);
}

void HookManager::RemovePostSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(PostSave, func, arg);
}

void HookManager::AddOnPreCopyCallBack(CK_COPYCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnPreCopy, func, arg, temp);
}

void HookManager::RemoveOnPreCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnPreCopy, func, arg);
}

void HookManager::AddOnPostCopyCallBack(CK_COPYCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnPostCopy, func, arg, temp);
}

void HookManager::RemoveOnPostCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnPostCopy, func, arg);
}

void HookManager::AddOnPreRenderCallBack(CK_RENDERCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnPreRender, func, arg, temp);
}

void HookManager::RemoveOnPreRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnPreRender, func, arg);
}

void HookManager::AddOnPostRenderCallBack(CK_RENDERCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnPostRender, func, arg, temp);
}

void HookManager::RemoveOnPostRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnPostRender, func, arg);
}

void HookManager::AddOnPostSpriteRenderCallBack(CK_RENDERCALLBACK func, void *arg, CKBOOL temp) {
    if (!func) return;
    HM_ADD_CALLBACK(OnPostSpriteRender, func, arg, temp);
}

void HookManager::RemoveOnPostSpriteRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    if (!func) return;
    HM_REMOVE_CALLBACK(OnPostSpriteRender, func, arg);
}

#undef HM_ADD_CALLBACK
#undef HM_REMOVE_CALLBACK