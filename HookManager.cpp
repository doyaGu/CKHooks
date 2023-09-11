#include "HookManager.h"

HookManager::HookManager(CKContext *context) : CKBaseManager(context, HOOKMANAGER_GUID, "Hook Manager") {
    context->RegisterNewManager(this);
}

HookManager::~HookManager() = default;

CKERROR HookManager::PreClearAll() {
    m_PreClearAllCallbacks();
    return CK_OK;
}

CKERROR HookManager::PostClearAll() {
    m_PostClearAllCallbacks();
    return CK_OK;
}

CKERROR HookManager::PreProcess() {
    m_PreProcessCallbacks();
    return CK_OK;
}

CKERROR HookManager::PostProcess() {
    m_PostProcessCallbacks();
    return CK_OK;
}

CKERROR HookManager::SequenceAddedToScene(CKScene *scn, CK_ID *objids, int count) {
    m_OnSequenceAddedToSceneCallbacks(scn, objids, count);
    return CK_OK;
}

CKERROR HookManager::SequenceRemovedFromScene(CKScene *scn, CK_ID *objids, int count) {
    m_OnSequenceRemovedFromSceneCallbacks(scn, objids, count);
    return CK_OK;
}

CKERROR HookManager::PreLaunchScene(CKScene *OldScene, CKScene *NewScene) {
    m_PreLaunchSceneCallbacks(OldScene, NewScene);
    return CK_OK;
}

CKERROR HookManager::PostLaunchScene(CKScene *OldScene, CKScene *NewScene) {
    m_PostLaunchSceneCallbacks(OldScene, NewScene);
    return CK_OK;
}

CKERROR HookManager::OnCKInit() {
    m_OnCKInitCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnCKEnd() {
    m_OnCKEndCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnCKReset() {
    m_OnCKResetCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnCKPostReset() {
    m_OnCKPostResetCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnCKPause() {
    m_OnCKPauseCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnCKPlay() {
    m_OnCKPlayCallbacks();
    return CK_OK;
}

CKERROR HookManager::SequenceToBeDeleted(CK_ID *objids, int count) {
    m_OnSequenceToBeDeletedCallbacks(objids, count);
    return CK_OK;
}

CKERROR HookManager::SequenceDeleted(CK_ID *objids, int count) {
    m_OnSequenceDeletedCallbacks(objids, count);
    return CK_OK;
}

CKERROR HookManager::PreLoad() {
    m_PreLoadCallbacks();
    return CK_OK;
}

CKERROR HookManager::PostLoad() {
    m_PostLoadCallbacks();
    return CK_OK;
}

CKERROR HookManager::PreSave() {
    m_PreSaveCallbacks();
    return CK_OK;
}

CKERROR HookManager::PostSave() {
    m_PostSaveCallbacks();
    return CK_OK;
}

CKERROR HookManager::OnPreCopy(CKDependenciesContext &context) {
    m_OnPreCopyCallbacks(&context);
    return CK_OK;
}

CKERROR HookManager::OnPostCopy(CKDependenciesContext &context) {
    m_OnPostCopyCallbacks(&context);
    return CK_OK;
}

CKERROR HookManager::OnPreRender(CKRenderContext *dev) {
    m_OnPreRenderCallbacks(dev);
    return CK_OK;
}

CKERROR HookManager::OnPostRender(CKRenderContext *dev) {
    m_OnPostRenderCallbacks(dev);
    return CK_OK;
}

CKERROR HookManager::OnPostSpriteRender(CKRenderContext *dev) {
    m_OnPostSpriteRenderCallbacks(dev);
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

void HookManager::AddPreClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreClearAllCallbacks.Append(func, arg);
}

void HookManager::RemovePreClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreClearAllCallbacks.Remove(func, arg);
}

void HookManager::AddPostClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostClearAllCallbacks.Append(func, arg);
}

void HookManager::RemovePostClearAllCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostClearAllCallbacks.Remove(func, arg);
}

void HookManager::AddPreProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreProcessCallbacks.Append(func, arg);
}

void HookManager::RemovePreProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreProcessCallbacks.Remove(func, arg);
}

void HookManager::AddPostProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostProcessCallbacks.Append(func, arg);
}

