#include "FileHook.h"

#include "VxMeMoryMappedFile.h"
#include "CKPluginManager.h"
#include "CKPathManager.h"
#include "CKParameterManager.h"
#include "CKBehavior.h"
#include "CKBeObject.h"
#include "CKInterfaceObjectManager.h"

#include "Utils.h"

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, OpenFile);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, OpenMemory);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, LoadFileData);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, LoadFile);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, LoadMemory);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, StartSave);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, SaveObject);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, SaveObjects);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, SaveObjects2);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, SaveObjects3);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, SaveObjectAsReference);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, EndSave);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, IncludeFile);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, IsObjectToBeSaved);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, LoadAndSave);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, RemapManagerInt);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, ClearData);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, ReadFileHeaders);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, ReadFileData);
CP_DEFINE_METHOD_HOOK_PTRS(CKFile, FinishLoading);

CP_DEFINE_METHOD_HOOK_PTRS(CKFile, ResolveReference);

#define CP_FILE_METHOD_NAME(Name) CP_HOOK_CLASS_NAME(CKFile)::CP_FUNC_NAME(Name)

#if CKVERSION == 0x13022002
typedef void (CKBehavior::*CKBehaviorApplyPatchLoadFunc)();
static CKBehaviorApplyPatchLoadFunc g_CKBehaviorApplyPatchLoadFunc = nullptr;

typedef void (CKBeObject::*CKBeObjectApplyOwnerFunc)();
static CKBeObjectApplyOwnerFunc g_CKBeObjectApplyOwnerFunc = nullptr;

typedef void (XClassArray<XString>::*ClearStringArrayFunc)();
static ClearStringArrayFunc g_ClearStringArrayFunc = nullptr;

typedef void (XClassArray<XString>::*ResizeStringArrayFunc)(int size);
static ResizeStringArrayFunc g_ResizeStringArrayFunc = nullptr;

typedef void (XClassArray<CKFilePluginDependencies>::*ResizePluginsDepsArrayFunc)(int size);
static ResizePluginsDepsArrayFunc g_ResizePluginsDepsArrayFunc = nullptr;

typedef void (XClassArray<XArray<int>>::*ClearIndexByClassIdArrayFunc)();
static ClearIndexByClassIdArrayFunc g_ClearIndexByClassIdArrayFunc = nullptr;

typedef void (XClassArray<XArray<int>>::*ResizeIndexByClassIdArrayFunc)(int size);
static ResizeIndexByClassIdArrayFunc g_ResizeIndexByClassIdArrayFunc = nullptr;

static int *g_MaxClassID = nullptr;
static CKDWORD *g_CurrentFileVersion = nullptr;
static CKDWORD *g_CurrentFileWriteMode = nullptr;
static CKBOOL *g_WarningForOlderVersion = nullptr;

CKBufferParser::CKBufferParser(void *Buffer, int Size)
    : m_Valid(FALSE),
      m_CursorPos(0),
      m_Buffer((char *)Buffer),
      m_Size(Size) {}

CKBufferParser::~CKBufferParser() {
    if (m_Valid)
        VxFree(m_Buffer);
}

CKBOOL CKBufferParser::Write(void *x, int size) {
    memcpy(&m_Buffer[m_CursorPos], x, size);
    m_CursorPos += size;
    return TRUE;
}

CKBOOL CKBufferParser::Read(void *x, int size) {
    memcpy(x, &m_Buffer[m_CursorPos], size);
    m_CursorPos += size;
    return TRUE;
}

char *CKBufferParser::ReadString() {
    int len = ReadInt();
    if (len == 0)
        return nullptr;

    char *str = new (VxMalloc(sizeof(char) * (len + 1))) char[len + 1];
    memset(str, 0, len + 1);
    memcpy(str, &m_Buffer[m_CursorPos], len);
    m_CursorPos += len;
    return str;
}

int CKBufferParser::ReadInt() {
    int val;
    memcpy(&val, &m_Buffer[m_CursorPos], sizeof(int));
    m_CursorPos += sizeof(int);
    return val;
}

void CKBufferParser::Seek(int Pos) {
    m_CursorPos = Pos;
}

void CKBufferParser::Skip(int Offset) {
    m_CursorPos += Offset;
}

CKBOOL CKBufferParser::IsValid() {
    return m_Valid;
}

int CKBufferParser::Size() {
    return m_Size;
}

int CKBufferParser::CursorPos() {
    return m_CursorPos;
}

CKStateChunk *CKBufferParser::ExtractChunk(int Size, CKFile *f) {
    auto *chunk = CreateCKStateChunk(0, f);
    if (!chunk->ConvertFromBuffer(&m_Buffer[m_CursorPos])) {
        VxDelete<CKStateChunk>(chunk);
        chunk = nullptr;
    }
    m_CursorPos += Size;
    return chunk;
}

void CKBufferParser::ExtractChunk(int Size, CKFile *f, CKFileChunk *chunk) {}

CKDWORD CKBufferParser::ComputeCRC(int Size, CKDWORD PrevCRC) {
    CKDWORD crc = CKComputeDataCRC(&m_Buffer[m_CursorPos], Size, PrevCRC);
    m_CursorPos += Size;
    return crc;
}

CKBufferParser *CKBufferParser::Extract(int Size) {
    auto *parser = new (VxMalloc(sizeof(CKBufferParser))) CKBufferParser(nullptr, Size);
    parser->Write(&m_Buffer[m_CursorPos], Size);
    return parser;
}

CKBOOL CKBufferParser::ExtractFile(char *Filename, int Size) {
    FILE *fp = fopen(Filename, "wb");
    if (!fp)
        return FALSE;
    fwrite(&m_Buffer[m_CursorPos], sizeof(char), Size, fp);
    fclose(fp);
    m_CursorPos += Size;
    return TRUE;
}

CKBufferParser *CKBufferParser::ExtractDecoded(int Size, CKDWORD *Key) { return nullptr; }

CKBufferParser *CKBufferParser::UnPack(int UnpackSize, int PackSize) {
    char *buffer = CKUnPackData(UnpackSize, &m_Buffer[m_CursorPos], PackSize);
    if (!buffer) {
        return nullptr;
    }

    auto *parser = new (VxMalloc(sizeof(CKBufferParser))) CKBufferParser(buffer, UnpackSize);
    parser->m_Valid = TRUE;
    return parser;
}

void CKBufferParser::InsertChunk(CKStateChunk *chunk) {
    int size = 0;
    if (chunk)
        size = chunk->ConvertToBuffer(nullptr);
    Write(&size, sizeof(int));
    if (chunk)
        chunk->ConvertToBuffer(&m_Buffer[m_CursorPos]);
    m_CursorPos += size;
}

