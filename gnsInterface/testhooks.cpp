#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "testhooks.h"
#include "log.h"




static unsigned long __cdecl my_HWM_pvg_get_obs(unsigned long p1)
{
    return TestHooks::instanace()->HWM_pvg_get_obs(p1);
}


static unsigned long __cdecl my_TSK_pvg_wait_evnt(unsigned long p1)
{
    return TestHooks::instanace()->TSK_pvg_wait_evnt(p1);
}

static unsigned long my_TSK_pvg_get_timer(void)
{
    return TestHooks::instanace()->TSK_pvg_get_timer();
}

static unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    return TestHooks::instanace()->TSK_pvg_send_msg_ex(p1, p2);
}

static unsigned long __cdecl my_TSK_pvg_get_msg(unsigned long p1, unsigned long p2)
{
    return TestHooks::instanace()->TSK_pvg_get_msg(p1, p2);
}

static unsigned long __cdecl my_TSK_pvg_proc_status(unsigned long p1)
{
    return TestHooks::instanace()->TSK_pvg_proc_status(p1);
}

static unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return TestHooks::instanace()->reg_read(num, addr, size, p4);
}

static unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return TestHooks::instanace()->reg_write(num, addr, size, p4);
}

static unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    return TestHooks::instanace()->SYS_pvg_var_ctrl(p1, p2);
}

//FILE

static unsigned long __cdecl my_FIL_vfs_open(char* name, unsigned long p1, unsigned long p2)
{
    return TestHooks::instanace()->FIL_vfs_open(name, p1, p2);
}

static unsigned long  __cdecl my_FIL_vfs_mmap(unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4, unsigned long p5, unsigned long p6)
{
    return TestHooks::instanace()->FIL_vfs_mmap(p1, p2, p3, p4, p5, p6);
}


static unsigned long  __cdecl my_mem_unmap(unsigned long p1, unsigned long p2)
{
    return TestHooks::instanace()->mem_unmap(p1, p2);
}


static unsigned long  __cdecl my_FIL_vfs_seek(unsigned long p1, unsigned long p2, unsigned long p3)
{
    return TestHooks::instanace()->FIL_vfs_seek(p1, p2, p3);
}

static void  __cdecl my_FIL_vfs_close(unsigned long p1)
{
    TestHooks::instanace()->FIL_vfs_close(p1);
}

static unsigned long  __cdecl my_FIL_vfs_rename(char* p1, char* p2)
{
    return TestHooks::instanace()->FIL_vfs_rename(p1, p2);
}


static unsigned long  __cdecl my_FIL_vfs_write(unsigned long p1, unsigned long p2, unsigned long p3)
{
    return TestHooks::instanace()->FIL_vfs_write(p1, p2, p3);
}


////
static char* __cdecl my_TXT_get_string(unsigned long p1)
{
    return TestHooks::instanace()->TXT_get_string(p1);
}






TestHooks* TestHooks::m_gInstance = NULL;

TestHooks::TestHooks()
{

    InitializeCriticalSection(&m_cs);

}

TestHooks::~TestHooks()
{
    DeleteCriticalSection(&m_cs);

}

TestHooks* TestHooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new TestHooks();
    }

    return m_gInstance;


}

void  TestHooks::lock()
{
    EnterCriticalSection(&m_cs);
}

void TestHooks::unlock()
{
    LeaveCriticalSection(&m_cs);
}