void HookManager::RemovePostProcessCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostProcessCallbacks.Remove(func, arg);
}

void HookManager::AddOnSequenceAddedToSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    m_OnSequenceAddedToSceneCallbacks.Append(func, arg);
}

void HookManager::RemoveOnSequenceAddedToSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    m_OnSequenceAddedToSceneCallbacks.Remove(func, arg);
}

void HookManager::AddOnSequenceRemovedFromSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    m_OnSequenceRemovedFromSceneCallbacks.Append(func, arg);
}

void HookManager::RemoveOnSequenceRemovedFromSceneCallBack(CK_SCENECALLBACK func, void *arg) {
    m_OnSequenceRemovedFromSceneCallbacks.Remove(func, arg);
}

void HookManager::AddPreLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    m_PreLaunchSceneCallbacks.Append(func, arg);
}

void HookManager::RemovePreLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    m_PreLaunchSceneCallbacks.Remove(func, arg);
}

void HookManager::AddPostLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    m_PostLaunchSceneCallbacks.Append(func, arg);
}

void HookManager::RemovePostLaunchSceneCallBack(CK_LAUNCHSCENECALLBACK func, void *arg) {
    m_PostLaunchSceneCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKInitCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKInitCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKInitCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKInitCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKEndCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKEndCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKEndCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKEndCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKResetCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKResetCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKPostResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPostResetCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKPostResetCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPostResetCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKPauseCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPauseCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKPauseCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPauseCallbacks.Remove(func, arg);
}

void HookManager::AddOnCKPlayCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPlayCallbacks.Append(func, arg);
}

void HookManager::RemoveOnCKPlayCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_OnCKPlayCallbacks.Remove(func, arg);
}

void HookManager::AddOnSequenceToBeDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    m_OnSequenceToBeDeletedCallbacks.Append(func, arg);
}

void HookManager::RemoveOnSequenceToBeDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    m_OnSequenceToBeDeletedCallbacks.Remove(func, arg);
}

void HookManager::AddOnSequenceDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    m_OnSequenceDeletedCallbacks.Append(func, arg);
}

void HookManager::RemoveOnSequenceDeletedCallBack(CK_DELETECALLBACK func, void *arg) {
    m_OnSequenceDeletedCallbacks.Remove(func, arg);
}

void HookManager::AddPreLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreLoadCallbacks.Append(func, arg);
}

void HookManager::RemovePreLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreLoadCallbacks.Remove(func, arg);
}

void HookManager::AddPostLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostLoadCallbacks.Append(func, arg);
}

void HookManager::RemovePostLoadCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostLoadCallbacks.Remove(func, arg);
}

void HookManager::AddPreSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreSaveCallbacks.Append(func, arg);
}

void HookManager::RemovePreSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PreSaveCallbacks.Remove(func, arg);
}

void HookManager::AddPostSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostSaveCallbacks.Append(func, arg);
}

void HookManager::RemovePostSaveCallBack(CK_PROCESSCALLBACK func, void *arg) {
    m_PostSaveCallbacks.Remove(func, arg);
}

void HookManager::AddOnPreCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    m_OnPreCopyCallbacks.Append(func, arg);
}

void HookManager::RemoveOnPreCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    m_OnPreCopyCallbacks.Remove(func, arg);
}

void HookManager::AddOnPostCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    m_OnPostCopyCallbacks.Append(func, arg);
}

void HookManager::RemoveOnPostCopyCallBack(CK_COPYCALLBACK func, void *arg) {
    m_OnPostCopyCallbacks.Remove(func, arg);
}

void HookManager::AddOnPreRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPreRenderCallbacks.Append(func, arg);
}

void HookManager::RemoveOnPreRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPreRenderCallbacks.Remove(func, arg);
}

void HookManager::AddOnPostRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPostRenderCallbacks.Append(func, arg);
}

void HookManager::RemoveOnPostRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPostRenderCallbacks.Remove(func, arg);
}

void HookManager::AddOnPostSpriteRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPostSpriteRenderCallbacks.Append(func, arg);
}

void HookManager::RemoveOnPostSpriteRenderCallBack(CK_RENDERCALLBACK func, void *arg) {
    m_OnPostSpriteRenderCallbacks.Remove(func, arg);
}