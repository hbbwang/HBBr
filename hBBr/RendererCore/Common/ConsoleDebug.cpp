#include <time.h>
#include "ConsoleDebug.h"
#include <mutex>
HANDLE ConsoleDebug::ReadMessageThread = NULL;
SOCKET ConsoleDebug::tcpSocket;
std::vector<SOCKET> ConsoleDebug::consoleSockets;
WORD ConsoleDebug::versionRequest;
WSADATA ConsoleDebug::wsaData;
struct sockaddr_in ConsoleDebug::consoleAddr;
struct sockaddr_in ConsoleDebug::addr;
int ConsoleDebug::err;
bool ConsoleDebug::bConnectedConsole = false;
bool ConsoleDebug::bConnectedFailed = false;
HANDLE ConsoleDebug::socketAcceptThread;
FILE* ConsoleDebug::log_file = NULL;
std::function<void(HString,  float, float, float, HString)> ConsoleDebug::printFuncAdd = [](HString, float, float, float, HString) {};

STARTUPINFO ConsoleDebug::si;
PROCESS_INFORMATION ConsoleDebug::pi;

std::mutex g_num_mutex;

HString rgb2hex(int r, int g, int b, bool with_head)
{
    std::stringstream ss;
    if (with_head)
        ss << "##";
    ss << std::hex << (r << 16 | g << 8 | b);
    return HString(ss.str().c_str());
}

std::map<HString, std::function<void()>> ConsoleDebug::commandLists;
DWORD WINAPI ReadConsoleMsgThreadFunc(LPVOID lpParamter)
{
    int acceptTimeOut = 0;
    while (true)
    {
        {
            _Sleep(1);
            ConsoleDebug::ReadMsgFromConsole();
        }
    }
    return 0;
}

DWORD WINAPI SocketAcceptThreadFunc(LPVOID lpParamter)
{
    while (true)
    {
        _Sleep(500);
        sockaddr_in addrClient;
        int addrClientlen = sizeof(addrClient);
        ConsoleDebug::consoleSockets.push_back(accept(ConsoleDebug::tcpSocket, (sockaddr FAR*) & addrClient, &addrClientlen));
    }
    return 0;
}

void ConsoleDebug::CreateConsole(HString consolePath ,bool bNoClient)
{
    if (ConsoleDebug::bConnectedConsole == true && ConsoleDebug::bConnectedFailed == false)
        return;
    ConsoleDebug::bConnectedFailed = false;
    //Socket
    versionRequest = MAKEWORD(2, 2);
    err = WSAStartup(versionRequest, &wsaData);
    if (err != 0)
    {
        printf("嵌套字未打开");
        WSACleanup();
        return;
    }
    else
    {
        printf("已打开套接字");
    }
    tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in daddr;
    memset((void*)&daddr, 0, sizeof(daddr));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    daddr.sin_port = htons(5050);

    auto retVal = ::bind(tcpSocket, (struct sockaddr*)&daddr, sizeof(daddr));
    if (SOCKET_ERROR == retVal)
    {
        printf("bind failed!\n");
        closesocket(tcpSocket);
        WSACleanup();
        return;
    }


    retVal = listen(tcpSocket, 1);
    if (SOCKET_ERROR == retVal)
    {
        printf("listen failed!\n");
        closesocket(tcpSocket);
        WSACleanup();
        return;
    }
    if (!bNoClient)
    {
        //system(consolePath.c_str());
        //HINSTANCE hi = ShellExecute(hwnd, L"open", consolePath.c_wstr(), NULL, NULL, SW_SHOWNORMAL);
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        CreateProcess(NULL,   // No module name (use command line)
            (TCHAR*)consolePath.c_wstr(),        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi);           // Pointer to PROCESS_INFORMATION structure
    }
   
    //接收来自客户端的请求
    printf("TCPServer start...\n");
    socketAcceptThread = CreateThread(NULL, 0, SocketAcceptThreadFunc, 0, 0, 0);

    ConsoleDebug::bConnectedConsole = true;
    _Sleep(25);
    ConsoleDebug::print_endl("Connect Console Succeed!", "0,255,0");
    ReadMessageThread = CreateThread(NULL, 0, ReadConsoleMsgThreadFunc, 0, 0, 0);

    //Create Log file

    auto err_src_name = fopen_s(&log_file, ".\\log.txt" , "w+");
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }

}

