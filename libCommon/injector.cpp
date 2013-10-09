#include <windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <stdio.h>
#pragma comment(lib, "Shlwapi.lib")

#include "injector.h"
#include "log.h"

 
#define BYTE_TYPE(x) __asm _emit x 
#define WORD_TYPE(x) BYTE_TYPE((x>>(0*8))&0xFF)    BYTE_TYPE((x>>(1*8))&0xFF)
#define DWORD_TYPE(x) BYTE_TYPE((x>>(0*8))&0xFF) BYTE_TYPE((x>>(1*8))&0xFF) BYTE_TYPE((x>>(2*8))&0xFF) BYTE_TYPE((x>>(3*8))&0xFF)

#define LDR_DATA_START 0xAFAFAFAF
#define LDR_CODE_END 0xFAFAFAFA

HANDLE ghProcess = NULL;
HANDLE ghThread = NULL;
bool gbInjecting = false;


PVOID SearchDWORD(PVOID Start, DWORD dwSearch)
{
    register PVOID pAddr = Start;

    while(*(PDWORD)pAddr != dwSearch)
        ((PBYTE&)pAddr)++;

    return pAddr;
}

// the actual loader code
void __declspec(naked) loader(void)
{
    __asm
    {
        // return address placeholder
        push 0xFFFFFFFF

        // save context
        pushfd
        pushad

        // get variables
        call temp
temp:    
        pop ebp
        sub ebp, offset temp
        mov eax, dword ptr[ebp + pLoadLibrary]
        mov ebx, dword ptr[ebp + pszDllPath]

        // call loadlibrary
        push ebx
        call eax

        // store return value of loadlibrary
        mov dword ptr[ebp + pRetVal], eax

        // get return address
        mov eax,  dword ptr[ebp + pReturnAddr]

        // move return address to placeholder
        mov dword ptr[esp + 0x24], eax

        // restore context
        popad
        popfd

        // leave cave
        ret

        // needed variables (first is also data marker)
        pReturnAddr: DWORD_TYPE(LDR_DATA_START)
        pLoadLibrary: DWORD_TYPE(0xFFFFFFFF)
        pszDllPath: DWORD_TYPE(0xFFFFFFFF)
        pRetVal: DWORD_TYPE(0xFFFFFFFF)

         // end marker
         DWORD_TYPE(LDR_CODE_END)
    }
}

// returns true if everything went fine
// if the function succeeds pdwReturn is the return value of LoadLibraryA in the remote process
BOOL CodeCaveInjectDll(IN char* szDllPath, /*IN DWORD dwProcessId,*/ HANDLE hProcess, /*IN DWORD dwThreadId,*/ HANDLE hThread, OUT PDWORD pdwReturn)
{
    BOOL bRet = FALSE;

    SIZE_T DllPath_size = sizeof(char) * (_tcslen(szDllPath) + 1);

    // get size of loader code
    DWORD dwSizeLoader = (DWORD)SearchDWORD(loader, LDR_CODE_END) - (DWORD)loader;

    //gpLogger->logMessageEx("dwSizeLoader=%d", dwSizeLoader);

    // allocate local buffer for loader code
    PVOID pLdrCode = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSizeLoader);
    
    //gpLogger->logMessageEx("pLdrCode=%08x", pLdrCode);
    
    memcpy(pLdrCode, loader, dwSizeLoader);

    // get pointer to first data variable (pReturnAddr)
    PDWORD pData = (PDWORD)SearchDWORD(pLdrCode, LDR_DATA_START);
    
    //gpLogger->logMessageEx("pData=%08x", pData);

    HMODULE hmod = GetModuleHandle("kernel32.dll");
    
#ifdef UNICODE    
    DWORD LLA = (DWORD)GetProcAddress(hmod, "LoadLibraryW");
#else
    DWORD LLA = (DWORD)GetProcAddress(hmod, "LoadLibraryA");
