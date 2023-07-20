#pragma once
#if _WIN32
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/socket.h>
#endif

#include "HString.h"
//
#include "Common.h"
#include <thread>
#include <map>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
namespace ConsoleDebug
{
    extern void CreateConsole(HString consolePath, bool bNoClient = false);
    extern void CleanupConsole();
    extern void ReadMsgFromConsole();

    extern HANDLE ReadMessageThread;
    extern SOCKET tcpSocket;
    extern std::vector<SOCKET> consoleSockets;
    extern WORD versionRequest;
    extern WSADATA wsaData;
    extern int err;
    extern bool bConnectedConsole;
    extern bool bConnectedFailed;
    extern struct sockaddr_in consoleAddr;
    extern struct sockaddr_in addr;
    extern FILE* log_file;
    extern std::function<void(HString,float ,float,float, HString)> printFuncAdd;

    extern STARTUPINFO si;
    extern PROCESS_INFORMATION pi;
    //自定义控制台命令：
    extern std::map<HString, std::function<void()>> commandLists;
    extern void AddNewCommand(HString newCommand, std::function<void()>func, int ParamerterCount = 0, ...);
    extern void execCommand(HString key);

    /* 输出信息到控制台,带字体颜色，带自动换行 */
    HBBR_API extern void print_endl(HString in, HString color = "255,255,255", HString background = "0,0,0", HString type = " ");
    HBBR_API extern void print(HString in, HString color = "255,255,255", HString background = "0,0,0", HString type = " ");

    extern HANDLE socketAcceptThread;

};
