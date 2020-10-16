#include "ModuleManager.hpp"

#if WIN32
#include <Shlwapi.h>
#include <Windows.h>
#include <dbghelp.h>
#include <sysinfoapi.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")
#elif __ANDROID__
#include <dlfcn.h>
#endif

namespace aoce {

ModuleManager* ModuleManager::instance = nullptr;
ModuleManager& ModuleManager::Get() {
    if (instance == nullptr) {
        instance = new ModuleManager();
    }
    return *instance;
}

ModuleManager::ModuleManager(/* args */) {}

ModuleManager::~ModuleManager() {}

void ModuleManager::registerModule(const std::string& name,
                                   loadModuleHandle handle) {
    if (modules.find(name) != modules.end()) {
        return;
    }
    ModuleInfo* moduleInfo = new ModuleInfo();
    modules[name] = moduleInfo;
#if __ANDROID__
    moduleInfo->name = "lib" + name + ".so";
#endif
    moduleInfo->name = name;
    moduleInfo->onLoadEvent = handle;
}

void ModuleManager::loadModule(const std::string& name) {
    if (modules.find(name) == modules.end()) {
        return;
    }
    ModuleInfo* moduleInfo = modules[name];
    if (moduleInfo->load) {
        return;
    }
    if (moduleInfo->onLoadEvent) {
        moduleInfo->module = moduleInfo->onLoadEvent();
    } else {
        loadModuleAction loadAction = nullptr;
#if WIN32
        char temp[512] = {0};
        GetDllDirectoryA(512, temp);
        char sz[512] = {0};
        HMODULE ihdll = GetModuleHandleA("aoce.dll");
        ::GetModuleFileNameA(ihdll, sz, 512);
        ::PathRemoveFileSpecA(sz);
        SetDllDirectoryA(sz);
        moduleInfo->handle = LoadLibraryExA(moduleInfo->name.c_str(), nullptr,
                                            LOAD_WITH_ALTERED_SEARCH_PATH);
        if (moduleInfo->handle) {
            loadAction = (loadModuleAction)GetProcAddress(
                (HMODULE)moduleInfo->handle, "NewModule");
        }
        SetDllDirectoryA(temp);
#elif __ANDROID__
        moduleInfo->handle =
            dlopen(moduleInfo->name.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (moduleInfo->handle) {
            loadAction =
                (LoadModuleAction)dlsym(moduleInfo->handle, "NewModule")
        }
#endif
        if (loadAction) {
            moduleInfo->module = loadAction();
        }
    }
    moduleInfo->load = moduleInfo->module != nullptr;
    if (!moduleInfo->load) {
        logMessage(LogLevel::warn, name + ": load failed.");
    }
    // 调用注册
    if (moduleInfo->module) {
        moduleInfo->module->loadModule();
    }
}

void ModuleManager::regAndLoad(const std::string& name) {
    registerModule(name);
    loadModule(name);
}

void ModuleManager::unloadModule(const std::string& name) {
    if (modules.find(name) == modules.end()) {
        return;
    }
    ModuleInfo* moduleInfo = modules[name];
    if (!moduleInfo->load) {
        return;
    }
    if (moduleInfo->module) {
        moduleInfo->module->unloadModule();
        delete moduleInfo->module;
        moduleInfo->module = nullptr;
    }
    if (moduleInfo->handle) {
#if WIN32
        FreeLibrary((HMODULE)moduleInfo->handle);
#elif __ANDROID__
        dlclose(moduleInfo->handle);
#endif
        moduleInfo->handle = nullptr;
    }
    moduleInfo->load = false;
}

}  // namespace aoce

#if WIN32

LONG WINAPI unhandledFilter(struct _EXCEPTION_POINTERS* lpExceptionInfo) {
    LONG ret = EXCEPTION_EXECUTE_HANDLER;
    TCHAR szFileName[64];
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    wsprintf(szFileName, TEXT("OEIP_%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp"),
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
             GetCurrentProcessId(), GetCurrentThreadId());

    HANDLE hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = ::GetCurrentThreadId();
        ExInfo.ExceptionPointers = lpExceptionInfo;
        ExInfo.ClientPointers = false;

        // write the dump
        BOOL bOK =
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                              MiniDumpNormal, &ExInfo, NULL, NULL);
        ::CloseHandle(hFile);
    }
    return ret;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        SetUnhandledExceptionFilter(
            (LPTOP_LEVEL_EXCEPTION_FILTER)unhandledFilter);
        // aoce::ModuleManager::Get().registerModule("aoce-mf");
    } else if (dwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
#elif __ANDROID__

#endif