CKBufferParser *CKBufferParser::Pack(int Size, int CompressionLevel) {
    if (Size <= 0)
        return nullptr;

    int newSize = 0;
    char *buffer = CKPackData(&m_Buffer[m_CursorPos], Size, newSize, CompressionLevel);
    if (!buffer) {
        return nullptr;
    }

    auto *parser = new (VxMalloc(sizeof(CKBufferParser))) CKBufferParser(buffer, newSize);
    parser->m_Valid = TRUE;
    return parser;
}

void CKBufferParser::Encode(int Size, CKDWORD *Key) {}

#endif

CKERROR CP_FILE_METHOD_NAME(OpenFile)(CKSTRING filename, CK_LOAD_FLAGS flags) {
#if CKVERSION == 0x13022002
    ClearData();

    if (!filename) {
        return CKERR_INVALIDPARAMETER;
    }

    m_FileName = CKStrdup(filename);
    m_MappedFile = new (VxMalloc(sizeof(VxMemoryMappedFile))) VxMemoryMappedFile(m_FileName);
    if (m_MappedFile->GetErrorType() != CK_OK) {
        return CKERR_INVALIDFILE;
    }

    m_Context->SetLastCmoLoaded(filename);
    return OpenMemory(m_MappedFile->GetBase(), m_MappedFile->GetFileSize(), flags);
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(OpenFile, filename, flags);
#endif
}

CKERROR CP_FILE_METHOD_NAME(OpenMemory)(void *MemoryBuffer, int BufferSize, CK_LOAD_FLAGS flags) {
#if CKVERSION == 0x13022002
    if (!MemoryBuffer) {
        return CKERR_INVALIDPARAMETER;
    }

    if (BufferSize < 32 || memcmp(MemoryBuffer, "Nemo", 4) != 0) {
        return CKERR_INVALIDFILE;
    }

    m_Parser = new (VxMalloc(sizeof(CKBufferParser))) CKBufferParser(MemoryBuffer, BufferSize);
    if (!m_Parser) {
        return CKERR_OUTOFMEMORY;
    }

    if (!m_Parser->m_Buffer) {
        VxDelete<CKBufferParser>(m_Parser);
        m_Parser = nullptr;
        return CKERR_INVALIDPARAMETER;
    }

    *g_WarningForOlderVersion = FALSE;
    m_Flags = flags;

    // m_IndexByClassId.Resize(*g_MaxClassID);
    CP_CALL_METHOD(m_IndexByClassId, g_ResizeIndexByClassIdArrayFunc, *g_MaxClassID);

    return ReadFileHeaders(&m_Parser);
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(OpenMemory, MemoryBuffer, BufferSize, flags);
#endif
}

CKERROR CP_FILE_METHOD_NAME(LoadFileData)(CKObjectArray *list) {
#if CKVERSION == 0x13022002
    CKERROR err = CK_OK;

    if (!m_Parser && !m_ReadFileDataDone)
        return CKERR_INVALIDFILE;

    for (XArray<CKBaseManager *>::Iterator it = m_Context->m_ManagersPreLoad.Begin(); it != m_Context->m_ManagersPreLoad.End(); ++it) {
        CKBaseManager *manager = *it;
        m_Context->m_CurrentManager = manager;
        manager->PreLoad();
    }
    m_Context->m_CurrentManager = nullptr;
    m_Context->m_InLoad = TRUE;

    if (m_ReadFileDataDone) {
        FinishLoading(list, m_Flags);
        if (*g_WarningForOlderVersion)
            m_Context->OutputToConsole((CKSTRING) "Obsolete File Format,Please Re-Save...");
    } else {
        err = ReadFileData(&m_Parser);
        if (err == CK_OK) {
            if (m_Parser) {
                VxDelete<CKBufferParser>(m_Parser);
                m_Parser = nullptr;
            }

            if (m_MappedFile) {
                VxDelete<VxMemoryMappedFile>(m_MappedFile);
                m_MappedFile = nullptr;
            }

            FinishLoading(list, m_Flags);
            if (*g_WarningForOlderVersion)
                m_Context->OutputToConsole((CKSTRING)"Obsolete File Format,Please Re-Save...");
        }
    }
    
    m_Context->SetAutomaticLoadMode(CKLOAD_INVALID, CKLOAD_INVALID, CKLOAD_INVALID, CKLOAD_INVALID);
    m_Context->SetUserLoadCallback(nullptr, nullptr);

    if (m_Parser) {
        VxDelete<CKBufferParser>(m_Parser);
        m_Parser = nullptr;
    }

    if (m_MappedFile) {
        VxDelete<VxMemoryMappedFile>(m_MappedFile);
        m_MappedFile = nullptr;
    }

    for (XArray<CKBaseManager *>::Iterator it = m_Context->m_ManagersPostLoad.Begin(); it != m_Context->m_ManagersPostLoad.End(); ++it) {
        CKBaseManager *manager = *it;
        m_Context->m_CurrentManager = manager;
        manager->PostLoad();
    }
    m_Context->m_CurrentManager = nullptr;
    m_Context->m_InLoad = FALSE;

    return err;
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(LoadFileData, list);
#endif
}

CKERROR CP_FILE_METHOD_NAME(LoadFile)(CKSTRING filename, CKObjectArray *list, CK_LOAD_FLAGS flags) {
#if CKVERSION == 0x13022002
    CKERROR err = OpenFile(filename, flags);
    if (err != CK_OK && err != CKERR_PLUGINSMISSING)
        return err;

    m_Context->SetLastCmoLoaded(filename);
    return LoadFileData(list);
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(LoadFile, filename, list, flags);
#endif
}

CKERROR CP_FILE_METHOD_NAME(LoadMemory)(void *MemoryBuffer, int BufferSize, CKObjectArray *list, CK_LOAD_FLAGS flags) {
#if CKVERSION == 0x13022002
    CKERROR err = OpenMemory(MemoryBuffer, BufferSize, flags);
    if (err != CK_OK && err != CKERR_PLUGINSMISSING)
        return err;

    return LoadFileData(list);
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(LoadMemory, MemoryBuffer, BufferSize, list, flags);
#endif
}

CKERROR CP_FILE_METHOD_NAME(StartSave)(CKSTRING filename, CKDWORD flags) {
    return CP_CALL_METHOD_ORIG(StartSave, filename, flags);
}

void CP_FILE_METHOD_NAME(SaveObject)(CKObject *obj, CKDWORD flags) {
    CP_CALL_METHOD_ORIG(SaveObject, obj, flags);
}

void CP_FILE_METHOD_NAME(SaveObjects)(CKObjectArray *array, CKDWORD flags) {
    CP_CALL_METHOD_ORIG(SaveObjects, array, flags);
}