void ConsoleDebug::CleanupConsole()
{
    closesocket(tcpSocket);
    WSACleanup();
    bConnectedConsole = false;
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(ReadMessageThread);
    CloseHandle(socketAcceptThread);
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

void ConsoleDebug::ReadMsgFromConsole()
{
    char buffer[4096];
    memset(buffer, '\0', sizeof(buffer));
    for (int i = 0; i < consoleSockets.size(); i++) {
        int rec = recv(consoleSockets[i], buffer, 4096, 0);
        buffer[4095] = '\0';
        if (rec > 0)
        {
            //MessageBoxA(0, buffer, 0, 0);
            execCommand(buffer);
        }
    }
}

char* UnicodeToUtf8(const wchar_t* unicode)
{
    int len;
    len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
    char* szUtf8 = (char*)malloc(len + 1);
    memset(szUtf8, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
    return szUtf8;
}

void ConsoleDebug::print(HString in, HString color, HString background, HString type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.Length() > 0 )
    {
        // 基于当前系统的当前日期/时间
        time_t now = time(0);
        // 把 now 转换为字符串形式
        struct tm tTm;
        localtime_s(&tTm, &now);
        tTm.tm_year += 1900;
        tTm.tm_mon++;
        char szTime[128];
        sprintf_s(szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
            tTm.tm_year, tTm.tm_mon, tTm.tm_mday,
            tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

        HString Data = szTime;

        std::vector<HString> colorArray = color.Split(",");
        float r, g, b;
        r = (float)HString::ToDouble(colorArray[0]);
        g = (float)HString::ToDouble(colorArray[1]);
        b = (float)HString::ToDouble(colorArray[2]);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
        WORD wr2 = 0 ;//方法二用系统宏定义颜色属性
        if (r > 30)
        {
            wr2 |= FOREGROUND_RED;
        }
        if (g > 30)
        {
            wr2 |= FOREGROUND_GREEN;
        }
        if (b > 30)
        {
            wr2 |= FOREGROUND_BLUE;
        }
        SetConsoleTextAttribute(hOut, wr2);

        Data = "[" + Data + "]";
        HString nIn = Data + in;
        printf(nIn.c_str());
        OutputDebugStringA(nIn.c_str());
        printFuncAdd(nIn, r / 255.0f, g / 255, b / 255, Data);

        auto err_src_name = fopen_s(&log_file, ".\\log.txt", "a+");
        if (log_file)
        {
            fwrite(nIn.c_str(), strlen(nIn.c_str()), 1, log_file);
            fclose(log_file);
            log_file = NULL;
        }
        if (bConnectedConsole == true)
        {
            HString out = nIn;
            //memset(buffer, 0, sizeof(buffer));
            out += "||//||" + color + "||//||" + background + "||//||" + Data;
            char* buffer = UnicodeToUtf8(out.c_wstr());
            //sprintf(buffer, UnicodeToUtf8(out.c_wstr()));

            for (int i = 0; i < consoleSockets.size(); i++) 
            {
                if (send(consoleSockets[i], buffer, 4096, 0) < 0)
                {
#ifdef _DEBUG
                    HString cs = "写入数据失败！";
                    cs += HString::FromUInt(::GetLastError());
                    printf(cs.c_str());
                    printf("\n");
#endif
                }
            }
            free(buffer);
        }
    }
}

void ConsoleDebug::print_endl(HString in, HString color, HString background, HString type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.Length() > 0 )
    {
        // 基于当前系统的当前日期/时间
        time_t now = time(0);
        // 把 now 转换为字符串形式
        struct tm tTm;
        localtime_s(&tTm, &now);
        tTm.tm_year += 1900;
        tTm.tm_mon++;
        char szTime[128];
        sprintf_s(szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
            tTm.tm_year, tTm.tm_mon, tTm.tm_mday,
            tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

        HString Data = szTime;

        std::vector<HString> colorArray = color.Split(",");
        float r, g, b;
        r = (float)HString::ToDouble(colorArray[0]);
        g = (float)HString::ToDouble(colorArray[1]);
        b = (float)HString::ToDouble(colorArray[2]);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
        WORD wr2 = 0;//方法二用系统宏定义颜色属性
        if (r > 30)
        {
            wr2 |= FOREGROUND_RED;
        }
        if (g > 30)
        {
            wr2 |= FOREGROUND_GREEN;
        }
        if (b > 30)
        {
            wr2 |= FOREGROUND_BLUE;
        }
        SetConsoleTextAttribute(hOut, wr2);

        Data = "[" + Data + "] ";
        printf((Data+in).c_str());
        printf("\n");

        OutputDebugStringA(((Data + in) +"\n").c_str());
        HString nIn = (Data + in) + "\n";

        printFuncAdd(nIn, r / 255.0f, g / 255, b / 255, Data);

        auto err_src_name = fopen_s(&log_file, ".\\log.txt", "a+");
        if (log_file != NULL)
        {
            fwrite(nIn.c_str(), strlen(nIn.c_str()), 1, log_file);
            fclose(log_file);
            log_file = NULL;
        }
        if (bConnectedConsole == true)
        {
            HString out = (Data + in);
            //memset(buffer, 0, sizeof(buffer));
            out += "\n||//||" + color + "||//||" + background + "||//||" + Data;
            char* buffer = UnicodeToUtf8(out.c_wstr());

            //sprintf(buffer, UnicodeToUtf8(out.c_wstr()));

            for (int i = 0; i < consoleSockets.size(); i++)
            {
                if (send(consoleSockets[i], buffer, 4096, 0) < 0)
                {
#ifdef _DEBUG
                    HString cs = "写入数据失败！";
                    cs += HString::FromUInt(::GetLastError());
                    printf(cs.c_str());
                    printf("\n");
#endif
                }
            }
            free(buffer);
        }
    }
}

void ConsoleDebug::AddNewCommand(HString newCommand, std::function<void()> func, int ParamerterCount, ...)
{
    std::map<HString, std::function<void()>>::iterator it;
    for (it = commandLists.begin(); it != commandLists.end(); it++)
    {
        if (it->first.IsSame(newCommand, false))
        {
            print_endl("Failed to add command [" + newCommand + "].This command has been existed.", "255,255,10");
            return;
        }
    }

    commandLists.insert(std::pair<HString, std::function<void()>>(newCommand, func));
}

/* 命令执行 */
void ConsoleDebug::execCommand(HString key)
{

}