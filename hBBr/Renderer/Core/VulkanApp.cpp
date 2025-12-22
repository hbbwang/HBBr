#include "../Common/Common.h"
#include "../Common/ConsoleDebug.h"
#include <string>
#include "VulkanApp.h"
#include "VulkanManager.h"

//CallStack 程序崩溃的时候跟踪堆栈
#pragma region CallStack
#ifdef _WIN32 // SetUnhandledExceptionFilter(ApplicationCrashHandler);
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#define TRACE_MAX_STACK_FRAMES 1024
#define TRACE_MAX_FUNCTION_NAME_LENGTH 1024
int printStackTrace(std::string* outStr = nullptr)
{
    void* stack[TRACE_MAX_STACK_FRAMES];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    WORD numberOfFrames = CaptureStackBackTrace(0, TRACE_MAX_STACK_FRAMES, stack, nullptr);
    char buf[sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR)];
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)buf;
    symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD displacement;
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    for (int i = 2; i < numberOfFrames; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(process, address, nullptr, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, &line))
        {
            char* formattedString = nullptr;
            SDL_asprintf(&formattedString, "address: 0x%0X : %s(line %lu) %s", (unsigned int)symbol->Address, line.FileName, line.LineNumber, symbol->Name);
            if (outStr)
            {
                outStr->append(formattedString);
                outStr->append("\n");
            }
            ConsoleDebug::printf_endl_error("%s", formattedString);
        }
        //else
        //{
        //    printf("SymGetLineFromAddr64 returned error code %lu.\n", GetLastError());
        //    printf("address: 0x%0X , at %s.\n", (unsigned int)symbol->Address, symbol->Name);
        //}
    }
    return 0;
}
LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException) {
    // 获取时间
    SYSTEMTIME syst;
    GetLocalTime(&syst);
    CHAR strDateTime[256];
    sprintf_s(strDateTime, 256, ".%d.%.2d.%.2d.%.2d.%.2d.%.2d.%.3d.dmp", syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, syst.wMilliseconds);
    // 获取文件完整路径
    CHAR szFilename[MAX_PATH];
    GetModuleFileNameA(nullptr, szFilename, MAX_PATH);
    // 输出文件名称
    std::string outfilename;
    outfilename += szFilename;
    outfilename += strDateTime;
    MakeSureDirectoryPathExists(outfilename.c_str());
    std::string outStr = "*** Unhandled Exception! ***\0";
    ConsoleDebug::printf_endl_error(outStr.c_str());
    printStackTrace(&outStr);
    DE_ASSERT(0, outStr.c_str());

    return EXCEPTION_EXECUTE_HANDLER;
}
#elif defined(__ANDROID__)
#else
#endif
void CallStack()
{
#if _WIN32
    SetUnhandledExceptionFilter(ApplicationCrashHandler);
#elif __ANDROID__
#else
#endif
}
#pragma endregion

std::unique_ptr<VulkanApp> VulkanApp::Instance;

thread_local bool bIsMianThread = false;
thread_local bool bIsRenderThread = false;
thread_local bool bInitialize = false;
bool bSdlQuit = false;//只在主线程Set,其他线程只读
bool bIsEnableDebug = false;

void VulkanApp::InitVulkanManager(bool bEnableDebug)
{
#if _WIN32
    SetConsoleOutputCP(65001); // 设置控制台输出代码页为 UTF-8
#endif
    CallStack();

    ConsoleDebug::printf_endl("Init SDL3.");
    if (!SDL_Init(
        SDL_INIT_AUDIO
        | SDL_INIT_VIDEO
        | SDL_INIT_JOYSTICK
        | SDL_INIT_HAPTIC
        | SDL_INIT_GAMEPAD
        | SDL_INIT_EVENTS
        | SDL_INIT_SENSOR
    ))
    {
        MessageOut("Init sdl3 window failed.", true, true, "255,0,0");
        return;
    }
    bIsMianThread = true;
    if (!SDL_Vulkan_LoadLibrary(nullptr))
    {
        MessageOut(SDL_GetError(), true, true, "255,0,0");
    }
    //Set sdl hints
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "0");
    bIsEnableDebug = bEnableDebug;
    //Init Vulkan Manager (Main Thread)
    VulkanManager::Get()->InitManager_MainThread(bIsEnableDebug);
    //Create Render Thread
    RenderThread = std::thread([this]()
    {
        ConsoleDebug::printf_endl("Init RenderThread.");
        //Init Vulkan Manager (Render Thread)
        VulkanManager::Get()->InitManager_RenderThread();
        bIsRenderThread = true;
        while (RenderLoop())
        {
        }
    });
}