void CP_FILE_METHOD_NAME(SaveObjects2)(CK_ID *ids, int count, CKDWORD flags) {
    CP_CALL_METHOD_ORIG(SaveObjects2, ids, count, flags);
}

void CP_FILE_METHOD_NAME(SaveObjects3)(CKObject **objs, int count, CKDWORD flags) {
    CP_CALL_METHOD_ORIG(SaveObjects3, objs, count, flags);
}

void CP_FILE_METHOD_NAME(SaveObjectAsReference)(CKObject *obj) {
    CP_CALL_METHOD_ORIG(SaveObjectAsReference, obj);
}

CKERROR CP_FILE_METHOD_NAME(EndSave)() {
    return CP_CALL_METHOD_ORIG(EndSave);
}

CKBOOL CP_FILE_METHOD_NAME(IncludeFile)(CKSTRING FileName, int SearchPathCategory) {
    return CP_CALL_METHOD_ORIG(IncludeFile, FileName, SearchPathCategory);
}

CKBOOL CP_FILE_METHOD_NAME(IsObjectToBeSaved)(CK_ID iID) {
#if CKVERSION == 0x13022002
    if (m_FileObjects.IsEmpty())
        return FALSE;

    for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it)
        if ((*it).ObjectCid == iID)
            return TRUE;

    return FALSE;
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(IsObjectToBeSaved, iID);
#endif
}

void CP_FILE_METHOD_NAME(LoadAndSave)(CKSTRING filename, CKSTRING filename_new) {
    CP_CALL_METHOD_ORIG(LoadAndSave, filename, filename_new);
}

void CP_FILE_METHOD_NAME(RemapManagerInt)(CKGUID Manager, int *ConversionTable, int TableSize) {
    if (m_FileObjects.IsEmpty())
        return;

    for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
        if (it->Data)
            it->Data->RemapManagerInt(Manager, ConversionTable, TableSize);
    }
}

void CP_FILE_METHOD_NAME(ClearData)() {
#if CKVERSION == 0x13022002
    for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin();
         it != m_FileObjects.End(); ++it) {
        if (it->Data) {
            VxDelete<CKStateChunk>(it->Data);
            it->Data = nullptr;
        }
        CKDeletePointer(it->Name);
        it->Name = nullptr;
    }

    for (XArray<CKFileManagerData>::Iterator it = m_ManagersData.Begin();
         it != m_ManagersData.End(); ++it) {
        if (it->data) {
            VxDelete<CKStateChunk>(it->data);
            it->data = nullptr;
        }
    }

    m_FileObjects.Clear();
    m_ManagersData.Clear();
    m_AlreadySavedMask.Clear();
    m_AlreadyReferencedMask.Clear();
    m_ReferencedObjects.Clear();

    // m_IndexByClassId.Clear();
    CP_CALL_METHOD(m_IndexByClassId, g_ClearIndexByClassIdArrayFunc);

    CKDeletePointer(m_FileName);
    m_FileName = nullptr;

    if (m_Parser) {
        VxDelete<CKBufferParser>(m_Parser);
        m_Parser = nullptr;
    }

    if (m_MappedFile) {
        VxDelete<VxMemoryMappedFile>(m_MappedFile);
        m_MappedFile = nullptr;
    }

    m_Flags = 0;
    m_SaveIDMax = 0;
#elif CKVERSION == 0x05082002
    CP_CALL_METHOD_ORIG(ClearData);
#endif
}

