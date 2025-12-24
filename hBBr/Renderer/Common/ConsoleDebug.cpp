#include <ctime>
#include "ConsoleDebug.h"
#include "HTime.h"
#include <mutex>
#include "FileSystem.h"
#include <SDL3/SDL.h>
#include "StringTools.h"
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
std::string LogFileName = "";
FILE* log_file = nullptr;
std::mutex g_num_mutex;

std::string rgb2hex(int r, int g, int b, bool with_head)
{
    std::stringstream ss;
    if (with_head)
        ss << "##";
    ss << std::hex << (r << 16 | g << 8 | b);
    return std::string(ss.str().c_str());
}

std::map<std::string, std::function<void()>> ConsoleDebug::commandLists;
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

void WriteToLogFile(FILE* &log_file , std::string log)
{
    if(LogFileName.size() <= 2)
    {
        LogFileName = FileSystem::GetProgramPath() + (std::string("log_") + HTime::CurrentDateAndTime() + ".txt");
    }
    #ifdef _WIN32
        auto cstr = LogFileName;
        auto err_src_name = fopen_s(&log_file, cstr.c_str(), "a+");
    #else
        log_file = fopen(LogFileName.c_str(), "a+");
    #endif
    if (log_file)
    {
        auto log_cstr = log;
        fwrite(log_cstr.c_str(), strlen(log_cstr.c_str()), 1, log_file);
        fclose(log_file);
        log_file = nullptr;
    }
}

void ConsoleDebug::CreateConsole(std::string consolePath ,bool bNoClient)
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
        printf("CreateConsole : WSAStartup 错误");
        WSACleanup();
        return;
    }
    else
    {

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
        //HINSTANCE hi = ShellExecute(hwnd, L"open", consolePath.c_wstr(), nullptr, nullptr, SW_SHOWNORMAL);
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        CreateProcess(nullptr,   // No module name (use command line)
            (TCHAR*)consolePath.c_str(),        // Command line
            nullptr,           // Process handle not inheritable
            nullptr,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            nullptr,           // Use parent's environment block
            nullptr,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi);           // Pointer to PROCESS_INFORMATION structure
    }
#endif
    printf("TCPServer start...\n");
    socketAcceptThread = std::thread(SocketAcceptThreadFunc);
    socketAcceptThread.detach();

    ConsoleDebug::bConnectedConsole = true;
    _Sleep(25);
    ConsoleDebug::print_endl("Connect Console Succeed!", "0,255,0");

    ReadMessageThread = std::thread(ReadConsoleMsgThreadFunc);
    ReadMessageThread.detach();

    //Create Log file
    LogFileName = FileSystem::GetProgramPath() + (std::string("log_") + HTime::CurrentDateAndTime() + ".txt");
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
        log_file = nullptr;
    }
}

void ConsoleDebug::ReadMsgFromConsole()
{
    char  buffer[4096];
    memset(buffer, '\0', sizeof(buffer));
    for (int i = 0; i < consoleSockets.size(); i++) {
        int rec = recv(consoleSockets[i], reinterpret_cast<char*>(buffer), 4096, 0);
        buffer[4095] = '\0';
        if (rec > 0)
        {
            //MessageBoxA(0, buffer, 0, 0);
            execCommand(buffer);
        }
    }
}

