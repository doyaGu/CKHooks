#include "HookLoader.h"

#include <cassert>
#include <cstdio>
#include <memory>

#include <yyjson.h>

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

    if (!filename || strlen(filename) == 0) {
        utils::OutputDebugA("%s: No path is given.", __FUNCTION__);
        return false;
    }

    utils::OutputDebugA("%s: Current Directory: %s", __FUNCTION__, utils::GetCurrentDir().CStr());

    char *buf;
    size_t len;
    {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            utils::OutputDebugA("%s: Failed to open %s", __FUNCTION__, filename);
            return false;
        }

        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        rewind(fp);

        buf = new char[len + 1];
        if (fread(buf, sizeof(char), len, fp) != len) {
            utils::OutputDebugA("%s: Failed to read content from %s", __FUNCTION__, filename);
            fclose(fp);
            delete[] buf;
            return false;
        }
        buf[len] = '\0';

        fclose(fp);
    }

    if (!LoadJson(buf, len)) {
        delete[] buf;
        return false;
    }
    delete[] buf;

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

        if (!hook->Load()) {
            utils::OutputDebugA("%s: Hook \"%s\" is not loadable, it will be unregistered.", __FUNCTION__, hook->GetInfo().name.CStr());
            m_HookMap.Remove(hook->GetInfo().name);
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

        hook->Unload();

        m_HookMap.Remove(hook->GetInfo().name);
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
            m_HookMap.Remove(hook->GetInfo().name);
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

bool HookLoader::LoadJson(char *data, size_t len) {
    if (!data || len == 0)
        return false;

    yyjson_read_flag flg = YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_INF_AND_NAN;
    yyjson_read_err err;
    auto doc = std::shared_ptr<yyjson_doc>(yyjson_read_opts(data, len, flg, nullptr, &err),
                                           [](yyjson_doc *doc) { yyjson_doc_free(doc); });
    if (!doc) {
        utils::OutputDebugA("%s: Parse Error: %s", __FUNCTION__, err.msg);
        return false;
    }

    yyjson_val *root = yyjson_doc_get_root(doc.get());
    if (!root) {
        utils::OutputDebugA("Unreachable error: yyjson_doc_get_root() return null.");
        return false;
    }

    yyjson_val *paths = yyjson_obj_get(root, "paths");
    if (paths) {
        if (yyjson_is_arr(paths)) {
            yyjson_val *val;
            yyjson_arr_iter iter = yyjson_arr_iter_with(paths);
            while ((val = yyjson_arr_iter_next(&iter))) {
                if (yyjson_is_str(val)) {
                    XString path = yyjson_get_str(val);

                    if (!utils::IsDirectoryExist(path))
                        continue;

                    path = utils::GetAbsolutePath(path);
                    if (!m_Paths.IsHere(path))
                        m_Paths.PushBack(path);
                }
            }
        } else {
            utils::OutputDebugA("%s: \"paths\" is not an array.", __FUNCTION__);
        }
    }

    yyjson_val *hooks = yyjson_obj_get(root, "hooks");
    if (!hooks) {
        utils::OutputDebugA("%s: \"hooks\" is not found.", __FUNCTION__);
        return false;
    }

    if (!yyjson_is_arr(hooks)) {
        utils::OutputDebugA("%s: Unknown format, \"hooks\" should be an array.", __FUNCTION__);
        return false;
    }

    if (yyjson_arr_size(hooks) == 0) {
        utils::OutputDebugA("%s: No hook found.", __FUNCTION__);
        return false;
    }

    yyjson_val *hook;
    yyjson_arr_iter iter = yyjson_arr_iter_with(hooks);
    while ((hook = yyjson_arr_iter_next(&iter))) {
        if (!yyjson_is_obj(hook)) {
            utils::OutputDebugA("%s: Unknown format, hook entry should be an object.", __FUNCTION__);
            continue;
        }

        HookModuleInfo info;

        {
            yyjson_val *hookName = yyjson_obj_get(hook, "name");
            if (!hookName) {
                utils::OutputDebugA("%s: Hook Name is required.", __FUNCTION__);
                continue;
            }

            if (!yyjson_is_str(hookName)) {
                utils::OutputDebugA("%s: Unknown format, Hook Name should be a string.", __FUNCTION__);
                continue;
            }

            const char *str = yyjson_get_str(hookName);
            if (str[0] == '\0') {
                utils::OutputDebugA("%s: Hook Name can not be empty.", __FUNCTION__);
                continue;
            }
            info.name = str;
        }

        {
            yyjson_val *hookFilename = yyjson_obj_get(hook, "filename");
            if (!hookFilename) {
                utils::OutputDebugA("%s: Hook Filename is required.", __FUNCTION__);
                continue;
            }

            if (!yyjson_is_str(hookFilename)) {
                utils::OutputDebugA("%s: Unknown format, Hook Filename should be a string.", __FUNCTION__);
                continue;
            }

            const char *str = yyjson_get_str(hookFilename);
            if (str[0] == '\0') {
                utils::OutputDebugA("%s: Hook Filename can not be empty.", __FUNCTION__);
                continue;
            }
            info.filename = str;
        }

        if (info.filename.RFind('.') == XString::NOTFOUND || !utils::StringIEndsWith(info.filename, ".dll"))
            info.filename << ".dll";

        if (!utils::IsFileExist(info.filename)) {
            bool resolved = false;
            for (auto it = m_Paths.Begin(); it != m_Paths.End(); ++it) {
                XString &path = *it;
                XString filename = utils::JoinPaths(path, info.filename);
                if (utils::IsFileExist(filename)) {
                    info.filename = filename;
                    resolved = true;
                }
            }

            if (!resolved) {
                utils::OutputDebugA("%s: \"%s\" is not found, non-existing hook will be ignored.", __FUNCTION__, info.filename.CStr());
                continue;
            }
        }

        {
            yyjson_val *hookHandlerName = yyjson_obj_get(hook, "handler");
            if (!hookHandlerName) {
                utils::OutputDebugA("%s: Hook Handler Name is required.", __FUNCTION__);
                continue;
            }

            if (!yyjson_is_str(hookHandlerName)) {
                utils::OutputDebugA("%s: Unknown format, Hook Handler Name should be a string.", __FUNCTION__);
                continue;
            }

            const char *str = yyjson_get_str(hookHandlerName);
            if (str[0] == '\0') {
                utils::OutputDebugA("%s: Hook Handler Name can not be empty.", __FUNCTION__);
                continue;
            }
            info.handlerName = str;
        }

        {
            yyjson_val *hookCode = yyjson_obj_get(hook, "code");
            if (!hookCode) {
                utils::OutputDebugA("%s: Hook Code is required.", __FUNCTION__);
                continue;
            }
            if (!yyjson_is_num(hookCode)) {
                utils::OutputDebugA("%s: Unknown format, Hook Code should be a number.", __FUNCTION__);
                continue;
            }
            info.code = static_cast<uint32_t>(yyjson_get_uint(hookCode));
        }

        {
            yyjson_val *hookVersion = yyjson_obj_get(hook, "version");
            if (!hookVersion) {
                utils::OutputDebugA("%s: Hook Version is required.", __FUNCTION__);
                continue;
            }
            if (!yyjson_is_num(hookVersion)) {
                utils::OutputDebugA("%s: Unknown format, Hook Version should be a number.", __FUNCTION__);
                continue;
            }
            info.version = static_cast<uint32_t>(yyjson_get_uint(hookVersion));
        }

        CreateHook(info);
    }

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
