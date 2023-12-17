#ifndef HOOKS_FILEHOOK_H
#define HOOKS_FILEHOOK_H

#include "CKFile.h"

#include "Macros.h"
#include "Hooks.h"

#define CP_DECLARE_FILE_METHOD_HOOK(Ret, Name, Args) CP_DECLARE_METHOD_HOOK(CKFile, Ret, Name, Args)

CP_HOOK_CLASS(CKFile) {
public:
    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, OpenFile, (CKSTRING filename, CK_LOAD_FLAGS flags));

    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, OpenMemory, (void *MemoryBuffer, int BufferSize, CK_LOAD_FLAGS flags));

    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, LoadFileData, (CKObjectArray *list));

    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, LoadFile, (CKSTRING filename, CKObjectArray *list, CK_LOAD_FLAGS flags));
    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, LoadMemory, (void *MemoryBuffer, int BufferSize, CKObjectArray *list, CK_LOAD_FLAGS flags));

    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, StartSave, (CKSTRING filename, CKDWORD flags));
    CP_DECLARE_FILE_METHOD_HOOK(void, SaveObject, (CKObject *obj, CKDWORD flags));
    CP_DECLARE_FILE_METHOD_HOOK(void, SaveObjects, (CKObjectArray *array, CKDWORD flags));
    CP_DECLARE_FILE_METHOD_HOOK(void, SaveObjects2, (CK_ID *ids, int count, CKDWORD flags));
    CP_DECLARE_FILE_METHOD_HOOK(void, SaveObjects3, (CKObject **objs, int count, CKDWORD flags));
    CP_DECLARE_FILE_METHOD_HOOK(void, SaveObjectAsReference, (CKObject *obj));
    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, EndSave, ());

    CP_DECLARE_FILE_METHOD_HOOK(CKBOOL, IncludeFile, (CKSTRING FileName, int SearchPathCategory));

    CP_DECLARE_FILE_METHOD_HOOK(CKBOOL, IsObjectToBeSaved, (CK_ID iID));

    CP_DECLARE_FILE_METHOD_HOOK(void, LoadAndSave, (CKSTRING filename, CKSTRING filename_new));
    CP_DECLARE_FILE_METHOD_HOOK(void, RemapManagerInt, (CKGUID Manager, int *ConversionTable, int TableSize));

    CP_DECLARE_FILE_METHOD_HOOK(void, ClearData, ());

    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, ReadFileHeaders, (CKBufferParser **ParserPtr));
    CP_DECLARE_FILE_METHOD_HOOK(CKERROR, ReadFileData, (CKBufferParser **ParserPtr));
    CP_DECLARE_FILE_METHOD_HOOK(void, FinishLoading, (CKObjectArray *list, CKDWORD flags));

    CP_DECLARE_FILE_METHOD_HOOK(CKObject *, ResolveReference, (CKFileObject *Data));

    static bool InitHooks(HookApi *api);
    static void ShutdownHooks(HookApi *api);
};

#endif /* HOOKS_FILEHOOK_H */