void ConsoleDebug::print(std::string in, std::string color, std::string background, std::string type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.size() > 0 )
    {
        std::string Data = HTime::CurrentDateAndTimeH();

        std::vector<std::string> colorArray = StringTool::split(color, ",");
        float r, g, b;
        r = (float)StringTool::ToDouble(colorArray[0]);
        g = (float)StringTool::ToDouble(colorArray[1]);
        b = (float)StringTool::ToDouble(colorArray[2]);
        #ifdef _WIN32
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); 
                WORD wr2 = 0 ;
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
        std::string nIn = Data + in;
        if (r == 255 && g < 100 && b < 100)
        {
            SDL_LogError(0, "%s", nIn.c_str());
        }
        else if (r > 200 && g > 200 && b < 100)
        {
            SDL_LogWarn(0, "%s", nIn.c_str());
        }
        else
        {
            SDL_Log("%s", nIn.c_str());
        }
        WriteToLogFile(log_file, nIn);

        if (bConnectedConsole == true)
        {
            std::string out = nIn;
            //memset(buffer, 0, sizeof(buffer));
            out += "||//||" + color + "||//||" + background + "||//||" + Data;

            //#ifdef _WIN32
            //    char* buffer = UnicodeToUtf8(out.c_str());
            //#else
            //    auto buf_temp = std::string(out.c_str());
            //    auto buffer = buf_temp.c_str();
            //#endif

            for (int i = 0; i < consoleSockets.size(); i++) 
            {
                if (send(consoleSockets[i], (char*)(out.c_str()), 4096, 0) < 0)
                {
#ifdef _DEBUG
                    std::string cs = "消息发送失败...";
                    cs += StringTool::FromUInt(::GetLastError());
                    printf(cs.c_str());
                    printf("\n");
#endif
                }
            }
            #ifdef _WIN32
                //free(buffer);
            #endif
        }
    }
}

void ConsoleDebug::print_endl(std::string in, std::string color, std::string background, std::string type)
{
    std::lock_guard<std::mutex> lock(g_num_mutex);
    if (in.length() > 0 )
    {
        std::string Data = HTime::CurrentDateAndTimeH();

        std::vector<std::string> colorArray = StringTool::split(color, ",");
        float r, g, b;
        r = (float)StringTool::ToDouble(colorArray[0]);
        g = (float)StringTool::ToDouble(colorArray[1]);
        b = (float)StringTool::ToDouble(colorArray[2]);
        #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD wr2 = 0;
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

        //if (r == 255 && g < 100 && b < 100)
        //{
        //    SDL_LogError(0, "%s", ((Data + in)).c_str());
        //}
        //else if (r > 200 && g > 200 && b < 100)
        //{
        //    SDL_LogWarn(0, "%s", ((Data + in)).c_str());
        //}
        //else
        //{
        //    SDL_Log("%s", ((Data + in)).c_str());
        //}

        printf("%s\n", ((Data + in)).c_str());
        std::string nIn = (Data + in) + "\n";
        
#ifdef _WIN32
		OutputDebugStringA(nIn.c_str());
#endif

        WriteToLogFile(log_file, nIn);

        if (bConnectedConsole == true)
        {
            std::string out = (Data + in);
            //memset(buffer, 0, sizeof(buffer));
            out += "\n||//||" + color + "||//||" + background + "||//||" + Data;
            //#ifdef _WIN32
            //    char* buffer = UnicodeToUtf8(out.c_str());
            //#else
            //    auto buf_temp = std::string(out.c_str());
            //    auto buffer = buf_temp.c_str();
            //#endif

            for (int i = 0; i < consoleSockets.size(); i++)
            {
                if (send(consoleSockets[i], (char*)(out.c_str()), 4096, 0) < 0)
                {
#ifdef _DEBUG
                    std::string cs = "消息发送失败...";
                    cs += StringTool::FromUInt(::GetLastError());
                    printf(cs.c_str());
                    printf("\n");
#endif
                }
            }
            #ifdef _WIN32
                //free(buffer);
            #endif
        }
    }
}

void ConsoleDebug::AddNewCommand(std::string newCommand, std::function<void()> func, int ParamerterCount, ...)
{
    std::map<std::string, std::function<void()>>::iterator it;
    for (it = commandLists.begin(); it != commandLists.end(); it++)
    {
        //if (it->first.IsSame(newCommand, false))
        if (StringTool::IsEqual(it->first, newCommand, false))
        {
            print_endl("Failed to add command [" + newCommand + "].This command has been existed.", "255,255,10");
            return;
        }
    }

    commandLists.insert(std::pair<std::string, std::function<void()>>(newCommand, func));
}

void ConsoleDebug::execCommand(std::string key)
{

}