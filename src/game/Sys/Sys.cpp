#include "Sys.hpp"
#include "../Com/Com.hpp"
#include "../Cbuf/Cbuf.hpp"
#include "../CL/CL.hpp"

#include <utils/memory/memory.hpp>

//DONE : 0x004305E0
void Sys_ShowConsole()
{
    HMODULE handle;
    if (!*(HWND*)(0x064A3288))
    {
        handle = GetModuleHandleA(0);
        Sys_CreateConsole(handle);
    }

    ShowWindow(*(HWND*)(0x064A3288) /*s_wcd.hWnd*/, SW_SHOWNORMAL);
    SendMessageA(*(HWND*)(0x0064A328C) /*s_wcd.hwndBuffer*/, EM_LINESCROLL, 0, 0xFFFF);
}

//THUNK : 0x004288A0
void Sys_CreateConsole  (HINSTANCE hInstance)
{
    memory::call<void(HINSTANCE)>(0x004288A0)(hInstance);
}

//DONE : 0x0064DC50
long __stdcall ConsoleWndProc(HWND hWnd, std::uint32_t msg, std::uint32_t wParam, long lParam)
{
    switch (msg)
    {
        case WM_SIZE: // 5
            SetWindowPos(*(HWND*)0x64A328C /*dword_64A328C*/, 0, 5, 70, lParam - 15, HIWORD(lParam) - 100, 0);
            SetWindowPos(*(HWND*)0x64A3298 /*::hWnd*/, 0, 5, HIWORD(lParam) - 100 + 78, lParam - 15, 20, 0);
            *(std::uint16_t*)0x64A389C /*dword_64A389C*/ = (std::uint16_t)lParam;
            *(long*)0x64A38A0 /*dword_64A38A0*/ = HIWORD(lParam);
            return DefWindowProcA(hWnd, msg, wParam, lParam);
        case WM_ACTIVATE: // 6
            if (wParam)
            {
                SetFocus(*(HWND*)0x64A3298 /*::hWnd*/);
            }
            return DefWindowProcA(hWnd, msg, wParam, lParam);
        case WM_CLOSE: // 16
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
}

//DONE : 0x00470190
long __stdcall InputLineWndProc(HWND hWnd, std::uint32_t msg, std::uint32_t wParam, long lParam)
{
    char string[1024];
    char buffer[1024];

    if (msg == 8)
    {
        if ((HWND)wParam == *(HWND*)0x64A3288)
        {
            SetFocus(hWnd);
            return 0;
        }
    }
    else if (msg == 258 && wParam == 13)
    {
        GetWindowTextA(*(HWND*)0x64A3298, string, 1024);
        strncat((char*)0x64A349C, string, 507 - strlen((const char*)0x64A349C));
        *(size_t*)((char*)0x64A349C + strlen((const char*)0x64A349C)) = 10;
        SetWindowTextA(*(HWND*)0x64A3298, *(LPCSTR*)0x6FAC0D);
        Com_sprintf(buffer, 1024, "]%s\n", string);
        memory::call<void(char*)>(0x4914B0)(buffer);
        return 0;
    }

    return CallWindowProcA(*(WNDPROC*)0x64A38A4, hWnd, msg, wParam, lParam);
}

//THUNK : 0x0042F0A0
void Sys_InitializeCriticalSections()
{
    memory::call<void()>(0x0042F0A0)();
}

//THUNK : 0x004301B0
void Sys_InitMainThread()
{
    memory::call<void()>(0x004301B0)();
}

//DONE : 0x004169C0
void Sys_Sleep(DWORD dwMilliseconds)
{
    Sleep(dwMilliseconds);
}

//THUNK but needs to move!
//TODO : 0x0064C620
bool IsServerRunning()
{
    return memory::call<bool()>(0x0064C620)();
}

//DONE : 0x0043EBB0
void Sys_CheckQuitRequest()
{
    int FirstActiveLocalClient; // eax
    auto onlinegame = (char*)(0x00B2BB48);
    auto xblive_privatematch = (char*)(0x0649E714);
    auto dword_649FF68 = (int*)(0x649FF68);

    if (*(bool*)(0x0649FB61)/*g_quitRequested*/ && Sys_IsMainThread())
    {
        if ( onlinegame[16]
            && !xblive_privatematch[16]
            && IsServerRunning()
            && Sys_Milliseconds() - *(int*)(0x0649FF94) < dword_649FF68[4])
        {
            if (!*(bool*)(0x0649FB60)/*g_quitMigrationStarted*/)
            {
                *(bool*)(0x0649FB60)/*g_quitMigrationStarted*/ = true;
                FirstActiveLocalClient = CL_ControllerIndexFromClientNum();
                Cbuf_AddText(FirstActiveLocalClient, "hostmigration_start\n");
            }
        }
        else
        {
            *(int*)(0x00A7FE90)/*cls*/ = 1;
            Cbuf_AddText(0, "quit\n");
        }
    }
}

//THUNK : 0x004C8E30
void Sys_RecordAccessibilityShortcutSettings()
{
    memory::call<void()>(0x004C8E30)();
}

//THUNK : 0x0040BC60
void Sys_AllowAccessibilityShortcutKeys(bool a1)
{
    memory::call<void(bool)>(0x0040BC60)(a1);
}

//DONE : 0x00475F00
void Sys_CreateSplashWindow()
{
    int SystemMetrics; // edi
    int v1; // ebx
    HANDLE ImageA; // ebp
    HWND Window; // eax
    HWND v4; // eax
    HWND v5; // esi
    int v6; // ecx
    int v7; // esi
    struct tagRECT Rect; // [esp+0h] [ebp-38h] BYREF
    WNDCLASSA WndClass; // [esp+10h] [ebp-28h] BYREF

    WndClass.style = 0;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.lpszMenuName = 0;
    WndClass.lpfnWndProc = DefWindowProcA;
    WndClass.hInstance = *(HINSTANCE*)(0x064A3AD4);
    WndClass.hIcon = LoadIconA(*(HINSTANCE*)(0x064A3AD4), (LPCSTR)1);
    WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH)6;
    WndClass.lpszClassName = "CoD Splash Screen";
    if (RegisterClassA(&WndClass))
    {
        SystemMetrics = GetSystemMetrics(16);
        v1 = GetSystemMetrics(17);
        ImageA = LoadImageA(0, "openiw4-splash.bmp", 0, 0, 0, 0x10u);
        if (ImageA)
        {
            memory::call<char()>(0x0045F8B0)();
            Window = CreateWindowExA(
                0x40000u,
                "CoD Splash Screen",
                "Modern Warfare 2 Multiplayer",
                0x80880000,
                (SystemMetrics - 320) / 2,
                (v1 - 100) / 2,
                320,
                100,
                0,
                0,
                *(HINSTANCE*)(0x064A3AD4),
                0);
            *(HWND*)(0x064A3050) /*g_splash_wv*/ = Window;
            if (Window)
            {
                v4 = CreateWindowExA(0, "Static", 0, 0x5000000Eu, 0, 0, 320, 100, Window, 0, *(HINSTANCE*)(0x064A3AD4), 0);
                v5 = v4;
                if (v4)
                {
                    SendMessageA(v4, 0x172u, 0, (LPARAM)ImageA);
                    GetWindowRect(v5, &Rect);
                    v6 = Rect.right - Rect.left + 2;
                    Rect.left = (SystemMetrics - v6) / 2;
                    v7 = Rect.bottom - Rect.top + 2;
                    Rect.top = (v1 - v7) / 2;
                    Rect.right = Rect.left + v6;
                    Rect.bottom = Rect.top + v7;
                    AdjustWindowRect(&Rect, 0x5000000Eu, 0);
                    SetWindowPos(*(HWND*)(0x064A3050) /*g_splash_wv*/, 0, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, 4u);
                }
            }
        }
    }
}