CKERROR CP_FILE_METHOD_NAME(ReadFileHeaders)(CKBufferParser **ParserPtr) {
#if CKVERSION == 0x13022002
    CKBufferParser *parser = *ParserPtr;

    // m_IncludedFiles.Clear();
    CP_CALL_METHOD(m_IncludedFiles, g_ClearStringArrayFunc);

    if (parser->Size() < 32)
        return CKERR_INVALIDFILE;

    union CKFileHdr0 {
        struct {
            CKDWORD Signature0;
            CKDWORD Signature1;
            CKDWORD Crc;
            CKDWORD CKVersion;
            CKDWORD FileVersion;
            CKDWORD FileVersion2;
            CKDWORD FileWriteMode;
            CKDWORD Hdr1PackSize;
        };
        CKDWORD d[8];
        char b[32];
    } hdr0;

    parser->Read(hdr0.d, sizeof(hdr0));

    if (hdr0.FileVersion2 != 0) {
        memset(hdr0.d, 0, sizeof(hdr0));
        *g_WarningForOlderVersion = TRUE;
    }

    if (hdr0.FileVersion >= 10) {
        m_Context->OutputToConsole((CKSTRING)"This version is too old to load this file");
        return CKERR_OBSOLETEVIRTOOLS;
    }

    union CKFileHdr1 {
        struct {
            CKDWORD DataPackSize;
            CKDWORD DataUnPackSize;
            CKDWORD ManagerCount;
            CKDWORD ObjectCount;
            CKDWORD MaxIDSaved;
            CKDWORD ProductVersion;
            CKDWORD ProductBuild;
            CKDWORD Hdr1UnPackSize;
        };
        CKDWORD d[8];
        char b[32];
    } hdr1;

    if (hdr0.FileVersion < 5) {
        memset(hdr1.b, 0, sizeof(hdr1));
    } else if (parser->Size() >= 64) {
        parser->Read(hdr1.b, sizeof(hdr1));
    } else {
        return CKERR_INVALIDFILE;
    }

    if (hdr1.ProductVersion >= 12) {
        hdr1.ProductVersion = 0;
        hdr1.ProductBuild = 0x1010000;
    }

    m_FileInfo.ProductVersion = hdr1.ProductVersion;
    m_FileInfo.ProductBuild = hdr1.ProductBuild;
    m_FileInfo.FileWriteMode = hdr0.FileWriteMode;
    m_FileInfo.CKVersion = hdr0.CKVersion;
    m_FileInfo.FileVersion = hdr0.FileVersion;
    m_FileInfo.FileSize = parser->Size();
    m_FileInfo.ManagerCount = hdr1.ManagerCount;
    m_FileInfo.ObjectCount = hdr1.ObjectCount;
    m_FileInfo.MaxIDSaved = hdr1.MaxIDSaved;
    m_FileInfo.Hdr1PackSize = hdr0.Hdr1PackSize;
    m_FileInfo.Hdr1UnPackSize = hdr1.Hdr1UnPackSize;
    m_FileInfo.DataPackSize = hdr1.DataPackSize;
    m_FileInfo.DataUnPackSize = hdr1.DataUnPackSize;
    m_FileInfo.Crc = hdr0.Crc;

    if (hdr0.FileVersion >= 8) {
        hdr0.Crc = 0;
        CKDWORD crc = CKComputeDataCRC(hdr0.b, sizeof(hdr0), 0);
        int prev = parser->CursorPos();
        parser->Seek(sizeof(hdr0));
        crc = parser->ComputeCRC(sizeof(hdr1), crc);
        crc = parser->ComputeCRC(m_FileInfo.Hdr1PackSize, crc);
        crc = parser->ComputeCRC(m_FileInfo.DataPackSize, crc);
        parser->Seek(prev);
        if (crc != m_FileInfo.Crc) {
            m_Context->OutputToConsole((CKSTRING)"Crc Error in m_File");
            return CKERR_FILECRCERROR;
        }

        if (m_FileInfo.Hdr1PackSize != m_FileInfo.Hdr1UnPackSize) {
            parser = parser->UnPack(m_FileInfo.Hdr1UnPackSize, m_FileInfo.Hdr1PackSize);
        }
    }

    if (m_FileInfo.FileVersion >= 7) {
        m_SaveIDMax = m_FileInfo.MaxIDSaved;
        m_FileObjects.Resize(m_FileInfo.ObjectCount);
        for (XArray<CKFileObject>::Iterator oit = m_FileObjects.Begin(); oit != m_FileObjects.End(); ++oit) {
            oit->ObjPtr = nullptr;
            oit->Name = nullptr;
            oit->Data = nullptr;
            oit->Object = parser->ReadInt();
            oit->ObjectCid = parser->ReadInt();
            oit->FileIndex = parser->ReadInt();
            oit->Name = parser->ReadString();
        }
    }

    CKBOOL noPluginMissing = TRUE;

    if (m_FileInfo.FileVersion >= 8) {
        const int pluginsDepCount = parser->ReadInt();
        // m_PluginsDep.Resize(pluginsDepCount);
        CP_CALL_METHOD(m_PluginsDep, g_ResizePluginsDepsArrayFunc, pluginsDepCount);
        for (XArray<CKFilePluginDependencies>::Iterator pit = m_PluginsDep.Begin(); pit != m_PluginsDep.End(); ++pit) {
            pit->m_PluginCategory = parser->ReadInt();

            const int count = parser->ReadInt();
            pit->m_Guids.Resize(count);

            parser->Read(&pit->m_Guids[0], sizeof(CKGUID) * count);

            if (m_Flags & 0x80) {
                for (int j = 0; j < count; ++j) {
                    CKGUID guid = pit->m_Guids[j];
                    CKPluginEntry *entry = CKGetPluginManager()->FindComponent(guid, pit->m_PluginCategory);
                    if (entry) {
                        pit->ValidGuids.Set(j);
                    } else {
                        noPluginMissing = FALSE;
                        pit->ValidGuids.Unset(j);
                    }
                }
            }
        }

        int includedFileSize = parser->ReadInt();
        if (includedFileSize > 0) {
            int includedFileCount = parser->ReadInt();
            // m_IncludedFiles.Resize(includedFileCount);
            CP_CALL_METHOD(m_IncludedFiles, g_ResizeStringArrayFunc, includedFileCount);
            includedFileSize -= 4;
        }
        parser->Skip(includedFileSize);
    }

    if (parser != *ParserPtr) {
        VxDelete<CKBufferParser>(parser);
        parser = *ParserPtr;
        parser->Skip(m_FileInfo.Hdr1PackSize);
    }

    *g_CurrentFileVersion = hdr0.FileVersion;
    *g_CurrentFileWriteMode = hdr0.FileWriteMode;

    if ((m_Flags & 0x80) && m_FileInfo.FileVersion < 8) {
        m_ReadFileDataDone = TRUE;
        CKERROR err = ReadFileData(&m_Parser);

        if (m_Parser) {
            VxDelete<CKBufferParser>(m_Parser);
            m_Parser = nullptr;
        }

        if (m_MappedFile) {
            VxDelete<VxMemoryMappedFile>(m_MappedFile);
            m_MappedFile = nullptr;
        }

        if (err != CK_OK) {
            m_Context->SetAutomaticLoadMode(CKLOAD_INVALID, CKLOAD_INVALID, CKLOAD_INVALID, CKLOAD_INVALID);
            m_Context->SetUserLoadCallback(nullptr, nullptr);
            m_Context->m_InLoad = FALSE;
            return err;
        }

        // m_PluginsDep.Resize(2);
        CP_CALL_METHOD(m_PluginsDep, g_ResizePluginsDepsArrayFunc, 2);

        m_PluginsDep[0].m_PluginCategory = CKPLUGIN_BEHAVIOR_DLL;
        m_PluginsDep[1].m_PluginCategory = CKPLUGIN_MANAGER_DLL;

        for (XArray<CKFileObject>::Iterator oit = m_FileObjects.Begin(); oit != m_FileObjects.End(); ++oit) {
            CKStateChunk *chunk = oit->Data;
            if (chunk && oit->ObjectCid == CKCID_BEHAVIOR) {
                CKGUID behGuid;

                chunk->StartRead();
                if (chunk->SeekIdentifier(CK_STATESAVE_BEHAVIORPROTOGUID) != 0) {
                    behGuid = chunk->ReadGuid();
                } else {
                    if (chunk->SeekIdentifier(CK_STATESAVE_BEHAVIORNEWDATA) != 0) {
                        if (chunk->GetDataVersion() < 5) {
                            behGuid = chunk->ReadGuid();
                            if ((chunk->ReadInt() & CKBEHAVIOR_BUILDINGBLOCK) == 0)
                                continue;
                        } else {
                            if ((chunk->ReadInt() & CKBEHAVIOR_BUILDINGBLOCK) == 0)
                                continue;
                            behGuid = chunk->ReadGuid();
                        }
                    }
                }

                if (behGuid.IsValid() && !CKGetPluginManager()->FindComponent(behGuid, CKPLUGIN_BEHAVIOR_DLL)) {
                    noPluginMissing = FALSE;
                    if (m_PluginsDep[0].m_Guids.Size() < m_PluginsDep[0].ValidGuids.Size()) {
                        m_PluginsDep[0].ValidGuids.Unset(m_PluginsDep[0].m_Guids.Size());
                    }
                    m_PluginsDep[0].m_Guids.PushBack(behGuid);
                }
            }
        }
    }

    return noPluginMissing ? CK_OK : CKERR_PLUGINSMISSING;
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(ReadFileHeaders, ParserPtr);
#endif
}

