#ifndef HOOKS_CALLBACKLIST_H
#define HOOKS_CALLBACKLIST_H

#include <utility>

#include "XArray.h"

template<typename R, typename... Args>
struct Callback {
    typedef R (*FuncType)(Args..., void *);

    FuncType callback;
    void *data;

    explicit Callback(FuncType func, void *arg = nullptr) : callback(func), data(arg) {
        assert(func != nullptr);
    }

    bool operator==(const Callback &rhs) const {
        return callback == rhs.callback && data == rhs.data;
    }

    bool operator!=(const Callback &rhs) const {
        return !(rhs == *this);
    }

    R operator()(Args... args) const {
        return callback(std::forward<Args>(args)..., data);
    }
};

template<typename R, typename... Args>
class CallbackList {
public:
    typedef R (*FuncType)(Args..., void *);

    typedef Callback<R, Args...> CallBackType;

    void operator()(Args... args) const {
        for (auto it = m_List.Begin(); it != m_List.End(); ++it) {
            (*it)(std::forward<Args>(args)...);
        }
    }

    bool Append(FuncType callback, void *data) {
        if (!callback) return false;

        CallBackType cb(callback, data);
        auto *prev = m_List.Find(cb);
        if (prev != m_List.End())
            return false;

        m_List.PushBack(cb);
        return true;
    }

    void Prepend(FuncType callback, void *data) {
        if (!callback) return;

        CallBackType cb(callback, data);
        auto *prev = m_List.Find(cb);
        if (prev == m_List.End())
            m_List.PushFront(cb);
    }

    void Remove(FuncType callback, void *data) {
        if (!callback) return;
        CallBackType cb(callback, data);
        m_List.Remove(cb);
    }

    bool Owns(FuncType callback, void *data) const {
        CallBackType cb(callback, data);
        return m_List.Find(cb) != m_List.End();
    }

    CallBackType *Begin() {
        return m_List.Begin();
    }

    CallBackType *End() {
        return m_List.End();
    }

    void Clear() {
        m_List.Clear();
    }

private:
    XArray<CallBackType> m_List;
};

#endif // HOOKS_CALLBACKLIST_H
