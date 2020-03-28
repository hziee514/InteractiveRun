// main.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"

#include <wtsapi32.h>
#include <userenv.h>

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")

class scoped_handle
{
private:
    HANDLE handle;
public:
    scoped_handle()
    {
        this->handle = NULL;
    }

    ~scoped_handle()
    {
        if (this->handle != NULL)
            CloseHandle(this->handle);
    }
    HANDLE get()
    {
        return this->handle;
    }
    PHANDLE ref()
    {
        return &this->handle;
    }
};

class scoped_env
{
private:
    LPVOID env;
public:
    scoped_env()
    {
        this->env = NULL;
    }

    ~scoped_env()
    {
        if (this->env != NULL)
            DestroyEnvironmentBlock(this->env);
    }
    LPVOID get()
    {
        return this->env;
    }
    LPVOID* ref()
    {
        return &this->env;
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    WCHAR path[MAX_PATH + 4] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);
    lstrcat(path, TEXT(".ini"));

    int nShowWindow = GetPrivateProfileInt(TEXT("Window"), TEXT("show"), SW_SHOWNORMAL, path);
    int dwFlags = GetPrivateProfileInt(TEXT("Window"), TEXT("flags"), STARTF_USESHOWWINDOW, path);

    scoped_handle token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, token.ref()))
    {
        return 1;
    }

    // 提升进程权限(SE_TCB_NAME)
    TOKEN_PRIVILEGES tkp;
    LookupPrivilegeValue(NULL, SE_TCB_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(token.get(), FALSE, &tkp, sizeof tkp, NULL, NULL))
    {
        return 2;
    }

    // 查询桌面会话访问令牌
    scoped_handle primaryToken;
    DWORD consoleSessionId = WTSGetActiveConsoleSessionId();
    // MSDN指出调用该函数必须使用LocalSystem账户，并且拥有SE_TCB_NAME权限
    if (!WTSQueryUserToken(consoleSessionId, primaryToken.ref()))
    {
        return 3;
    }

    // 获取桌面用户环境变量
    scoped_env env;
    if (!CreateEnvironmentBlock(env.ref(), primaryToken.get(), FALSE))
    {
        return 4;
    }

    WCHAR desk[32] = { 0 };
    lstrcpy(desk, TEXT("winsta0\\default"));

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = desk;
    si.wShowWindow = nShowWindow;
    si.dwFlags = dwFlags;
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // 在桌面环境中运行程序
    if (!CreateProcessAsUser(primaryToken.get(),
        NULL,
        lpCmdLine,
        NULL,
        NULL,
        FALSE,
        NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
        env.get(),
        NULL,
        &si,
        &pi))
    {
        return 5;
    }

    // 等待桌面进程终止
    WaitForSingleObject(pi.hProcess, INFINITE);

    return 0;
}