//DONE : 0x004A7B10
void Sys_ShowSplashWindow()
{
    if (*(HWND*)(0x064A3050)/*g_splash_wv*/)
    {
        ShowWindow(*(HWND*)(0x064A3050)/*g_splash_wv*/, 5);
        UpdateWindow(*(HWND*)(0x064A3050)/*g_splash_wv*/);
    }
}

//THUNK : 0x0064D270
void Sys_RegisterClass()
{
    memory::call<void()>(0x0064D270)();
}

//DONE : 0x0042A660
int Sys_Milliseconds()
{
    if (!*(int*)(0x064A304C)/*Sys_Milliseconds(void)::initialized*/)
    {
        *(int*)(0x064A3034) /*sys_timeBase*/ = timeGetTime();
        *(int*)(0x064A304C)/*Sys_Milliseconds(void)::initialized*/ = 1;
    }
    return timeGetTime() - *(int*)(0x064A3034) /*sys_timeBase*/;
}

//THUNK : 0x004EC730
int* Sys_GetValue(int a1)
{
    return memory::call<int*(int)>(0x004EC730)(a1);
}

//THUNK : 0x0064D300
void Sys_getcwd()
{
    memory::call<void()>(0x0064D300)();
}

//THUNK : 0x004C37D0
bool Sys_IsMainThread()
{
    return memory::call<bool()>(0x004C37D0)();
}

//DONE : 0x004FC200
void Sys_EnterCriticalSection(int a1)
{
    EnterCriticalSection((LPCRITICAL_SECTION)(24 * a1 + 0x6499BC8));
}

//DONE : 0x004FC200
void Sys_LeaveCriticalSection(int a1)
{
    LeaveCriticalSection((LPCRITICAL_SECTION)(24 * a1 + 105487304));
}

//THUNK : 0x0064CF10
void Sys_EnumerateHw()
{
    memory::call<void()>(0x0064CF10)();
}

//DONE : 0x004CA4A0
bool Sys_IsDatabaseReady()
{
    return WaitForSingleObject(*(HANDLE*)(0x01CDE7F8)/*databaseCompletedEvent*/, 0) == 0;
}

//DONE : 0x00441280
bool Sys_IsDatabaseReady2()
{
    return WaitForSingleObject(*(HANDLE*)(0x01CDE858)/*databaseCompletedEvent2*/, 0) == 0;
}

//THUNK : 0x0043D570
void Sys_Error(char* error, ...)
{
    va_list args;
    va_start(args, error);
    vprintf(error, args);
    va_end(args);

    memory::call<void(char*, va_list)>(0x0043D570)(error, args);
}

//DONE : 0x004F5250
bool Sys_ReleaseThreadOwnership()
{
    return SetEvent(*(HANDLE*)(0x01CDE704) /*noThreadOwnershipEvent*/);
}

//DONE : 0x004C3650
bool Sys_DatabaseCompleted()
{
    Sys_EnterCriticalSection(12);
    *(DWORD*)0x1CDE84C = 1; //dword_1CDE84C
    Sys_LeaveCriticalSection(12);

    HANDLE result = *(HANDLE*)0x1CDE85C; //dword_1CDE85C

    if(result)
    {
        WaitForSingleObject(result, INFINITE);
    }

    HANDLE event = *(HANDLE*)0x1CDE7F8; //dword_1CDE7F8
    return SetEvent(event);
}