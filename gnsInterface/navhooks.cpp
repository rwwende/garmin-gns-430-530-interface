#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "navhooks.h"
#include "log.h"


#define NAV_ACTIVE_REGISTER (0x7f2)
#define NAV_STANDBY_REGISTER (0x7f6)
//#define NAV_UNKNOWN_REGISTER (0x7e8) //size 1


static unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    return NavHooks::instanace()->TSK_pvg_send_msg_ex(p1, p2);
}

static unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return NavHooks::instanace()->reg_read(num, addr, size, p4);
}

static unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return NavHooks::instanace()->reg_write(num, addr, size, p4);
}

static unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    return NavHooks::instanace()->SYS_pvg_var_ctrl(p1, p2);
}






NavHooks* NavHooks::m_gInstance = NULL;

NavHooks::NavHooks()
{

    InitializeCriticalSection(&m_cs);

    m_activeNavVal = 0xffffffff;
    m_standbyNavVal = 0xffffffff;

    m_activeNavSet = 0;
    m_standbyNavSet = 0;


}

NavHooks::~NavHooks()
{
    DeleteCriticalSection(&m_cs);

}

NavHooks* NavHooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new NavHooks();
    }

    return m_gInstance;


}

void  NavHooks::lock()
{
    EnterCriticalSection(&m_cs);
}

void NavHooks::unlock()
{
    LeaveCriticalSection(&m_cs);
}


bool NavHooks::hook(SharedStruct<GNSIntf>*  pShared)
{

    bool res = true;
    int i;


    m_pShared = pShared;
    m_pData = pShared->get();

    HMODULE h_cdp_com_box_sim = GetModuleHandle("cdp_vloc_box_sim.dll");

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
void  NavHooks::setActiveFrequency(unsigned long freq)
{
    lock();

    m_activeNavSet = freq;

    //logMessageEx("--- NavHooks::setActiveFrequency %d", freq);

    unlock();
}

void  NavHooks::setStandbyFrequency(unsigned long freq)
{
    lock();

    m_standbyNavSet = freq;

    //logMessageEx("--- NavHooks::setActiveFrequency %d", freq);

    unlock();
}

unsigned long NavHooks::TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res =  m_TSK_pvg_send_msg_ex_fn(p1, p2);

    //logMessageEx("--- NavHooks::TSK_pvg_send_msg_ex %08x, %08x -> %08x", p1, p2, res);

    unlock();

    return res;
}


unsigned long NavHooks::reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{

    unsigned long res=0;

    lock();

    //Check is a frequency must be set
    if(0 != m_activeNavSet)
    {
        m_reg_write_fn(NAV_ACTIVE_REGISTER, &m_activeNavSet, sizeof(m_activeNavSet), 0);
        m_activeNavVal = m_activeNavSet;
        m_activeNavSet = 0;
    }

    if(0 != m_standbyNavSet)
    {
        m_reg_write_fn(NAV_STANDBY_REGISTER, &m_standbyNavSet, sizeof(m_standbyNavSet), 0);
        m_standbyNavVal = m_standbyNavSet;
        m_standbyNavSet = 0;
    }



    res = m_reg_read_fn(num, addr, size, p4);



    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- NavHooks::reg_read %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);


    checkFrequencies();

    unlock();

    return res;

}

unsigned long NavHooks::reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;

    lock();

    res = m_reg_write_fn(num, addr, size, p4);


    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- NavHooks::reg_write %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);

    checkFrequencies();

    unlock();

    return res;

}

unsigned long NavHooks::SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res = m_SYS_pvg_var_ctrl_fn(p1, p2);

    //logMessageEx("--- NavHooks::SYS_pvg_var_ctrl %08x, %08x", p1, p2);

    checkFrequencies();

    unlock();

    return res;
}


void NavHooks::checkFrequencies()
{
    unsigned long val;

    if( 0 == m_reg_read_fn(NAV_ACTIVE_REGISTER, &val, sizeof(val), 0))
    {
        if(val != m_activeNavVal)
        {
            //logMessageEx("----- NAV_ACTIVE_REGISTER %d", val);

            m_activeNavVal = val;
            FreqInfo msg;
            msg.msgType = MGS_NAV_ACTIVE;
            msg.freq = val;

            Hooks::instanace()->notifyFreqencyChange(&msg);
        }

    }

    if( 0 == m_reg_read_fn(NAV_STANDBY_REGISTER, &val, sizeof(val), 0))
    {
        if(val != m_standbyNavVal)
        {
            //logMessageEx("----- NAV_STANDBY_REGISTER %d", val);

            m_standbyNavVal = val;
            FreqInfo msg;
            msg.msgType = MGS_NAV_STANDBY;
            msg.freq = val;

            Hooks::instanace()->notifyFreqencyChange(&msg);

        }
    }

}

