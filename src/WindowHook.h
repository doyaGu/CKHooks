#ifndef HOOKS_WINDOWHOOK_H
#define HOOKS_WINDOWHOOK_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "CallbackList.h"

typedef LRESULT (CALLBACK *LPFNWNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef LRESULT (*WH_WNDPROCCALLBACK)(HWND, UINT, WPARAM, LPARAM, void *);
typedef void (*WH_WNDPROCRETCALLBACK)(HWND, UINT, WPARAM, LPARAM, LRESULT, void *);

class WindowHook {
public:
    static WindowHook &GetInstance();

    ~WindowHook();

    bool Init(HWND hwnd);
    void Shutdown();

    void AddOnWndProcCallBack(WH_WNDPROCCALLBACK func, void *arg);
    void RemoveOnWndProcCallBack(WH_WNDPROCCALLBACK func, void *arg);

    void AddOnWndProcRetCallBack(WH_WNDPROCRETCALLBACK func, void *arg);
    void RemoveOnWndProcRetCallBack(WH_WNDPROCRETCALLBACK func, void *arg);

private:
    WindowHook();

    void HookWndProc();
    void UnhookWndProc();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    CallbackList<LRESULT, HWND, UINT, WPARAM, LPARAM> m_OnWndProcCallBacks;
    CallbackList<void, HWND, UINT, WPARAM, LPARAM, LRESULT> m_OnWndProcRetCallBacks;

    HWND m_WndHandle = nullptr;

    static LPFNWNDPROC s_WndProc;
};


#endif /* HOOKS_WINDOWHOOK_H */
