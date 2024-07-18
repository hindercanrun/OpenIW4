#include "main.hpp"
#include "loader/loader.hpp"

#include "Sys/Sys.hpp"
#include "Com/Com.hpp"
#include "Win/Win.hpp"
#include "Dvar/Dvar.hpp"
#include "Cmd/Cmd.hpp"
#include "Cbuf/Cbuf.hpp"
#include "DB/DB.hpp"
#include "LSP/LSP.hpp"
#include "Images/Images.hpp"

#include "MSG/MSG.hpp"
#include "Huffman/Huffman.hpp"

#include "defs.hpp"

#include <utils/memory/memory.hpp>

#define NOUPNP

//TODO : 0x0064AE50
double SecondsPerTick()
{
	//return memory::call<double()>(0x0064AE50)();

    memory::call<void(std::int32_t)>(0x4A5E00)(2);

    std::uint64_t time, time2;
    long long v8;

    HANDLE currentThread = GetCurrentThread();
    std::int32_t nPriority = GetThreadPriority(currentThread);
    SetThreadPriority(currentThread, nPriority);
    Sleep(0); //why is this called, IW?

    LARGE_INTEGER performanceCount;
    performanceCount.QuadPart = 0;
    LARGE_INTEGER v4;
    v4.QuadPart = 0;
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&performanceCount);

    time = __rdtsc();

    do
    {
        QueryPerformanceCounter(&v4);
        v8 = v4.QuadPart - performanceCount.QuadPart;
    } while ((std::double_t)(v4.QuadPart - performanceCount.QuadPart) / (std::double_t)frequency.QuadPart <= 0.25);

    time2 = __rdtsc();

    QueryPerformanceCounter(&v4);

    *(std::double_t*)&v8 = (std::double_t)(v4.QuadPart - performanceCount.QuadPart)
        /
        (((std::double_t)(std::int64_t)time2 - time) * (std::double_t)frequency.QuadPart);
    SetThreadPriority(currentThread, nPriority);

    memory::call<void(std::int32_t)>(0x4A5E00)(0);

    return *(std::double_t*)&v8;
}

//TODO : 0x47ADF0
void InitTiming()
{
	//*(std::double_t*)(0x47ADF0) /*msecPerRawTimerTick*/ = SecondsPerTick() * 1000.0;
    memory::call<void()>(0x48D750)();
}

//DONE : 0x437EB0
void Field_Clear(field_t* edit)
{
    memset(edit->buffer, 0, sizeof(edit->buffer));

    edit->cursor = 0;
    edit->scroll = 0;
    edit->drawWidth = 256;
}

//This function is blank in 159
//DONE : 0x4BB9B0
void Session_InitDvars()
{
    
}

//Temp fix until further reimp

void* ReallocateAssetPool(XAssetType type, std::size_t newSize)
{
    auto DB_XAssetPool = (void**)0x7337F8;
    auto g_poolSize = (std::uint32_t*)0x733510;

    auto size = DB_GetXAssetTypeSize(type);
    auto poolEntry = malloc(newSize * size);
    DB_XAssetPool[type] = poolEntry;
    g_poolSize[type] = newSize;

    return poolEntry;
}

void testMe()
{
    Com_Printf(16, "testMe was called\n");
    Sys_Error("Oh god oh fuck");
}

void patches()
{
#if defined(NOUPNP) && defined(DEBUG)
    memory::kill(0x405410);
#endif
    Sys_ShowConsole();
    ReallocateAssetPool(ASSET_TYPE_WEAPON, 2400);
    static const dvar_t* cg_fov = *reinterpret_cast<dvar_t**>(0x861968);
    cg_fov = Dvar_RegisterFloat("cg_fov", 90.0f, 0.0f, FLT_MAX, 68, "The field of view angle in degrees");

	//ignore IWI version check
	memory::set<std::uint8_t>(0x544746, 0xEB);
}

//DONE : 0x004513D0
std::int32_t main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	patches();

	const char* LocalizationFilename; // eax
	char* error_msg; // eax

	Sys_InitializeCriticalSections();
	Sys_InitMainThread();
	if (Win_InitLocalization(0))
	{
#ifdef MATCHING
		if (!I_strnicmp(lpCmdLine, "allowdupe", 9) && lpCmdLine[9] <= 32 || (Sys_GetSemaphoreFileName(), Sys_CheckCrashOrRerun()))
		{
#endif
			if (!hPrevInstance)
			{
				Com_InitParse();
				Dvar_Init();
				InitTiming();
				Sys_EnumerateHw();
				Sys_RecordAccessibilityShortcutSettings();
				Sys_AllowAccessibilityShortcutKeys(0);
				*(HINSTANCE*)(0x1A04788) = 0;
				I_strncpyz((char*)(0x1A00840) /*sys_cmdline*/, lpCmdLine, 1024);
				Sys_CreateSplashWindow();
				Sys_ShowSplashWindow();
				Sys_RegisterClass();
				SetErrorMode(1u);
				Sys_Milliseconds();
				Session_InitDvars();
				Com_Init((char*)(0x1A00840) /*sys_cmdline*/);
				Sys_getcwd();
				SetFocus(*(HWND*)(0x1A04784) /*g_wv*/);

				cmd_function_s testFunction;
				Cmd_AddCommandInternal("testMe", testMe, &testFunction, 0);

				while (1)
				{
					if (*(int*)(0x1A04790))
						Sys_Sleep(5u);
					if (*(char*)(0x1A02ADC))
					{
						if (Sys_IsMainThread())
						{
							Cbuf_AddText(0, "quit\n");
						}
					}
					Com_Frame();
				}
			}
#ifdef MATCHING
        }
#endif
		Win_ShutdownLocalization();
		return 0;
	}
	else
	{
		LocalizationFilename = Win_GetLocalizationFilename();
		error_msg = va(
			"Could not load %s.\n\nPlease make sure Modern Warfare 2 is run from the correct folder.",
			LocalizationFilename);
		MessageBoxA(0, error_msg, "Modern Warfare 2 - Fatal Error", MB_ICONHAND);
		return 0;
	}
}

void replace_funcs()
{
    memory::replace(0x4A7910, main);
    memory::replace(0x42C830, Sys_ShowConsole);
    // memory::replace(0x40BFF0, Sys_Error);
    // memory::replace(0x413600, DB_DirtyDiscError);
	// memory::replace(0x544730, Image_VerifyHeader);
}

std::int32_t __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, std::int32_t nShowCmd)
{
    loader::load("iw4sp.exe");
    replace_funcs();
    return memory::call<std::int32_t()>(0x67A16F)();
}
