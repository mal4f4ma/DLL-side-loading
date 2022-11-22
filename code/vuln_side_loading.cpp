// vuln_side_loading.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>

typedef int(__cdecl *MYPROC)(LPCWSTR);

int main(int argc, char *argv[])
{
    std::cout << "Esta app es vulnerable a DLL side-loading, ejecutala con cuidado!\n";
    if (argc != 2)
    {
        std::cout << "Usage: app.exe opcion\nOpciones:\n1. Cargar DLL por nombre\n2. Cargar DLL con path completo";
        return 0;
    }
    int k = atoi(argv[1]);
    HINSTANCE hinstLib;
    if (k == 1)
    {
        hinstLib = LoadLibrary(L"my_custom.dll");
    }
    else if (k == 2)
    {
        hinstLib = LoadLibrary(L"C:\\Windows\\System32\\my_custom.dll");
    }
    else
    {
        return 0;
    }
    if (hinstLib)
    {
        std::cout << "Se cargo correctamente la DLL\n";
        FreeLibrary(hinstLib);
    }
    else
    {
        std::cout << "Error al cargar la DLL " << argv[1] << "\n";
    }
    return 0;
}