#endif
    // fill pLoadLibrary data
    pData[1] = (DWORD)LLA;

    // open target process
    //if (HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION
    //    | PROCESS_VM_READ
    //    | PROCESS_VM_WRITE, FALSE, dwProcessId))
    {
        // alloc space for dll path
        PVOID pszDllPath = VirtualAllocEx(hProcess, NULL, DllPath_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        //PVOID pszDllPath = VirtualAllocEx(hProcess, NULL, _tcsclen(szDllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        
        
        //gpLogger->logMessageEx("pszDllPath=%08x", pszDllPath);
        
        if (pszDllPath)
        {
            // write dll path
            BOOL wpr = WriteProcessMemory(hProcess, pszDllPath, szDllPath, DllPath_size, NULL);
            //gpLogger->logMessageEx("wpr=%d", wpr);
            if (wpr)
            {
                // fill loader code pszDllPath data
                pData[2] = (DWORD)pszDllPath;

                // open main thread
                //if (HANDLE hThread  = OpenThread(THREAD_SUSPEND_RESUME
                //    | THREAD_GET_CONTEXT
                //    | THREAD_SET_CONTEXT, FALSE, dwThreadId))
                {
                    // suspend main thread
                    //???if (SuspendThread(hThread) != 0xFFFFFFFF)
                    {
                        // get main thread context
                        CONTEXT ctx = {CONTEXT_CONTROL};
                        if (GetThreadContext(hThread, &ctx))
                        {
                            // fill loader code pReturnAddr data, all needed data is collected now
                            pData[0]   = ctx.Eip;
                            
                            
                            PVOID pRemoteLdr = VirtualAllocEx(hProcess, NULL, dwSizeLoader, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                            //gpLogger->logMessageEx("pRemoteLdr=%08x", pRemoteLdr);
                            if (pRemoteLdr)
                            {
                                // write loader code
                                if (WriteProcessMemory(hProcess, pRemoteLdr, pLdrCode, dwSizeLoader, NULL))
                                {
                                    // free loader code buffer
                                    HeapFree(GetProcessHeap(), NULL, (PVOID)pLdrCode);

                                    // set eip to first instruction of loader code
                                    ctx.Eip = (DWORD)pRemoteLdr;

                                    // set modified thread context to target process
                                    if (SetThreadContext(hThread, &ctx))
                                    {
                                        // resume execution, let the loader run
                                        if (ResumeThread(hThread) != 0xFFFFFFFF)
                                        {
                                            // everything went fine
                                            bRet = TRUE;

                                            // check if LoadLibraryA in loader code already returned
                                            DWORD dwRetValue = 0;
                                            do 
                                            {
                                                //Sleep(125);
                                                Sleep(500);
                                                // get pRetVal of our allocated loader code in target process
                                                //BOOL rpm = 
                                                ReadProcessMemory(hProcess, (PVOID)((PBYTE)pRemoteLdr + (dwSizeLoader - 4)), &dwRetValue, 4, NULL);
                                                // check for default value
                                                //gpLogger->logMessageEx("Checking for check for default value rpm=%d dwRetValue=%08x", rpm, dwRetValue);
                                                logMessageEx("--- Waiting for process to start");
                                            } while (dwRetValue == 0xFFFFFFFF);

                                            *pdwReturn = dwRetValue;
                                        }
                                    }
                                }
                                VirtualFreeEx(hProcess, pRemoteLdr, 0, MEM_RELEASE);
                            } 
                        }
                        if (!bRet)
                        {
                            ResumeThread(hThread);
                        }
                    }         
                    //CloseHandle(hThread);
                }
            }
            VirtualFreeEx(hProcess, pszDllPath, 0, MEM_RELEASE);
        }
        //CloseHandle(hProcess);
    }

    return bRet;
}

// gets the main thread id from TIB by processid 
// returns 0 when failed
DWORD GetMainThreadId(IN DWORD dwProcessId)
{
    DWORD dwRet = 0;

    if (HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwProcessId))
    {

        PVOID pThreadId = (PVOID)((DWORD)__readfsdword(0x18) + 0x24);

        DWORD dwThreadId = 0;

        if (ReadProcessMemory(hProcess, pThreadId, &dwThreadId, 4, NULL))
            dwRet = dwThreadId;

        CloseHandle(hProcess);
    }

    return dwRet;
}


