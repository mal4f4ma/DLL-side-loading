// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL, TEXT("Proccess Attach"), TEXT("BugCon - 2022"), MB_ICONERROR | MB_OK);
        break;
    case DLL_THREAD_ATTACH:
        MessageBox(NULL, TEXT("Thread Attach"), TEXT("BugCon - 2022"), MB_ICONERROR | MB_OK);
        break;
    case DLL_THREAD_DETACH:
        MessageBox(NULL, TEXT("Thread detach"), TEXT("BugCon - 2022"), MB_ICONERROR | MB_OK);
        break;
    case DLL_PROCESS_DETACH:
        MessageBox(NULL, TEXT("Proccess detach"), TEXT("BugCon - 2022"), MB_ICONERROR | MB_OK);
        break;
    }
    return TRUE;
}