CKERROR CP_FILE_METHOD_NAME(ReadFileData)(CKBufferParser **ParserPtr) {

#if CKVERSION == 0x13022002
    CKBufferParser *parser = *ParserPtr;

    if (m_FileInfo.FileWriteMode & (CKFILE_CHUNKCOMPRESSED_OLD | CKFILE_WHOLECOMPRESSED)) {
        parser = parser->UnPack(m_FileInfo.DataUnPackSize, m_FileInfo.DataPackSize);
        (*ParserPtr)->Skip(m_FileInfo.DataPackSize);
    }

    if (m_FileInfo.FileVersion < 8) {
        if (m_FileInfo.FileVersion < 2) {
            *g_WarningForOlderVersion = TRUE;
        } else {
            int prev = parser->CursorPos();
            if (m_FileInfo.Crc != parser->ComputeCRC(parser->Size() - parser->CursorPos())) {
               parser->Seek(prev);
                m_Context->OutputToConsole((CKSTRING)"Crc Error in m_File");
                return CKERR_FILECRCERROR;
            }
            parser->Seek(prev);
        }

        m_SaveIDMax = parser->ReadInt();
        m_FileInfo.ObjectCount = parser->ReadInt();

        if (m_FileObjects.IsEmpty()) {
            m_FileObjects.Resize(m_FileInfo.ObjectCount);
            m_FileObjects.Memset(0);
        }
    }

    if (m_FileInfo.FileVersion >= 6) {
        if (m_FileInfo.ManagerCount > 0) {
            m_ManagersData.Resize(m_FileInfo.ManagerCount);
            for (XArray<CKFileManagerData>::Iterator mit = m_ManagersData.Begin(); mit != m_ManagersData.End(); ++mit) {
                parser->Read(mit->Manager.d, sizeof(CKGUID));
                const int managerDataSize = parser->ReadInt();
                mit->data = parser->ExtractChunk(managerDataSize, this);
            }
        }
    }

    if (m_FileInfo.ObjectCount > 0) {
        if (m_FileInfo.FileVersion >= 4) {
            for (XArray<CKFileObject>::Iterator oit = m_FileObjects.Begin(); oit != m_FileObjects.End(); ++oit) {
                if (m_FileInfo.FileVersion < 7) {
                    oit->Object = parser->ReadInt();
                }

                const int fileObjectSize = parser->ReadInt();
                if (m_FileInfo.FileVersion < 7 || (m_Flags & 0x100) == 0 || oit->ObjectCid == CKCID_BEHAVIOR) {
                    oit->Data = parser->ExtractChunk(fileObjectSize, this);
                    if (oit->Data) {
                        const int postPackSize = oit->Data->GetDataSize();
                        oit->PostPackSize = postPackSize;
                    }
                }
            }
        } else {
            const int fileObjectCount = m_FileObjects.Size();
            for (int o = 0; o < fileObjectCount; ++o) {
                CKFileObject *obj = &m_FileObjects[o];
                obj->Object = parser->ReadInt();
                const int fileObjectUnPackSize = parser->ReadInt();
                if (fileObjectUnPackSize > 0) {
                    *g_WarningForOlderVersion = TRUE;
                    const int chunkCid = parser->ReadInt();
                    obj->Data = CreateCKStateChunk(chunkCid);
                    obj->SaveFlags = parser->ReadInt();
                    const int dataSize = parser->ReadInt();
                    if (m_FileInfo.FileWriteMode & CKFILE_CHUNKCOMPRESSED_OLD) {
                        obj->Data->m_ChunkSize = dataSize;
                    } else {
                        obj->Data->m_ChunkSize = dataSize >> 2;
                    }

                    if (dataSize == fileObjectUnPackSize) {
                        obj->Data->m_ChunkSize = dataSize >> 2;
                    }

                    obj->Data->m_Data = (int *)VxMalloc(dataSize);
                    parser->Read(obj->Data->m_Data, dataSize);
                    if (dataSize != fileObjectUnPackSize && (m_FileInfo.FileWriteMode & CKFILE_CHUNKCOMPRESSED_OLD) &&
                        !obj->Data->UnPack(fileObjectUnPackSize)) {
                        if (obj->Data) {
                            VxDelete<CKStateChunk>(obj->Data);
                            obj->Data = nullptr;
                        }
                        m_Context->OutputToConsoleEx((CKSTRING)"Crc Error While Unpacking : Object=>%d \n", o);
                    }
                }
            }
        }
    }

    if (m_FileInfo.FileVersion < 7) {
        for (XArray<CKFileObject>::Iterator oit = m_FileObjects.Begin(); oit != m_FileObjects.End(); ++oit) {
            oit->Name = nullptr;
            if (oit->Data) {
                if (oit->Data->SeekIdentifier(1)) {
                    oit->Data->ReadString(&oit->Name);
                }
                oit->ObjectCid = oit->Data->GetChunkClassID();
            }
        }
    }

    if (m_IncludedFiles.Size() > 0) {
        for (XClassArray<XString>::Iterator iit = m_IncludedFiles.Begin(); iit != m_IncludedFiles.End(); ++iit) {
            const int fileNameLength = parser->ReadInt();
            char fileName[256] = "";
            if (fileNameLength > 0) {
                parser->Read(fileName, fileNameLength);
            }
            fileName[fileNameLength] = '\0';

            const int fileSize = parser->ReadInt();
            if (fileSize > 0) {
                XString temp = m_Context->GetPathManager()->GetVirtoolsTemporaryFolder();
                CKPathMaker pm(nullptr, temp.Str(), fileName, nullptr);
                char *filePath = pm.GetFileName();
                parser->ExtractFile(filePath, fileSize);
            }
        }
    }

    if (parser && parser != *ParserPtr) {
        VxDelete<CKBufferParser>(parser);
    }

    return CK_OK;
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(ReadFileData, ParserPtr);
#endif
}

