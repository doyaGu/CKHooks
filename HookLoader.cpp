#include "HookLoader.h"

#include <cassert>
#include <cstdio>

#include <cJSON.h>

#include "Utils.h"

HookLoader &HookLoader::GetInstance() {
    static HookLoader instance;
    return instance;
}

HookLoader::~HookLoader() {
    if (m_Flag != 0)
        Fini();
}

bool HookLoader::Init(const char *filename) {
    if (IsInitialized())
        return false;

    utils::OutputDebugA("%s: Current Directory: %s", __FUNCTION__, utils::GetCurrentDir().CStr());

    if (!LoadJson(filename))
        return false;

    if (GetHookCount() == 0) {
        utils::OutputDebugA("%s: No Hooks", __FUNCTION__);
        return false;
    }

    m_Flag |= HLF_INITIALIZED;
    return true;
}

void HookLoader::Fini() {
    if (IsLoaded()) {
        UnloadHooks();
    }

    if (IsInitialized()) {
        m_Flag &= ~HLF_INITIALIZED;
    }
}

void HookLoader::LoadHooks() {
    HookModule **it = m_Hooks.Begin();
    while (it != m_Hooks.End()) {
        HookModule *hook = *it;
        assert(hook != nullptr);
        auto &info = hook->GetInfo();

        if (!hook->Load()) {
            utils::OutputDebugA("%s: Hook \"%s\" is not loadable, it will be unregistered.", __FUNCTION__, info.name.CStr());
            m_HookMap.Remove(info.name);
            it = m_Hooks.Remove(it);
            DestroyHook(hook);
        } else {
            ++it;
        }
    }

    m_Flag |= HLF_LOADED;
}

void HookLoader::UnloadHooks() {
    HookModule **it = m_Hooks.RBegin();
    while (it != m_Hooks.REnd()) {
        HookModule *hook = *it;
        assert(hook != nullptr);
        auto &info = hook->GetInfo();

        hook->Unload();

        m_HookMap.Remove(info.name);
        m_Hooks.PopBack();
        DestroyHook(hook);
        --it;
    }

    m_Flag &= ~HLF_LOADED;
}

void HookLoader::IterateHooks(HookModuleCallback callback, void *arg) {
    HookModule **it = m_Hooks.Begin();
    while (it != m_Hooks.End()) {
        HookModule *hook = *it;
        assert(hook != nullptr);

        if (callback(hook, arg) == HMR_TERMINATE) {
            auto &info = hook->GetInfo();
            m_HookMap.Remove(info.name);
            it = m_Hooks.Remove(it);
            DestroyHook(hook);
        } else {
            ++it;
        }
    }
}

int HookLoader::GetHookCount() {
    return m_Hooks.Size();
}

HookModule *HookLoader::GetHookByIndex(int index) {
    return m_Hooks[index];
}

HookModule *HookLoader::GetHookByName(const char *name) {
    if (!name || strlen(name) == 0) return nullptr;
    return m_HookMap[name];
}

void *HookLoader::SetData(void *data, int id) {
    size_t old = 0;
    m_Data.LookUp(id, old);
    m_Data[id] = reinterpret_cast<size_t>(data);
    return reinterpret_cast<void *>(old);
}

void *HookLoader::GetData(int id) const {
    size_t data = 0;
    if (!m_Data.LookUp(id, data))
        return nullptr;
    return reinterpret_cast<void *>(data);
}

HookLoader::HookLoader() = default;

