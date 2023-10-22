#pragma once
#if _WIN32
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
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
#include <ostream>
#include <string>
#include <vector>

#include "FileSystem.h"

namespace ConsoleDebug
{
    extern void CreateConsole(HString consolePath, bool bNoClient = false);
    extern void CleanupConsole();
    extern void ReadMsgFromConsole();

    extern std::thread ReadMessageThread;

    extern SOCKET tcpSocket;
    extern std::vector<SOCKET> consoleSockets;

    extern int err;
    extern bool bConnectedConsole;
    extern bool bConnectedFailed;
    extern struct sockaddr_in consoleAddr;
    extern struct sockaddr_in addr;
    extern std::function<void(HString,float ,float,float, HString)> printFuncAdd;


    //自定义控制台命令：
    extern std::map<HString, std::function<void()>> commandLists;
    extern void AddNewCommand(HString newCommand, std::function<void()>func, int ParamerterCount = 0, ...);
    extern void execCommand(HString key);

    /* 输出信息到控制台,带字体颜色，带自动换行 */
    HBBR_API extern void print_endl(HString in, HString color = "255,255,255", HString background = "0,0,0", HString type = " ");
    HBBR_API extern void print(HString in, HString color = "255,255,255", HString background = "0,0,0", HString type = " ");

    template<typename ...Arg>
    extern void printf_endl(HString in , Arg...args)
    {
        char* formattedString = nullptr;
        SDL_asprintf(&formattedString, in.c_str(), args...);
        print_endl(formattedString);
    }
    template<typename ...Arg>
    extern void printf_endl_warning(HString in, Arg...args)
    {
        char* formattedString = nullptr;
        SDL_asprintf(&formattedString, in.c_str(), args...);
        print_endl(formattedString, "255,255,0");
    }
    template<typename ...Arg>
    extern void printf_endl_error(HString in, Arg...args)
    {
        char* formattedString = nullptr;
        SDL_asprintf(&formattedString, in.c_str(), args...);
        print_endl(formattedString, "255,0,0");
    }

    extern std::thread socketAcceptThread;

};