void CP_FILE_METHOD_NAME(FinishLoading)(CKObjectArray *list, CKDWORD flags) {
#if CKVERSION == 0x13022002
    XBitArray exclusion;
    exclusion.Set(CKCID_PARAMETER);
    exclusion.Set(CKCID_PARAMETEROUT);
    exclusion.Set(CKCID_PARAMETERLOCAL);
    exclusion.Set(CKCID_BEHAVIOR);

    XBitArray inclusion;
    inclusion.Or(CKGetClassDesc(CKCID_BEOBJECT)->Children);
    inclusion.Or(CKGetClassDesc(CKCID_OBJECTANIMATION)->Children);
    inclusion.Or(CKGetClassDesc(CKCID_ANIMATION)->Children);

    CKObjectManager *objectManager = m_Context->m_ObjectManager;
    objectManager->StartLoadSession(m_SaveIDMax + 1);

    int options = CK_OBJECTCREATION_NONAMECHECK;
    if (flags & (CK_LOAD_DODIALOG | CK_LOAD_AUTOMATICMODE | CK_LOAD_CHECKDUPLICATES)) {
        options |= CK_OBJECTCREATION_ASK;
    }
    if (flags & CK_LOAD_AS_DYNAMIC_OBJECT) {
        options |= CK_OBJECTCREATION_DYNAMIC;
    }

    for (int i = 0; i < m_FileObjects.Size(); ++i) {
        CKFileObject *it = &m_FileObjects[i];
        m_IndexByClassId[it->ObjectCid].PushBack(i);
        if (it->ObjectCid != CKCID_RENDERCONTEXT && it->Data) {
            int id = *(int *) &it->Object;
            CKObject *obj = nullptr;
            if (id >= 0) {
                CK_CREATIONMODE res;
                obj = m_Context->CreateObject(it->ObjectCid, it->Name, (CK_OBJECTCREATION_OPTIONS) options, &res);
                it->Options = (res == CKLOAD_USECURRENT) ? CKFileObject::CK_FO_RENAMEOBJECT : CKFileObject::CK_FO_DEFAULT;
            } else {
                it->Object = -id;
                ResolveReference(it);
                it->Options = CKFileObject::CK_FO_RENAMEOBJECT;
            }
            objectManager->RegisterLoadObject(obj, it->Object);
            it->ObjPtr = obj;
            it->CreatedObject = obj->GetID();
        }
    }

    if (!m_IndexByClassId[CKCID_LEVEL].IsEmpty()) {
        if (m_FileInfo.ProductVersion <= 1 && m_FileInfo.ProductBuild <= 0x2000000) {
            m_Context->m_PVInformation = 0;
        } else {
            m_Context->m_PVInformation = m_FileInfo.ProductVersion;
        }
    }

    if ((m_Flags & 0x100) == 0) {
        for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
            if (it->Data) {
                it->Data->RemapObjects(m_Context);
            }
        }
        for (XArray<CKFileManagerData>::Iterator it = m_ManagersData.Begin(); it != m_ManagersData.End(); ++it) {
            if (it->data) {
                it->data->RemapObjects(m_Context);
            }
        }
    }

    if (!m_IndexByClassId[CKCID_LEVEL].IsEmpty()) {
        int index = m_IndexByClassId[CKCID_LEVEL][0];
        auto *level = (CKLevel *) m_FileObjects[index].ObjPtr;
        if (level && !m_Context->GetCurrentLevel()) {
            m_Context->SetCurrentLevel(level);
        }
    }

    int count = 0;

    if ((m_Flags & 0x100) == 0) {
        bool hasGridManager = false;
        for (XArray<CKFileManagerData>::Iterator it = m_ManagersData.Begin(); it != m_ManagersData.End(); ++it) {
            CKBaseManager *manager = m_Context->GetManagerByGuid(it->Manager);
            if (manager) {
                manager->LoadData(it->data, this);
                if (manager->GetGuid() == GRID_MANAGER_GUID) {
                    hasGridManager = true;
                }

                if (it->data) {
                    VxDelete<CKStateChunk>(it->data);
                    it->data = nullptr;
                }
            }
        }

        if (!hasGridManager) {
            CKBaseManager *manager = m_Context->GetManagerByGuid(GRID_MANAGER_GUID);
            manager->LoadData(nullptr, this);
        }

        bool levelLoaded = false;
        for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
            if (!it->Data)
                continue;

            if (it->Options != CKFileObject::CK_FO_DEFAULT)
                continue;

            if (exclusion.IsSet(it->ObjectCid))
                continue;

            CKObject *obj = it->ObjPtr;
            if (!obj)
                continue;

            if (CKIsChildClassOf(obj, CKCID_LEVEL)) {
                if (levelLoaded)
                    continue;
                levelLoaded = true;
            }

            obj->Load(it->Data, this);
            ++count;

            if (m_Context->m_UICallBackFct) {
                CKUICallbackStruct cbs;
                cbs.Reason = CKUIM_LOADSAVEPROGRESS;
                cbs.Param1 = count;
                cbs.Param1 = m_FileObjects.Size();
                m_Context->m_UICallBackFct(cbs, m_Context->m_InterfaceModeData);
            }

            if (list && inclusion.IsSet(it->ObjectCid)) {
                list->InsertRear(obj);
            }
        }

        for (XArray<int>::Iterator iit = m_IndexByClassId[CKCID_PARAMETERLOCAL].Begin(); iit != m_IndexByClassId[CKCID_PARAMETERLOCAL].End(); ++iit) {
            CKFileObject *it = &m_FileObjects[*iit];
            if (!it->Data || it->Options != CKFileObject::CK_FO_DEFAULT)
                continue;

            CKObject *obj = it->ObjPtr;
            if (obj) {
                obj->Load(it->Data, this);
                ++count;

                if (m_Context->m_UICallBackFct) {
                    CKUICallbackStruct cbs;
                    cbs.Reason = CKUIM_LOADSAVEPROGRESS;
                    cbs.Param1 = count;
                    cbs.Param1 = m_FileObjects.Size();
                    m_Context->m_UICallBackFct(cbs, m_Context->m_InterfaceModeData);
                }

                if (list && inclusion.IsSet(it->ObjectCid)) {
                    list->InsertRear(obj);
                }
            }

            if (it->Data) {
                VxDelete<CKStateChunk>(it->Data);
                it->Data = nullptr;
            }
        }

        for (XArray<int>::Iterator iit = m_IndexByClassId[CKCID_PARAMETER].Begin(); iit != m_IndexByClassId[CKCID_PARAMETER].End(); ++iit) {
            CKFileObject *it = &m_FileObjects[*iit];
            if (!it->Data || it->Options != CKFileObject::CK_FO_DEFAULT)
                continue;

            CKObject *obj = it->ObjPtr;
            if (obj) {
                obj->Load(it->Data, this);
                ++count;

                if (m_Context->m_UICallBackFct) {
                    CKUICallbackStruct cbs;
                    cbs.Reason = CKUIM_LOADSAVEPROGRESS;
                    cbs.Param1 = count;
                    cbs.Param1 = m_FileObjects.Size();
                    m_Context->m_UICallBackFct(cbs, m_Context->m_InterfaceModeData);
                }

                if (list && inclusion.IsSet(it->ObjectCid)) {
                    list->InsertRear(obj);
                }
            }

            if (it->Data) {
                VxDelete<CKStateChunk>(it->Data);
                it->Data = nullptr;
            }
        }

        for (XArray<int>::Iterator iit = m_IndexByClassId[CKCID_PARAMETEROUT].Begin(); iit != m_IndexByClassId[CKCID_PARAMETEROUT].End(); ++iit) {
            CKFileObject *it = &m_FileObjects[*iit];
            if (!it->Data || it->Options != CKFileObject::CK_FO_DEFAULT)
                continue;

            CKObject *obj = it->ObjPtr;
            if (obj) {
                obj->Load(it->Data, this);
                ++count;

                if (m_Context->m_UICallBackFct) {
                    CKUICallbackStruct cbs;
                    cbs.Reason = CKUIM_LOADSAVEPROGRESS;
                    cbs.Param1 = count;
                    cbs.Param1 = m_FileObjects.Size();
                    m_Context->m_UICallBackFct(cbs, m_Context->m_InterfaceModeData);
                }

                if (list && inclusion.IsSet(it->ObjectCid)) {
                    list->InsertRear(obj);
                }
            }

            if (it->Data) {
                VxDelete<CKStateChunk>(it->Data);
                it->Data = nullptr;
            }
        }
    }

    for (XArray<int>::Iterator iit = m_IndexByClassId[CKCID_BEHAVIOR].Begin(); iit != m_IndexByClassId[CKCID_BEHAVIOR].End(); ++iit) {
        CKFileObject *it = &m_FileObjects[*iit];
        if (!it->Data || it->Options != CKFileObject::CK_FO_DEFAULT)
            continue;

        auto *beh = (CKBehavior *) it->ObjPtr;
        if (beh) {
            beh->Load(it->Data, this);
            ++count;

            if (m_Context->m_UICallBackFct) {
                CKUICallbackStruct cbs;
                cbs.Reason = CKUIM_LOADSAVEPROGRESS;
                cbs.Param1 = count;
                cbs.Param1 = m_FileObjects.Size();
                m_Context->m_UICallBackFct(cbs, m_Context->m_InterfaceModeData);
            }

            if (list && inclusion.IsSet(it->ObjectCid)) {
                list->InsertRear(beh);
            }
        }

        if ((beh->GetFlags() & CKBEHAVIOR_TOPMOST) || beh->GetType() == CKBEHAVIORTYPE_SCRIPT) {
            if (list) {
                list->InsertRear(beh);
            }
        }

        if (it->Data) {
            VxDelete<CKStateChunk>(it->Data);
            it->Data = nullptr;
        }
    }

    if ((m_Flags & 0x100) == 0) {
        for (XArray<int>::Iterator iit = m_IndexByClassId[CKCID_INTERFACEOBJECTMANAGER].Begin();
             iit != m_IndexByClassId[CKCID_INTERFACEOBJECTMANAGER].End(); ++iit) {
            CKFileObject *it = &m_FileObjects[*iit];
            auto *obj = (CKInterfaceObjectManager *) it->ObjPtr;
            if (obj && list) {
                list->InsertRear(obj);
            }
        }

        for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
            if (it->ObjPtr && it->Data && it->Options == CKFileObject::CK_FO_DEFAULT && CKIsChildClassOf(it->ObjectCid, CKCID_BEOBJECT)) {
                auto *beo = (CKBeObject *)it->ObjPtr;
                CP_CALL_METHOD_PTR(beo, g_CKBeObjectApplyOwnerFunc);
            }
        }

        for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
            if (it->ObjPtr && it->Options == CKFileObject::CK_FO_DEFAULT) {
                CKObject *obj = it->ObjPtr;
                obj->PostLoad();
            }
        }

        for (XArray<CKFileObject>::Iterator it = m_FileObjects.Begin(); it != m_FileObjects.End(); ++it) {
            CKObject *obj = m_Context->GetObject(it->CreatedObject);
            if (obj) {
                if (CKIsChildClassOf(it->ObjectCid, CKCID_BEHAVIOR)) {
                    auto *beh = (CKBehavior *) it->ObjPtr;
                    CP_CALL_METHOD_PTR(beh, g_CKBehaviorApplyPatchLoadFunc);
                    beh->CallCallbackFunction(CKM_BEHAVIORLOAD);
                }
                if (it->Data && CKIsChildClassOf(it->ObjectCid, CKCID_BEOBJECT)) {
                    auto *beo = (CKBeObject *) obj;
                    beo->ApplyPatchForOlderVersion(m_FileObjects.Size(), it);
                }
            }
        }
    }

    objectManager->EndLoadSession();