bool HookLoader::LoadJson(const char *path) {
    if (!path || strlen(path) == 0) {
        utils::OutputDebugA("%s: No path is given.", __FUNCTION__, path);
        return false;
    }

    char *buf = nullptr;
    {
        FILE *fp = fopen(path, "rb");
        if (!fp) {
            utils::OutputDebugA("%s: Failed to open %s", __FUNCTION__, path);
            return false;
        }

        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        rewind(fp);

        buf = new char[size + 1];
        if (fread(buf, sizeof(char), size, fp) != size) {
            utils::OutputDebugA("%s: Failed to read content from %s", __FUNCTION__, path);
            fclose(fp);
            delete[] buf;
            return false;
        }
        buf[size] = '\0';

        fclose(fp);
    }

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        utils::OutputDebugA("%s: Parse Error, Invalid json format.", __FUNCTION__);
        delete[] buf;
        return false;
    }

    cJSON *hooks = cJSON_GetObjectItem(root, "hooks");
    if (!hooks) {
        utils::OutputDebugA("%s: \"hooks\" is not found.", __FUNCTION__);
        cJSON_Delete(root);
        delete[] buf;
        return false;
    }

    if (!cJSON_IsArray(hooks)) {
        utils::OutputDebugA("%s: Unknown format, \"hooks\" should be an array.", __FUNCTION__);
        cJSON_Delete(root);
        delete[] buf;
        return false;
    }

    int hookCount = cJSON_GetArraySize(hooks);
    if (hookCount == 0) {
        utils::OutputDebugA("%s: No hook found.", __FUNCTION__);
        cJSON_Delete(root);
        delete[] buf;
        return false;
    }

    for (int i = 0; i < hookCount; i++) {
        cJSON *hook = cJSON_GetArrayItem(hooks, i);
        if (!hook) continue;

        if (!cJSON_IsObject(hook)) {
            utils::OutputDebugA("%s: Unknown format, \"%s\" should be an object.", __FUNCTION__, hook->string);
            continue;
        }

        HookModuleInfo info;

        {
            cJSON *hookName = cJSON_GetObjectItem(hook, "name");
            if (!hookName) {
                utils::OutputDebugA("%s: Hook Name is required.", __FUNCTION__);
                continue;
            }
            if (!cJSON_IsString(hookName)) {
                utils::OutputDebugA("%s: Unknown format, Hook Name should be a string.", __FUNCTION__);
                continue;
            }
            if (strlen(hookName->valuestring) == 0) {
                utils::OutputDebugA("%s: Hook Name can not be empty.", __FUNCTION__);
                continue;
            }
            info.name = hookName->valuestring;
        }

        {
            cJSON *hookFilename = cJSON_GetObjectItem(hook, "filename");
            if (!hookFilename) {
                utils::OutputDebugA("%s: Hook Filename is required.", __FUNCTION__);
                continue;
            }
            if (!cJSON_IsString(hookFilename)) {
                utils::OutputDebugA("%s: Unknown format, Hook Filename should be a string.", __FUNCTION__);
                continue;
            }
            if (strlen(hookFilename->valuestring) == 0) {
                utils::OutputDebugA("%s: Hook Filename can not be empty.", __FUNCTION__);
                continue;
            }
            info.filename = hookFilename->valuestring;
        }

        if (info.filename.RFind('.') == XString::NOTFOUND || !info.filename.IEndsWith(".dll"))
            info.filename << ".dll";

        if (!utils::IsFileExist(info.filename)) {
            utils::OutputDebugA("%s: \"%s\" is not found, non-existing hook will be ignored.", __FUNCTION__, info.filename.CStr());
            continue;
        }

        {
            cJSON *hookHandlerName = cJSON_GetObjectItem(hook, "handler");
            if (!hookHandlerName) {
                utils::OutputDebugA("%s: Hook Handler Name is required.", __FUNCTION__);
                continue;
            }
            if (!cJSON_IsString(hookHandlerName)) {
                utils::OutputDebugA("%s: Unknown format, Hook Handler Name should be a string.", __FUNCTION__);
                continue;
            }
            if (strlen(hookHandlerName->valuestring) == 0) {
                utils::OutputDebugA("%s: Hook Handler Name can not be empty.", __FUNCTION__);
                continue;
            }
            info.handlerName = hookHandlerName->valuestring;
        }

        {
            cJSON *hookCode = cJSON_GetObjectItem(hook, "code");
            if (!hookCode) {
                utils::OutputDebugA("%s: Hook Code is required.", __FUNCTION__);
                continue;
            }
            if (!cJSON_IsNumber(hookCode)) {
                utils::OutputDebugA("%s: Unknown format, Hook Code should be a number.", __FUNCTION__);
                continue;
            }
            info.code = hookCode->valueint;
        }

        {
            cJSON *hookVersion = cJSON_GetObjectItem(hook, "version");
            if (!hookVersion) {
                utils::OutputDebugA("%s: Hook Version is required.", __FUNCTION__);
                continue;
            }
            if (!cJSON_IsNumber(hookVersion)) {
                utils::OutputDebugA("%s: Unknown format, Hook Version should be a number.", __FUNCTION__);
                continue;
            }
            info.version = hookVersion->valueint;
        }

        CreateHook(info);
    }

    cJSON_Delete(root);
    delete[] buf;
    return true;
}

HookModule *HookLoader::CreateHook(const HookModuleInfo &info) {
    HookModule *prev = nullptr;
    m_HookMap.LookUp(info.name, prev);
    if (prev) {
        utils::OutputDebugA("%s: Hook \"%s\" already exists, the old one will be overwritten.", __FUNCTION__, info.name.CStr());
        prev->Fini();
        prev->Init(info);
        return prev;
    }

    auto *hook = new HookModule();
    hook->Init(info);
    hook->AddCallback(HMCI_ONPREFINI, OnPreHookFini, this);

    m_Hooks.PushBack(hook);
    m_HookMap.Insert(info.name, hook);

    return hook;
}

void HookLoader::DestroyHook(HookModule *hook) {
    delete hook;
}

int HookLoader::OnPreHookFini(HookModule *hook, void *arg) {
    assert(arg != nullptr);
    auto *loader = reinterpret_cast<HookLoader *>(arg);
    auto &info = hook->GetInfo();
    if (loader->m_HookMap.IsHere(info.name)) {
        loader->m_Hooks.Remove(hook);
        loader->m_HookMap.Remove(info.name);
    }
    return 0;
}