// checks wheter a module is loaded identified by it's full filepath or filename
// if bFullPath is FALSE and modulename is not unique func will return (DWORD)-1
// if a module with given modulename / path was not found in target process func will return 0 otherwise module handle
// lowercase / uppercase doesn't matter
DWORD GetModule(IN DWORD dwProcessId, IN char* szModuleNameOrPath, IN BOOL bFullPath)
{
    DWORD dwRet = 0;
    DWORD dwCount = 0;

    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

    if (hSnapShot != INVALID_HANDLE_VALUE) 
    {
        MODULEENTRY32 me = {0};
        me.dwSize = sizeof(MODULEENTRY32);

        BOOL bRet = Module32First(hSnapShot, &me);

        while(bRet) 
        {
            //PCHAR szCompare = me.szExePath;
            //USES_CONVERSION;
            char* szCompare = me.szExePath;
            //CHAR szCompare[260];// = me.szExePath;
            
            //strcpy(szCompare, T2A(me.szExePath));
            

            if (!bFullPath)
            {
                //szCompare = PathFindFileNameA(me.szExePath);
                szCompare = PathFindFileName(me.szExePath);
            }
            
            if (!_tcsicmp(szCompare, szModuleNameOrPath))
            {            
                dwRet = (DWORD)me.hModule;

                if (bFullPath)
                    break;
                else
                    dwCount++;
            }
            bRet = Module32Next(hSnapShot, &me);
        }

        if (!bFullPath && dwCount > 1)
            dwRet = 0xFFFFFFFF;

        CloseHandle(hSnapShot);
    }

    return dwRet;
}


int startAndInject(char* pExeName, char* pExePath, char* pLibFile)
{
    BOOL bFullPath = FALSE;

    gbInjecting = true;

    //
    /* CreateProcess API initialization */
    STARTUPINFO siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo); 
    siStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    siStartupInfo.wShowWindow = SW_HIDE;
    
    
    CreateProcess(NULL, pExeName,
                             0, 0, false,
                            CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, 
                            NULL, 
                            pExePath,
                            &siStartupInfo, 
                            &piProcessInfo
                    );
               
    if (DWORD dwProcessId = piProcessInfo.dwProcessId)    
    {
        DWORD dwModule = 0x0;

        if (!dwModule && dwModule != 0xFFFFFFFF)
        {
            DWORD dwRetValue = 0xFFFFFFFF;
            
            if (CodeCaveInjectDll(pLibFile,  piProcessInfo.hProcess,  piProcessInfo.hThread, &dwRetValue))
            {
                dwModule = GetModule(dwProcessId, pLibFile, TRUE);
                
                logMessageEx("--- Library loaded");
                ghProcess = piProcessInfo.hProcess;
                ghThread = piProcessInfo.hThread;
            }

        }
        else if (dwModule)
        {
            logMessageEx("--- Library is already loaded");
        }
        else if (!bFullPath && dwModule == 0xFFFFFFFF)
        {
            logMessageEx("--- Library is not unique");
        }    
    }
    else
    {
        logMessageEx("--- Process %s not found", pExeName);
    }

    logMessageEx("--- Waiting for process to exit");


    //WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
    /* Release handles */
    //CloseHandle(piProcessInfo.hProcess);
    //CloseHandle(piProcessInfo.hThread);

    gbInjecting = false;

    return 0;
}   

//0 , terminated, -1 not terminated
int checkProcessTerminated()
{
    int res = -1;
    
    if(!gbInjecting)
    {
        if(NULL != ghProcess)
        {
            if(WAIT_OBJECT_0 == WaitForSingleObject(ghProcess, 0))
            {
                /* Release handles */
                CloseHandle(ghProcess);
                CloseHandle(ghThread);
                
                ghProcess = NULL;
                ghThread  = NULL;
                
                res = 0;
                
            }
        }else
        {
            res = -1;
        }
        
      
    }
        
    return res;
}
