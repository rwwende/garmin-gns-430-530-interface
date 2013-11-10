#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "krnlsimhooks.h"
#include "log.h"



static unsigned long __cdecl my_SIM_proc_rqst(unsigned long p1)
{
    return KrlnsimHooks::instanace()->SIM_proc_rqst(p1);
}



KrlnsimHooks* KrlnsimHooks::m_gInstance = NULL;

KrlnsimHooks::KrlnsimHooks()
{

    InitializeCriticalSection(&m_cs);

}

KrlnsimHooks::~KrlnsimHooks()
{
    DeleteCriticalSection(&m_cs);

}

KrlnsimHooks* KrlnsimHooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new KrlnsimHooks();
    }

    return m_gInstance;


}

void  KrlnsimHooks::lock()
{
    EnterCriticalSection(&m_cs);
}

void KrlnsimHooks::unlock()
{
    LeaveCriticalSection(&m_cs);
}


bool KrlnsimHooks::hook(SharedStruct<GNSIntf>*  pShared)
{

    bool res = true;
    int i;


    m_pShared = pShared;
    m_pData = pShared->get();

    HMODULE krnlsim = GetModuleHandle("krnlsim.dll");


    SDLLHook G530SIMHook = 
    {
        "G530SIM.exe",
        false, DefaultHook, // Default hook disabled, NULL function pointer.
        {
            { "SIM_proc_rqst", my_SIM_proc_rqst},
            { NULL, NULL }
        }
    };
    HookAPICallsMod(&G530SIMHook, krnlsim);
    i=0;
    m_SIM_proc_rqst_fn = (SIM_proc_rqst_t)G530SIMHook.Functions[i++].OrigFn;


    return res;
}




unsigned long KrlnsimHooks::SIM_proc_rqst(unsigned long p1)
{
    unsigned long res=0;

    //lock();

    logMessageEx("--- KrlnsimHooks::SIM_proc_rqst IN %08x", p1);
    logAdjustIndent(2);

    res = m_SIM_proc_rqst_fn(p1);

    logAdjustIndent(-2);
    logMessageEx("--- KrlnsimHooks::SIM_proc_rqst OUT -> %d", res);

    //unlock();

    return res;
}

