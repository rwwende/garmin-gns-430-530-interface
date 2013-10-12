#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "comhooks.h"
#include "log.h"


#define COM_ACTIVE_REGISTER (0x7f0)
#define COM_STANDBY_REGISTER (0x7f4)
#define COM_UNKNOWN_REGISTER (0x7e8) //size 1

static unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    return ComHooks::instanace()->TSK_pvg_send_msg_ex(p1, p2);
}

static unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return ComHooks::instanace()->reg_read(num, addr, size, p4);
}

static unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return ComHooks::instanace()->reg_write(num, addr, size, p4);
}

static unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    return ComHooks::instanace()->SYS_pvg_var_ctrl(p1, p2);
}






ComHooks* ComHooks::m_gInstance = NULL;

ComHooks::ComHooks()
{

    InitializeCriticalSection(&m_cs);

    m_activeComVal = 0xffffffff;
    m_standbyComVal = 0xffffffff;

    m_activeComSet = 0;
    m_standbyComSet = 0;


}

ComHooks::~ComHooks()
{
    DeleteCriticalSection(&m_cs);

}

ComHooks* ComHooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new ComHooks();
    }

    return m_gInstance;


}

void  ComHooks::lock()
{
    EnterCriticalSection(&m_cs);
}

void ComHooks::unlock()
{
    LeaveCriticalSection(&m_cs);
}


bool ComHooks::hook(SharedStruct<GNSIntf>*  pShared)
{

    bool res = true;
    int i;


    m_pShared = pShared;
    m_pData = pShared->get();

    HMODULE h_cdp_com_box_sim = GetModuleHandle("cdp_com_box_sim.dll");

    SDLLHook krnlsimHook = 
    {
        "krnlsim.dll",
        false, NULL, // Default hook disabled, NULL function pointer.
        {
            { "TSK_pvg_send_msg_ex", my_TSK_pvg_send_msg_ex},
            { "reg_read", my_reg_read},
            { "reg_write", my_reg_write},
            { "SYS_pvg_var_ctrl" , my_SYS_pvg_var_ctrl},
            { NULL, NULL }
        }
    };



    i=0;
    HookAPICallsMod(&krnlsimHook, h_cdp_com_box_sim);
    m_TSK_pvg_send_msg_ex_fn = (TSK_pvg_send_msg_ex_t)krnlsimHook.Functions[i++].OrigFn;
    m_reg_read_fn = (reg_read_t)krnlsimHook.Functions[i++].OrigFn;
    m_reg_write_fn = (reg_write_t)krnlsimHook.Functions[i++].OrigFn;
    m_SYS_pvg_var_ctrl_fn = (SYS_pvg_var_ctrl_t)krnlsimHook.Functions[i++].OrigFn;



    return res;
}



//COM functions
void  ComHooks::setActiveFrequency(unsigned long freq)
{
    lock();

    m_activeComSet = freq;

    //logMessageEx("--- ComHooks::setActiveFrequency %d", freq);

    unlock();
}

void  ComHooks::setStandbyFrequency(unsigned long freq)
{
    lock();

    m_standbyComSet = freq;

    //logMessageEx("--- ComHooks::setActiveFrequency %d", freq);

    unlock();
}


unsigned long ComHooks::TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res =  m_TSK_pvg_send_msg_ex_fn(p1, p2);

    //logMessageEx("--- ComHooks::TSK_pvg_send_msg_ex %08x, %08x -> %08x", p1, p2, res);

    unlock();

    return res;
}



unsigned long ComHooks::reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{

    unsigned long res=0;

    lock();

    //Check is a frequency must be set
    if(0 != m_activeComSet)
    {
        m_reg_write_fn(COM_ACTIVE_REGISTER, &m_activeComSet, sizeof(m_activeComSet), 0);
        m_activeComVal = m_activeComSet;
        m_activeComSet = 0;
    }

    if(0 != m_standbyComSet)
    {
        m_reg_write_fn(COM_STANDBY_REGISTER, &m_standbyComSet, sizeof(m_standbyComSet), 0);
        m_standbyComVal = m_standbyComSet;
        m_standbyComSet = 0;
    }



    res = m_reg_read_fn(num, addr, size, p4);

    

    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- ComHooks::reg_read %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);

    checkFrequencies();

    unlock();

    return res;

}

unsigned long ComHooks::reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;

    lock();

    res = m_reg_write_fn(num, addr, size, p4);


    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- ComHooks::reg_write %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);

    checkFrequencies();

    unlock();

    return res;

}

unsigned long ComHooks::SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;
    
    lock();

    res = m_SYS_pvg_var_ctrl_fn(p1, p2);

    //logMessageEx("--- ComHooks::SYS_pvg_var_ctrl %08x, %08x", p1, p2);
    
    checkFrequencies();

    unlock();

    return res;
}


void ComHooks::checkFrequencies()
{
    unsigned long val;
    
    if( 0 == m_reg_read_fn(COM_ACTIVE_REGISTER, &val, sizeof(val), 0))
    {
        if(val != m_activeComVal)
        {
            //logMessageEx("----- COM_ACTIVE_REGISTER %d", val);

            m_activeComVal = val;
            FreqInfo msg;
            msg.msgType = MGS_COM_ACTIVE;
            msg.freq = val;

            Hooks::instanace()->notifyFreqencyChange(&msg);
        }

    }

    if( 0 == m_reg_read_fn(COM_STANDBY_REGISTER, &val, sizeof(val), 0))
    {
        if(val != m_standbyComVal)
        {
            //logMessageEx("----- COM_STANDBY_REGISTER %d", val);

            m_standbyComVal = val;
            FreqInfo msg;
            msg.msgType = MGS_COM_STANDBY;
            msg.freq = val;

            Hooks::instanace()->notifyFreqencyChange(&msg);

        }
    }

}