#include "HookLoader.h"

#include <cassert>
#include <cstdio>
#include <memory>

#include <yyjson.h>

#include "Logger.h"
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
    assert(filename && filename[0] != '\0');

    if (IsInitialized())
        return false;

    char *buf;
    size_t len;
    {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            LOG_ERROR("Failed to open %s", filename);
            return false;
        }

        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        rewind(fp);

        buf = new char[len + 1];
        if (fread(buf, sizeof(char), len, fp) != len) {
            LOG_ERROR("Failed to read %s", filename);
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
        LOG_INFO("No hook will be loaded.");
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
            LOG_ERROR("Hook \"%s\" is not loadable, it will be unregistered.", hook->GetInfo().name.CStr());
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
        LOG_ERROR("JSON Parse Error: %s", err.msg);
        return false;
    }

    yyjson_val *root = yyjson_doc_get_root(doc.get());
    if (!root) {
        LOG_ERROR("Unreachable error: yyjson_doc_get_root() return NULL.");
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
            LOG_ERROR("Unknown schema, \"paths\" is not an array.");
        }
    }

    yyjson_val *hooks = yyjson_obj_get(root, "hooks");
    if (!hooks) {
        LOG_ERROR("Entry \"hooks\" is not found.");
        return false;
    }

    if (!yyjson_is_arr(hooks)) {
        LOG_ERROR("Unknown schema, \"hooks\" should be an array.");
        return false;
    }

    if (yyjson_arr_size(hooks) == 0) {
        LOG_ERROR("No hook found.");
        return false;
    }

    yyjson_val *hook;
    yyjson_arr_iter iter = yyjson_arr_iter_with(hooks);
    while ((hook = yyjson_arr_iter_next(&iter))) {
        if (!yyjson_is_obj(hook)) {
            LOG_ERROR("Unknown schema, hook entry should be an object.");
            continue;
        }

        HookModuleInfo info;

        {
            yyjson_val *hookName = yyjson_obj_get(hook, "name");
            if (!hookName) {
                LOG_ERROR("Hook Name is required.");
                continue;
            }

            if (!yyjson_is_str(hookName)) {
                LOG_ERROR("Unknown schema, Hook Name should be a string.");
                continue;
            }

            const char *str = yyjson_get_str(hookName);
            if (str[0] == '\0') {
                LOG_ERROR("Hook Name can not be empty.");
                continue;
            }
            info.name = str;
        }

        {
            yyjson_val *hookFilename = yyjson_obj_get(hook, "filename");
            if (!hookFilename) {
                LOG_ERROR("Hook Filename is required.");
                continue;
            }

            if (!yyjson_is_str(hookFilename)) {
                LOG_ERROR("Unknown schema, Hook Filename should be a string.");
                continue;
            }

            const char *str = yyjson_get_str(hookFilename);
            if (str[0] == '\0') {
                LOG_ERROR("Hook Filename can not be empty.");
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
                LOG_ERROR("Unable to find \"%s\", non-existing hook will be ignored.", info.filename.CStr());
                continue;
            }
        }

        {
            yyjson_val *hookHandlerName = yyjson_obj_get(hook, "handler");
            if (!hookHandlerName) {
                LOG_ERROR("Hook Handler Name is required.");
                continue;
            }

            if (!yyjson_is_str(hookHandlerName)) {
                LOG_ERROR("Unknown schema, Hook Handler Name should be a string.");
                continue;
            }

            const char *str = yyjson_get_str(hookHandlerName);
            if (str[0] == '\0') {
                LOG_ERROR("Hook Handler Name can not be empty.");
                continue;
            }
            info.handlerName = str;
        }

        {
            yyjson_val *hookCode = yyjson_obj_get(hook, "code");
            if (!hookCode) {
                LOG_ERROR("Hook Code is required.");
                continue;
            }
            if (!yyjson_is_num(hookCode)) {
                LOG_ERROR("Unknown schema, Hook Code should be a number.");
                continue;
            }
            info.code = static_cast<uint32_t>(yyjson_get_uint(hookCode));
        }

        {
            yyjson_val *hookVersion = yyjson_obj_get(hook, "version");
            if (!hookVersion) {
                LOG_ERROR("Hook Version is required.");
                continue;
            }
            if (!yyjson_is_num(hookVersion)) {
                LOG_ERROR("Unknown schema, Hook Version should be a number.");
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
        LOG_ERROR("Hook \"%s\" already exists, the old one will be overwritten.", info.name.CStr());
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
