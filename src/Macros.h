#ifndef HOOKS_MACROS_H
#define HOOKS_MACROS_H

#define CP_HOOK_CLASS_NAME(Class) \
    Class##Hook

#define CP_HOOK_CLASS(Class) \
    class CP_HOOK_CLASS_NAME(Class) : public Class

#define CP_FUNC_NAME(Name) \
    Name##Hook

#define CP_FUNC_TYPE_NAME(Name) \
    Name##Func

#define CP_FUNC_PTR_NAME(Name) \
    s_##Name##Func

#define CP_FUNC_ORIG_PTR_NAME(Name) \
    s_##Name##FuncOrig

#define CP_FUNC_TARGET_PTR_NAME(Name) \
    s_##Name##FuncTarget

#define CP_DECLARE_METHOD_HOOK(Class, Ret, Name, Args) \
    Ret CP_FUNC_NAME(Name) Args; \
    typedef Ret(Class::*CP_FUNC_TYPE_NAME(Name)) Args; \
    static CP_FUNC_TYPE_NAME(Name) CP_FUNC_PTR_NAME(Name); \
    static CP_FUNC_TYPE_NAME(Name) CP_FUNC_ORIG_PTR_NAME(Name); \
    static CP_FUNC_TYPE_NAME(Name) CP_FUNC_TARGET_PTR_NAME(Name);

#define CP_DEFINE_METHOD_HOOK_PTRS(Class, Name) \
    CP_HOOK_CLASS_NAME(Class)::CP_FUNC_TYPE_NAME(Name) CP_HOOK_CLASS_NAME(Class)::CP_FUNC_PTR_NAME(Name) = \
        reinterpret_cast<CP_HOOK_CLASS_NAME(Class)::CP_FUNC_TYPE_NAME(Name)>(&CP_HOOK_CLASS_NAME(Class)::CP_FUNC_NAME(Name)); \
    CP_HOOK_CLASS_NAME(Class)::CP_FUNC_TYPE_NAME(Name) CP_HOOK_CLASS_NAME(Class)::CP_FUNC_ORIG_PTR_NAME(Name) = nullptr; \
    CP_HOOK_CLASS_NAME(Class)::CP_FUNC_TYPE_NAME(Name) CP_HOOK_CLASS_NAME(Class)::CP_FUNC_TARGET_PTR_NAME(Name) = nullptr;

#define CP_CALL_METHOD(Ref, Func, ...) \
    ((&Ref)->*Func)(__VA_ARGS__)

#define CP_CALL_METHOD_PTR(Ptr, Func, ...) \
    (Ptr->*Func)(__VA_ARGS__)

#define CP_CALL_METHOD_ORIG(Name, ...) \
    (this->*CP_FUNC_ORIG_PTR_NAME(Name))(__VA_ARGS__)

#endif // HOOKS_MACROS_H