#elif CKVERSION == 0x05082002
    CP_CALL_METHOD_ORIG(FinishLoading, list, flags);
#endif
}

CKObject * CP_FILE_METHOD_NAME(ResolveReference)(CKFileObject *Data) {
#if CKVERSION == 0x13022002
    if (!CKIsChildClassOf(Data->ObjectCid, CKCID_PARAMETER))
        return nullptr;

    if (!Data->Data)
        return nullptr;

    CKStateChunk *chunk = Data->Data;
    chunk->StartRead();
    if (!chunk->SeekIdentifier(64))
        return nullptr;

    CKParameterManager *pm = m_Context->GetParameterManager();
    CKGUID paramGuid = chunk->ReadGuid();
    CKParameterType paramType = pm->ParameterGuidToType(paramGuid);
    const int paramCount = m_Context->GetObjectsCountByClassID(Data->ObjectCid);
    if (paramCount <= 0)
        return nullptr;

    CK_ID *paramIds = m_Context->GetObjectsListByClassID(Data->ObjectCid);
    CKParameter *target = nullptr;
    for (int i = 0; i < paramCount; ++i) {
        CKParameter *param = (CKParameter *) m_Context->GetObject(paramIds[i]);
        if (param && param->GetName() && param->GetType() == paramType && strcmp(param->GetName(), Data->Name) == 0) {
            target = param;
            break;
        }
    }
    return target;
#elif CKVERSION == 0x05082002
    return CP_CALL_METHOD_ORIG(ResolveReference, Data);
#endif
}