VkWindow* VulkanApp::CreateVulkanWindow(int w, int h, const char* title)
{
	//Create Vulkan Window
    auto newWindow = new VkWindow(w, h, title);
    newWindow->SetFocus();
    //Create Renderer...
    AllWindows.push_back(newWindow);
    return newWindow;
}

bool VulkanApp::MainLoop()
{
    bool bContinueLoop = true;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        auto window = GetWindowFromID(event.window.windowID);
        switch (event.type)
        {
		    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: //窗口关闭事件
		    { 
                ConsoleDebug::printf_endl("Close window : {}", window->GetTitle().c_str());
                break;
		    }
		    case SDL_EVENT_WINDOW_FOCUS_GAINED:
		    {			
                break;
		    }
		    case SDL_EVENT_WINDOW_FOCUS_LOST:
		    {			
                break;
		    }
		    //case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		    //case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		    case SDL_EVENT_WINDOW_RESIZED:
		    {
                break;
		    }
		    case SDL_EVENT_QUIT://窗口完全关闭,SDL即将退出
		    {		
                bContinueLoop = false;
                bSdlQuit = true;
                RenderThread.join();
                break;
		    }
		    case SDL_EVENT_MOUSE_BUTTON_DOWN:
		    case SDL_EVENT_MOUSE_BUTTON_UP:
		    {
                break;
		    }
		    case SDL_EVENT_KEY_DOWN:
		    case SDL_EVENT_KEY_UP:
		    {
			    break;
		    }
		    case SDL_EVENT_MOUSE_WHEEL:
		    {
			    break;
		    }
		    case SDL_EVENT_DROP_FILE:
		    {
			    break;
		    }
		    case SDL_EVENT_WINDOW_EXPOSED:
		    case SDL_EVENT_WINDOW_SHOWN:
		    {
			    break;
		    }
		    case SDL_EVENT_WINDOW_MINIMIZED:
		    case SDL_EVENT_WINDOW_HIDDEN:
		    {
			    break;
		    }
		    case SDL_EVENT_FINGER_DOWN:
		    {
			    //SDL_ShowSimpleMessageBox(0, "", "(test)手指按下", nullptr);
			    break;
		    }
        }
    }
	//GameObject main loop code here
    {
        if (!bInitialize)
        {
            ConsoleDebug::printf_endl("Init GameMainLoop.");
            bInitialize = true;
        }
        else
        {
            for (auto& w : AllWindows)
            {
				w->Update_MainThread();
            }
        }
    }
    return bContinueLoop;
}

bool VulkanApp::RenderLoop()
{
    bool bContinueLoop = true;
	//Render code here
    {
        if (!bInitialize)
        {
            ConsoleDebug::printf_endl("Init RenderMainLoop.");
            bInitialize = true;
        }
        else
        {
            std::vector<std::function<void()>>funcs;
            while (RenderThreadFuncs.try_dequeue(funcs))
            {
                for (auto& f : funcs)
                    f();
                funcs.clear();
            }
            for (auto& w : AllWindows)
            {
                w->Update_RenderThead();
            }
        }
		//Exit render loop.Must be insure render objects are destroyed before quit.务必确保渲染对象在退出前被销毁完毕
        if (bSdlQuit)
        {
            bContinueLoop = false;
        }
    }
    return bContinueLoop;
}

void VulkanApp::Release()
{
    ConsoleDebug::printf_endl("Release VulkanApp.");
    VulkanManager::Get()->ReleaseManager();
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
    if (Instance)
        Instance.reset();
}

VkWindow* VulkanApp::GetFocusWindow()
{
    for (auto window : AllWindows)
    {
        SDL_WindowFlags windowFlags = SDL_GetWindowFlags(window->GetWindowHandle());
        if (windowFlags & SDL_WINDOW_INPUT_FOCUS)
        {
            return window;
        }
    }
    return nullptr;
}

VkWindow* VulkanApp::GetWindowFromID(SDL_WindowID id)
{
    for (auto window : AllWindows)
    {
        if (window->GetWindowID() == id)
        {
            return window;
        }
	}
    return nullptr;
}

void VulkanApp::EnqueueRenderFunc(std::function<void()> func)
{
    RenderThreadFuncs.enqueue(std::move(func));
}
