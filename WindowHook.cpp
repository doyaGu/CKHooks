#include "WindowHook.h"

LPFNWNDPROC WindowHook::s_WndProc = nullptr;

WindowHook &WindowHook::GetInstance() {
    static WindowHook instance;
    return instance;
}

WindowHook::~WindowHook() {
    Shutdown();
}

bool WindowHook::Init(HWND hwnd) {
    if (!hwnd)
        return false;

    HookWndProc();
    return true;
}

void WindowHook::Shutdown() {
    m_OnWndProcCallBacks.Clear();
    m_OnWndProcRetCallBacks.Clear();
    UnhookWndProc();
}

void WindowHook::AddOnWndProcCallBack(WH_WNDPROCCALLBACK func, void *arg) {
    m_OnWndProcCallBacks.Append(func, arg);
}

void WindowHook::RemoveOnWndProcCallBack(WH_WNDPROCCALLBACK func, void *arg) {
    m_OnWndProcCallBacks.Remove(func, arg);
}

void WindowHook::AddOnWndProcRetCallBack(WH_WNDPROCRETCALLBACK func, void *arg) {
    m_OnWndProcRetCallBacks.Append(func, arg);
}

void WindowHook::RemoveOnWndProcRetCallBack(WH_WNDPROCRETCALLBACK func, void *arg) {
    m_OnWndProcRetCallBacks.Remove(func, arg);
}

WindowHook::WindowHook() = default;

void WindowHook::HookWndProc() {
    s_WndProc = reinterpret_cast<LPFNWNDPROC>(GetWindowLongPtr(m_WndHandle, GWLP_WNDPROC));
    SetWindowLongPtr(m_WndHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowHook::WndProc));
}

void WindowHook::UnhookWndProc() {
    if (s_WndProc)
        SetWindowLongPtr(m_WndHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_WndProc));
}

LRESULT WindowHook::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    LRESULT result;

    auto &cbs1 = GetInstance().m_OnWndProcCallBacks;
    for (auto *it = cbs1.Begin(); it != cbs1.End(); ++it) {
        result = (*it)(hWnd, msg, wParam, lParam);
        if (result != 0)
            return result;
    }

    result = s_WndProc(hWnd, msg, wParam, lParam);

    auto &cbs2 = GetInstance().m_OnWndProcRetCallBacks;
    for (auto *it = cbs2.Begin(); it != cbs2.End(); ++it) {
        (*it)(hWnd, msg, wParam, lParam, result);
    }

    return result;
}