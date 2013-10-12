// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>
#include <stdio.h>
#include "log.h"
#include "hooks.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    char moduleName[MAX_PATH];
    char logFileName[MAX_PATH];

	switch (ul_reason_for_call)
	{
	    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls( hModule );


            GetModuleFileNameA(GetModuleHandle(0), moduleName, sizeof(moduleName));
            sprintf(logFileName, "%s.log", moduleName);
            logInit(logFileName);

            //Hook to GNSx30.exe
            Hooks::instanace()->hookGnsx30(hModule);

            break;
        }
	    case DLL_THREAD_ATTACH:
	    case DLL_THREAD_DETACH:
	    case DLL_PROCESS_DETACH:
		    break;
	}
	return TRUE;
}

