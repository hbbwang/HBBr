#include <ctime>
#include "ConsoleDebug.h"
#include "HTime.h"
#include <mutex>
//#include <iconv.h>
#ifdef _WIN32

#define CloseSocket(socket) closesocket(socket)
#define WSACleanup()    WSACleanup()
WORD versionRequest;
WSADATA wsaData;
STARTUPINFO si;
PROCESS_INFORMATION pi;
#else
#define CloseSocket(socket) close(socket)
#define WSACleanup() 
#endif

std::thread ConsoleDebug::ReadMessageThread;

SOCKET ConsoleDebug::tcpSocket;
std::vector<SOCKET> ConsoleDebug::consoleSockets;

struct sockaddr_in ConsoleDebug::consoleAddr;
struct sockaddr_in ConsoleDebug::addr;
int ConsoleDebug::err;
bool ConsoleDebug::bConnectedConsole = false;
bool ConsoleDebug::bConnectedFailed = false;
std::thread ConsoleDebug::socketAcceptThread;
HString LogFileName = (HString(".\\log_") + HTime::CurrentDateAndTime() + ".txt");
std::function<void(HString,  float, float, float, HString)> ConsoleDebug::printFuncAdd = [](HString, float, float, float, HString) {};

FILE* log_file = NULL;
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
void ReadConsoleMsgThreadFunc()
{
    while (true)
    {
        {
            _Sleep(1);
            ConsoleDebug::ReadMsgFromConsole();
        }
    }
}

void  SocketAcceptThreadFunc()
{
    while (true)
    {
        _Sleep(500);
        sockaddr_in addrClient;              
#ifdef _WIN32
        int addrClientlen = sizeof(addrClient);
        ConsoleDebug::consoleSockets.push_back(accept(ConsoleDebug::tcpSocket, (sockaddr FAR*) & addrClient, &addrClientlen));
#else
        socklen_t addrClientlen = sizeof(addrClient);
        ConsoleDebug::consoleSockets.push_back(accept(ConsoleDebug::tcpSocket, reinterpret_cast<sockaddr*>(&addrClient), &addrClientlen));
#endif 

    }
}

void WriteToLogFile(FILE* &log_file , HString log)
{
    #ifdef _WIN32
        auto err_src_name = fopen_s(&log_file, LogFileName.c_str(), "a+");
    #else
        log_file = fopen(LogFileName.c_str(), "a+");
    #endif
    if (log_file)
    {
        fwrite(log.c_str(), strlen(log.c_str()), 1, log_file);
        fclose(log_file);
        log_file = NULL;
    }
}

void ConsoleDebug::CreateConsole(HString consolePath ,bool bNoClient)
{
    if (ConsoleDebug::bConnectedConsole == true && ConsoleDebug::bConnectedFailed == false)
        return;
    ConsoleDebug::bConnectedFailed = false;
#if _WIN32
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
#endif
    tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in daddr;
    std::memset((void*)&daddr, 0, sizeof(daddr));
    daddr.sin_family = AF_INET;
#ifdef _WIN32
    daddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
    daddr.sin_addr.s_addr = INADDR_ANY;
#endif
    daddr.sin_port = htons(5050);

    auto retVal = ::bind(tcpSocket, (struct sockaddr*)&daddr, sizeof(daddr));
#if _WIN32
    if (SOCKET_ERROR == retVal)
#else
    if (retVal == -1)
#endif
    {
        printf("bind failed!\n");
        CloseSocket(tcpSocket);
        WSACleanup();
        return;
    }


    retVal = listen(tcpSocket, 1);
#if _WIN32
    if (SOCKET_ERROR == retVal)
#else
    if (retVal == -1)
#endif
    {
        printf("listen failed!\n");
        CloseSocket(tcpSocket);
        WSACleanup();
        return;
    }
#if _WIN32
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
#endif
    //接收来自客户端的请求
    printf("TCPServer start...\n");
    socketAcceptThread = std::thread(SocketAcceptThreadFunc);
    socketAcceptThread.detach();

    ConsoleDebug::bConnectedConsole = true;
    _Sleep(25);
    ConsoleDebug::print_endl("Connect Console Succeed!", "0,255,0");

    ReadMessageThread = std::thread(ReadConsoleMsgThreadFunc);
    ReadMessageThread.detach();

    //Create Log file
    LogFileName = (HString(".\\log_") + HTime::CurrentDateAndTime() + ".txt");
    WriteToLogFile(log_file,"");
}

void ConsoleDebug::CleanupConsole()
{
    CloseSocket(tcpSocket);
    WSACleanup();
    bConnectedConsole = false;
#ifdef _WIN32
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
    //CloseHandle(ReadMessageThread);
    //CloseHandle(socketAcceptThread);

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
#ifdef _WIN32
char* UnicodeToUtf8(const wchar_t* unicode)
{

        int len;
        len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
        char* szUtf8 = (char*)malloc(len + 1);
        memset(szUtf8, 0, len + 1);
        WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
    return szUtf8;
}
#endif
void ConsoleDebug::print(HString in, HString color, HString background, HString type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.Length() > 0 )
    {
        HString Data = HTime::CurrentDateAndTimeH();

        std::vector<HString> colorArray = color.Split(",");
        float r, g, b;
        r = (float)HString::ToDouble(colorArray[0]);
        g = (float)HString::ToDouble(colorArray[1]);
        b = (float)HString::ToDouble(colorArray[2]);
        #ifdef _WIN32
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
        #endif
        Data = "[" + Data + "]";
        HString nIn = Data + in;
        printf(nIn.c_str());
        #ifdef _WIN32
                OutputDebugStringA(nIn.c_str());
        #endif
        printFuncAdd(nIn, r / 255.0f, g / 255, b / 255, Data);

        WriteToLogFile(log_file, nIn);

        if (bConnectedConsole == true)
        {
            HString out = nIn;
            //memset(buffer, 0, sizeof(buffer));
            out += "||//||" + color + "||//||" + background + "||//||" + Data;
            #ifdef _WIN32
                char* buffer = UnicodeToUtf8(out.c_wstr());
            #else
                auto buffer = std::string(out.c_str()).c_str();
            #endif
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
            #ifdef _WIN32
                free(buffer);
            #endif
        }
    }
}

void ConsoleDebug::print_endl(HString in, HString color, HString background, HString type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.Length() > 0 )
    {
        HString Data = HTime::CurrentDateAndTimeH();

        std::vector<HString> colorArray = color.Split(",");
        float r, g, b;
        r = (float)HString::ToDouble(colorArray[0]);
        g = (float)HString::ToDouble(colorArray[1]);
        b = (float)HString::ToDouble(colorArray[2]);
        #ifdef _WIN32
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
        #endif
        Data = "[" + Data + "] ";
        printf((Data+in).c_str());
        printf("\n");
        #ifdef _WIN32
        OutputDebugStringA(((Data + in) +"\n").c_str());
        #endif
        HString nIn = (Data + in) + "\n";

        printFuncAdd(nIn, r / 255.0f, g / 255, b / 255, Data);

        WriteToLogFile(log_file, nIn);

        if (bConnectedConsole == true)
        {
            HString out = (Data + in);
            //memset(buffer, 0, sizeof(buffer));
            out += "\n||//||" + color + "||//||" + background + "||//||" + Data;
            #ifdef _WIN32
                char* buffer = UnicodeToUtf8(out.c_wstr());
            #else
                auto buffer = std::string(out.c_str()).c_str();
            #endif
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
            #ifdef _WIN32
                free(buffer);
            #endif
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