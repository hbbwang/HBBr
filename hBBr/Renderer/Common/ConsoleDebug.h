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

#include <thread>
#include <map>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ostream>
#include <string>
#include <vector>
#include "StringTools.h"
#include "FileSystem.h"

namespace ConsoleDebug
{
    extern void CreateConsole(std::string consolePath, bool bNoClient = false);
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

    //自定义控制台命令：
    extern std::map<std::string, std::function<void()>> commandLists;
    extern void AddNewCommand(std::string newCommand, std::function<void()>func, int ParamerterCount = 0, ...);
    extern void execCommand(std::string key);

    /* 输出信息到控制台,带字体颜色，带自动换行 */
    HBBR_API extern void print_endl(std::string in, std::string color = "255,255,255", std::string background = "0,0,0", std::string type = " ");
    HBBR_API extern void print(std::string in, std::string color = "255,255,255", std::string background = "0,0,0", std::string type = " ");

    template<typename ...Arg>
    extern void printf_endl(std::string in , Arg...args)
    {
        print_endl(StringTool::vformat(in.c_str(), args...));
    }
    template<typename ...Arg>
    extern void printf_endl_warning(std::string in, Arg...args)
    {
        print_endl(StringTool::vformat(in.c_str(), args...), "255,255,0"); 
    }
    template<typename ...Arg>
    extern void printf_endl_error(std::string in, Arg...args)
    {
        print_endl(StringTool::vformat(in.c_str(), args...), "255,50,0");
    }
    template<typename ...Arg>
    extern void printf_endl_succeed(std::string in, Arg...args)
    {
        print_endl(StringTool::vformat(in.c_str(), args...), "50,255,60");
    }

    extern std::thread socketAcceptThread;

};
