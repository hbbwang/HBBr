#pragma once
#include "Common.h"
//CallStack 程序崩溃的时候跟踪堆栈

#if _WIN32 // SetUnhandledExceptionFilter(ApplicationCrashHandler);
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
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
            ConsoleDebug::printf_endl_error("%s", formattedString );
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
#elif __ANDROID__

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