bool TestHooks::hook(SharedStruct<GNSIntf>*  pShared)
{

    bool res = true;
    int i;


    m_pShared = pShared;
    m_pData = pShared->get();

    HMODULE hMod;
    //hMod = GetModuleHandle("cdp_com_box_sim.dll");
    //hMod = GetModuleHandle("cdp_vloc_box_sim.dll");
    //hMod = GetModuleHandle("cdp_annun_box_sim.dll");
    //hMod = GetModuleHandle("sys_resource.dll");
    hMod = GetModuleHandle("IOP_B.dll");


    
//#pragma warning FIXME
    SDLLHook krnlsimHook = 
    {
        "krnlsim.dll",
        //false, NULL, // Default hook disabled, NULL function pointer.
        true, DefaultHook,
        {

            { "HWM_pvg_get_obs", my_HWM_pvg_get_obs},
            { "TSK_pvg_wait_evnt", my_TSK_pvg_wait_evnt},
            { "TSK_pvg_get_timer", my_TSK_pvg_get_timer},
            { "TSK_pvg_send_msg_ex", my_TSK_pvg_send_msg_ex},
            { "TSK_pvg_get_msg", my_TSK_pvg_get_msg},
            { "TSK_pvg_proc_status", my_TSK_pvg_proc_status},
            { "reg_read", my_reg_read},
            { "reg_write", my_reg_write},
            { "SYS_pvg_var_ctrl" , my_SYS_pvg_var_ctrl},
            { "FIL_vfs_open" , my_FIL_vfs_open},
            { "FIL_vfs_mmap" , my_FIL_vfs_mmap},
            { "mem_unmap" , my_mem_unmap},
            { "FIL_vfs_seek" , my_FIL_vfs_seek},
            { "FIL_vfs_close" , my_FIL_vfs_close},
            { "FIL_vfs_rename" , my_FIL_vfs_rename},
            { "FIL_vfs_write" , my_FIL_vfs_write},
            
            { NULL, NULL }
        }
    };



    i=0;
    HookAPICallsMod(&krnlsimHook, hMod);
    
    m_HWM_pvg_get_obs_fn = (HWM_pvg_get_obs_t)krnlsimHook.Functions[i++].OrigFn;
    m_TSK_pvg_wait_evnt_fn = (TSK_pvg_wait_evnt_t)krnlsimHook.Functions[i++].OrigFn;
    m_TSK_pvg_get_timer_fn = (TSK_pvg_get_timer_t)krnlsimHook.Functions[i++].OrigFn;
    m_TSK_pvg_send_msg_ex_fn = (TSK_pvg_send_msg_ex_t)krnlsimHook.Functions[i++].OrigFn;
    m_TSK_pvg_get_msg_fn = (TSK_pvg_get_msg_t)krnlsimHook.Functions[i++].OrigFn;
    m_TSK_pvg_proc_status_fn = (TSK_pvg_proc_status_t)krnlsimHook.Functions[i++].OrigFn;
    m_reg_read_fn = (reg_read_t)krnlsimHook.Functions[i++].OrigFn;
    m_reg_write_fn = (reg_write_t)krnlsimHook.Functions[i++].OrigFn;
    m_SYS_pvg_var_ctrl_fn = (SYS_pvg_var_ctrl_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_open_fn = (FIL_vfs_open_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_mmap_fn = (FIL_vfs_mmap_t)krnlsimHook.Functions[i++].OrigFn;
    m_mem_unmap_fn = (mem_unmap_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_seek_fn = (FIL_vfs_seek_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_close_fn = (FIL_vfs_close_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_rename_fn = (FIL_vfs_rename_t)krnlsimHook.Functions[i++].OrigFn;
    m_FIL_vfs_write_fn = (FIL_vfs_write_t)krnlsimHook.Functions[i++].OrigFn;



    SDLLHook sysresourcesHook = 
    {
        "sys_resource.dll",
        false, NULL, // Default hook disabled, NULL function pointer.
        //true, DefaultHook,
        {

            { "TXT_get_string", my_TXT_get_string},
            { NULL, NULL }
        }
    };
    i=0;
    HookAPICallsMod(&sysresourcesHook, hMod);
    m_TXT_get_string_fn = (TXT_get_string_t)sysresourcesHook.Functions[i++].OrigFn;




    return res;
}

unsigned long TestHooks::HWM_pvg_get_obs(unsigned long p1)
{
    unsigned long res=0;

    res = m_HWM_pvg_get_obs_fn(p1);

    unsigned long* pAddr = (unsigned long*)p1;

    logMessageEx("--- TestHooks::HWM_pvg_get_obs %08x [%08x][%d]-> %08x", p1, *pAddr, *pAddr, res);

    return res;

}


unsigned long TestHooks::TSK_pvg_wait_evnt(unsigned long p1)
{
    unsigned long res=0;

    res = m_TSK_pvg_wait_evnt_fn(p1);

    //logMessageEx("--- TestHooks::TSK_pvg_wait_evnt %08x -> %08x", p1, res);

    return res;
}

unsigned long TestHooks::TSK_pvg_get_timer(void)
{
    unsigned long res=0;

    res = m_TSK_pvg_get_timer_fn();

    //logMessageEx("--- TestHooks::TSK_pvg_get_timer -> %d", res);

    return res;

}

unsigned long TestHooks::TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    res =  m_TSK_pvg_send_msg_ex_fn(p1, p2);

    logMessageEx("--- TestHooks::TSK_pvg_send_msg_ex %08x, %08x -> %08x", p1, p2, res);


    return res;

}


unsigned long TestHooks::TSK_pvg_proc_status(unsigned long p1)
{
    unsigned long res=0;

    res =  m_TSK_pvg_proc_status_fn(p1);

    logMessageEx("--- TestHooks::TSK_pvg_proc_status %08x  -> %08x", p1, res);

    return res;
}

unsigned long TestHooks::TSK_pvg_get_msg(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;
    unsigned long val1;
    unsigned long val2;



    unsigned long* pval1 = (unsigned long*)p1;
    unsigned long* pval2 = (unsigned long*)p2;
    *pval1 = 0;
    *pval2 = 0;

    val1 = *(unsigned long*)p1;
    val2 = *(unsigned long*)p2;

    //logMessageEx("--- TestHooks::TSK_pvg_get_msg IN %08x [%08x], %08x [%08x] -> %08x", p1, val1, p2, val2, res);

    res =  m_TSK_pvg_get_msg_fn(p1, p2);
    //res = 1;

    if(0 == res)
    {
        *pval1 = *pval1 & 0x0000FFFF;
    }

    val1 = *(unsigned long*)p1;
    val2 = *(unsigned long*)p2;

    logMessageEx("--- TestHooks::TSK_pvg_get_msg %08x [%08x], %08x [%08x] -> %08x", p1, val1, p2, val2, res);

    return res;

}



unsigned long TestHooks::reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;


    res = m_reg_read_fn(num, addr, size, p4);

    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    logMessageEx("--- TestHooks::reg_read %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);

    return res;

}

unsigned long TestHooks::reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;

    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    logMessageEx("--- TestHooks::reg_write %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);

    res = m_reg_write_fn(num, addr, size, p4);

    return res;

}

unsigned long TestHooks::SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    //unsigned long val1 = *(unsigned long*)p1;
    unsigned long val2 = *(unsigned long*)p2;
    
    res = m_SYS_pvg_var_ctrl_fn(p1, p2);

    logMessageEx("--- TestHooks::SYS_pvg_var_ctrl %08x, %08x [%08x] -> %08x", p1, p2, val2, res);

    return res;
}


unsigned long TestHooks::FIL_vfs_open(char* name, unsigned long p1, unsigned long p2)
{
    unsigned long res=0;


    res = m_FIL_vfs_open_fn(name, p1, p2);

    logMessageEx("--- TestHooks::FIL_vfs_open %s %08x, %08x -> %08x", name, p1, p2,  res);

    return res;

}

unsigned long TestHooks::FIL_vfs_mmap(unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4, unsigned long p5, unsigned long p6)
{
    unsigned long res=0;


    res = m_FIL_vfs_mmap_fn(p1, p2, p3, p4, p5, p6);

    logMessageEx("--- TestHooks::FIL_vfs_mmap %08x, %08x, %08x, %08x, %08x, %08x ->%08x", p1, p2, p3, p4, p5, p6, res);

    return res;

}

unsigned long TestHooks::mem_unmap(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;


    res = m_mem_unmap_fn(p1, p2);

    logMessageEx("--- TestHooks::mem_unmap %08x, %08x -> %08x", p1, p2, res);

    return res;

}


unsigned long TestHooks::FIL_vfs_seek(unsigned long p1, unsigned long p2, unsigned long p3)
{
    unsigned long res=0;


    res = m_FIL_vfs_seek_fn(p1, p2, p3);

    logMessageEx("--- TestHooks::FIL_vfs_seek %08x, %08x, %08x ->%08x", p1, p2, p3, res);

    return res;

}

void TestHooks::FIL_vfs_close(unsigned long p1)
{

    m_FIL_vfs_close_fn(p1);

    logMessageEx("--- TestHooks::FIL_vfs_close %08x", p1);

}

unsigned long TestHooks::FIL_vfs_rename(char* p1, char* p2)
{
    unsigned long res=0;


    res = m_FIL_vfs_rename_fn(p1, p2);

    logMessageEx("--- TestHooks::FIL_vfs_rename %s, %s -> %08x", p1, p2, res);

    return res;

}


unsigned long TestHooks::FIL_vfs_write(unsigned long p1, unsigned long p2, unsigned long p3)
{
    unsigned long res=0;


    res = m_FIL_vfs_write_fn(p1, p2, p3);

    logMessageEx("--- TestHooks::FIL_vfs_write %08x, %08x, %08x ->%08x", p1, p2, p3, res);

    return res;

}



//Resources
char*  TestHooks::TXT_get_string(unsigned long p1)
{
    char* res = m_TXT_get_string_fn(p1);

    logMessageEx("--- TestHooks::TXT_get_string %08x -> %s", p1, res);

    return res;
}