bool CP_HOOK_CLASS_NAME(CKFile)::InitHooks(HookApi *api) {
#define CP_ADD_METHOD_HOOK(Name, Module, Symbol) \
        if ((api->CreateHookApi(Module, Symbol, \
                            *reinterpret_cast<void **>(&CP_FUNC_PTR_NAME(Name)), \
                            reinterpret_cast<void **>(&CP_FUNC_ORIG_PTR_NAME(Name)), \
                            reinterpret_cast<void **>(&CP_FUNC_TARGET_PTR_NAME(Name))) != HAEC_OK || \
            api->EnableHook(*reinterpret_cast<void **>(&CP_FUNC_TARGET_PTR_NAME(Name))) != HAEC_OK)) \
                return false

    CP_ADD_METHOD_HOOK(OpenFile, "CK2.dll", "?OpenFile@CKFile@@QAEJPADW4CK_LOAD_FLAGS@@@Z");
    CP_ADD_METHOD_HOOK(OpenMemory, "CK2.dll", "?OpenMemory@CKFile@@QAEJPAXHW4CK_LOAD_FLAGS@@@Z");

    CP_ADD_METHOD_HOOK(LoadFileData, "CK2.dll", "?LoadFileData@CKFile@@QAEJPAVCKObjectArray@@@Z");

    CP_ADD_METHOD_HOOK(LoadFile, "CK2.dll", "?Load@CKFile@@QAEJPADPAVCKObjectArray@@W4CK_LOAD_FLAGS@@@Z");
    CP_ADD_METHOD_HOOK(LoadMemory, "CK2.dll", "?Load@CKFile@@QAEJPAXHPAVCKObjectArray@@W4CK_LOAD_FLAGS@@@Z");

    CP_ADD_METHOD_HOOK(StartSave, "CK2.dll", "?StartSave@CKFile@@QAEJPADK@Z");
    CP_ADD_METHOD_HOOK(SaveObject, "CK2.dll", "?SaveObject@CKFile@@QAEXPAVCKObject@@K@Z");
    CP_ADD_METHOD_HOOK(SaveObjects, "CK2.dll", "?SaveObjects@CKFile@@QAEXPAVCKObjectArray@@K@Z");
    CP_ADD_METHOD_HOOK(SaveObjects2, "CK2.dll", "?SaveObjects@CKFile@@QAEXPAKHK@Z");
    CP_ADD_METHOD_HOOK(SaveObjects3, "CK2.dll","?SaveObjects@CKFile@@QAEXPAPAVCKObject@@HK@Z");
    CP_ADD_METHOD_HOOK(SaveObjectAsReference, "CK2.dll", "?SaveObjectAsReference@CKFile@@QAEXPAVCKObject@@@Z");
    CP_ADD_METHOD_HOOK(EndSave, "CK2.dll", "?EndSave@CKFile@@QAEJXZ");

    CP_ADD_METHOD_HOOK(IncludeFile, "CK2.dll", "?IncludeFile@CKFile@@QAEHPADH@Z");

    CP_ADD_METHOD_HOOK(IsObjectToBeSaved, "CK2.dll", "?IsObjectToBeSaved@CKFile@@QAEHK@Z");

    CP_ADD_METHOD_HOOK(LoadAndSave, "CK2.dll", "?LoadAndSave@CKFile@@QAEXPAD0@Z");
    CP_ADD_METHOD_HOOK(RemapManagerInt, "CK2.dll", "?RemapManagerInt@CKFile@@QAEXUCKGUID@@PAHH@Z");

    CP_ADD_METHOD_HOOK(ClearData, "CK2.dll", "?ClearData@CKFile@@IAEXXZ");

    CP_ADD_METHOD_HOOK(ReadFileHeaders, "CK2.dll", "?ReadFileHeaders@CKFile@@IAEJPAPAVCKBufferParser@@@Z");
    CP_ADD_METHOD_HOOK(ReadFileData, "CK2.dll", "?ReadFileData@CKFile@@IAEJPAPAVCKBufferParser@@@Z");
    CP_ADD_METHOD_HOOK(FinishLoading, "CK2.dll", "?FinishLoading@CKFile@@IAEXPAVCKObjectArray@@K@Z");

    CP_ADD_METHOD_HOOK(ResolveReference, "CK2.dll", "?ResolveReference@CKFile@@IAEPAVCKObject@@PAUCKFileObject@@@Z");

    void *base = utils::GetModuleBaseAddress("CK2.dll");
    assert(base != nullptr);

#if CKVERSION == 0x13022002
    g_CKBehaviorApplyPatchLoadFunc = utils::ForceReinterpretCast<CKBehaviorApplyPatchLoadFunc>(base, 0x6337);
    g_CKBeObjectApplyOwnerFunc = utils::ForceReinterpretCast<CKBeObjectApplyOwnerFunc>(base, 0x1BBA6);

    g_ClearStringArrayFunc = utils::ForceReinterpretCast<ClearStringArrayFunc>(base, 0xDFD7);
    g_ResizeStringArrayFunc = utils::ForceReinterpretCast<ResizeStringArrayFunc>(base, 0x20A0F);

    g_ResizePluginsDepsArrayFunc = utils::ForceReinterpretCast<ResizePluginsDepsArrayFunc>(base, 0x2098F);

    g_ClearIndexByClassIdArrayFunc = utils::ForceReinterpretCast<ClearIndexByClassIdArrayFunc>(base, 0x209C3);
    g_ResizeIndexByClassIdArrayFunc = utils::ForceReinterpretCast<ResizeIndexByClassIdArrayFunc>(base, 0x209E0);

    g_MaxClassID = utils::ForceReinterpretCast<decltype(g_MaxClassID)>(base, 0x5AB0C);
    g_CurrentFileWriteMode = utils::ForceReinterpretCast<decltype(g_CurrentFileWriteMode)>(base, 0x5F6B8);
    g_CurrentFileVersion = utils::ForceReinterpretCast<decltype(g_CurrentFileVersion)>(base, 0x5F6BC);
    g_WarningForOlderVersion = utils::ForceReinterpretCast<decltype(g_WarningForOlderVersion)>(base, 0x5F6C0);
#endif

    return true;

#undef CP_ADD_METHOD_HOOK
}

void CP_HOOK_CLASS_NAME(CKFile)::ShutdownHooks(HookApi *api) {
#define CP_REMOVE_METHOD_HOOK(Name) \
    api->DisableHook(*reinterpret_cast<void **>(&CP_FUNC_TARGET_PTR_NAME(Name))); \
    api->RemoveHook(*reinterpret_cast<void **>(&CP_FUNC_TARGET_PTR_NAME(Name)))

    CP_REMOVE_METHOD_HOOK(OpenFile);
    CP_REMOVE_METHOD_HOOK(OpenMemory);

    CP_REMOVE_METHOD_HOOK(LoadFileData);

    CP_REMOVE_METHOD_HOOK(LoadFile);
    CP_REMOVE_METHOD_HOOK(LoadMemory);

    CP_REMOVE_METHOD_HOOK(StartSave);
    CP_REMOVE_METHOD_HOOK(SaveObject);
    CP_REMOVE_METHOD_HOOK(SaveObjects);
    CP_REMOVE_METHOD_HOOK(SaveObjects2);
    CP_REMOVE_METHOD_HOOK(SaveObjects3);
    CP_REMOVE_METHOD_HOOK(SaveObjectAsReference);
    CP_REMOVE_METHOD_HOOK(EndSave);

    CP_REMOVE_METHOD_HOOK(IncludeFile);

    CP_REMOVE_METHOD_HOOK(IsObjectToBeSaved);

    CP_REMOVE_METHOD_HOOK(LoadAndSave);
    CP_REMOVE_METHOD_HOOK(RemapManagerInt);

    CP_REMOVE_METHOD_HOOK(ClearData);

    CP_REMOVE_METHOD_HOOK(ReadFileHeaders);
    CP_REMOVE_METHOD_HOOK(ReadFileData);
    CP_REMOVE_METHOD_HOOK(FinishLoading);

    CP_REMOVE_METHOD_HOOK(ResolveReference);

#undef CP_REMOVE_METHOD_HOOK
}
