#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "iopsimhooks.h"
#include "log.h"




static unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    return IopSimHooks::instanace()->TSK_pvg_send_msg_ex(p1, p2);
}

static unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return IopSimHooks::instanace()->reg_read(num, addr, size, p4);
}

static unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    return IopSimHooks::instanace()->reg_write(num, addr, size, p4);
}

static unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    return IopSimHooks::instanace()->SYS_pvg_var_ctrl(p1, p2);
}






IopSimHooks* IopSimHooks::m_gInstance = NULL;

IopSimHooks::IopSimHooks()
{

    InitializeCriticalSection(&m_cs);

}

IopSimHooks::~IopSimHooks()
{
    DeleteCriticalSection(&m_cs);

}

IopSimHooks* IopSimHooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new IopSimHooks();
    }

    return m_gInstance;


}

void  IopSimHooks::lock()
{
    EnterCriticalSection(&m_cs);
}

void IopSimHooks::unlock()
{
    LeaveCriticalSection(&m_cs);
}


bool IopSimHooks::hook(SharedStruct<GNSIntf>*  pShared)
{

    bool res = true;
    int i;


    m_pShared = pShared;
    m_pData = pShared->get();

    HMODULE h_cdp_com_box_sim = GetModuleHandle("IOP_SIM.dll");

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


unsigned long IopSimHooks::TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res =  m_TSK_pvg_send_msg_ex_fn(p1, p2);

    logMessageEx("--- IopSimHooks::TSK_pvg_send_msg_ex %08x, %08x -> %08x", p1, p2, res);

    unlock();

    return res;
}


unsigned long IopSimHooks::reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{

    unsigned long res=0;

    lock();


    res = m_reg_read_fn(num, addr, size, p4);



    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    logMessageEx("--- IopSimHooks::reg_read %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);



    unlock();

    return res;

}

unsigned long IopSimHooks::reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;

    lock();

    res = m_reg_write_fn(num, addr, size, p4);


    unsigned long val = *(unsigned long*)addr;
    unsigned char ucval = *(unsigned char*)addr;
    logMessageEx("--- IopSimHooks::reg_write %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);


    unlock();

    return res;

}

unsigned long IopSimHooks::SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res = m_SYS_pvg_var_ctrl_fn(p1, p2);

    logMessageEx("--- IopSimHooks::SYS_pvg_var_ctrl %08x, %08x", p1, p2);


    unlock();

    return res;
}





#if 0

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "IOP_SIM_hooks.h"
#include "sharedstruct.h"


static GNSIntf* pIntf;
static HMODULE h_iop_sim;


///////////////////////////////////////////////////////////
typedef unsigned long (__cdecl *TSK_pvg_send_msg_ex_t)(unsigned long p1, unsigned long p2);
TSK_pvg_send_msg_ex_t		g_TSK_pvg_send_msg_ex;

unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
	unsigned long res=0;
	static bool first_message=true;

	//Update the autopilot data from the trainerShrObj data
	{
		static unsigned long *top = (unsigned long*)((unsigned long)h_iop_sim + 0x1fd68);
		
		unsigned char* trainerShrObj = (unsigned char*)(top[0]);
	
		if(NULL != trainerShrObj)
		{

			SharedObjShr* pSharedObj = (SharedObjShr*)(trainerShrObj+0x3C);
			

			//float*  pDTK = (float*)(trainerShrObj + 0x50 + 20);
			//unsigned long*  pGPSActive = (unsigned long*)(trainerShrObj + 0x50 + 32);

			//if(*pGPSActive > 0)
			if(pSharedObj->fpl_enabled)
			{
				float  DTK= pSharedObj->dtk;//*pDTK;

				if(DTK > 0)
				{
					DTK = DTK*180.0f/PI;
				}else
				{
					DTK = 360 + DTK*180.0f/PI;
				}

				pIntf->override_gps = true;
				pIntf->dtk = DTK;
				pIntf->lcdi = pSharedObj->lcdi;
				pIntf->vcdi = pSharedObj->vcdi;
				pIntf->cdi_horizontal_offtrack = pSharedObj->cdi_horizontal_offtrack;


			}else
			{
				// gps is not active
				pIntf->override_gps = false;
				pIntf->dtk = 0;
				pIntf->lcdi = 0;
				pIntf->vcdi = 0;
				pIntf->cdi_horizontal_offtrack = 0;

			}
		}else
		{
			// gps is not active
			pIntf->override_gps = false;
			pIntf->dtk = 0;
			pIntf->lcdi = 0;
			pIntf->vcdi = 0;
			pIntf->cdi_horizontal_offtrack = 0;

		}

	}
	if(0x12 == p1)
	{

		unsigned long* addr = (unsigned long* )p2;
		unsigned long addr_msg = *addr;

		//unsigned long* a1 = (unsigned long*)(addr_val+0*4);
		unsigned long  msg_type = *(unsigned long*)(addr_msg+1*4);


		if(0 == msg_type)
		{
			send_msg_ex_0x12* pmsg  = (send_msg_ex_0x12*)(addr_msg+0x0b);

			if(first_message)
			{
				pIntf->first_altitude2 = pmsg->altitude2;
				first_message = false;
			}

            if(pIntf->simulatorConnected)
            {
			    pmsg->altitude = pIntf->msg_ex_0x12.altitude;
			    pmsg->altitude2 = pIntf->msg_ex_0x12.altitude2;

			    pmsg->speed = pIntf->msg_ex_0x12.speed;
			    pmsg->heading = pIntf->msg_ex_0x12.heading;

			    pmsg->latitude = pIntf->msg_ex_0x12.latitude;
			    pmsg->longitude = pIntf->msg_ex_0x12.longitude;
            }


		}
		
	}

	res =  g_TSK_pvg_send_msg_ex(p1, p2);


	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4);
reg_read_t		g_reg_read;

unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4)
{
	unsigned long res=0;
	
	res = g_reg_read(num, addr, val, p4);

	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4);
reg_write_t		g_reg_write;

unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4)
{
	unsigned long res=0;

	res = g_reg_write(num, addr, val, p4);

	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);
SYS_pvg_var_ctrl_t		g_SYS_pvg_var_ctrl;

unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
	unsigned long res=0;

	res = g_SYS_pvg_var_ctrl(p1, p2);

	return res;
}



static SDLLHook krnlsimHook = 
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


int hook_IOP_SIM(GNSIntf* pSharedInterface)
{
	int res = 0;
	int i;
	

    pIntf= pSharedInterface;

	HMODULE h_gns530 = GetModuleHandle(NULL);
	h_iop_sim = GetModuleHandle("IOP_SIM.dll");
	HMODULE h_krnlsim = GetModuleHandle("krnlsim.dll");
	HMODULE h_sys_resources = GetModuleHandle("sys_resource.dll");

	i=0;
	HookAPICallsMod(&krnlsimHook, h_iop_sim);
	g_TSK_pvg_send_msg_ex = (TSK_pvg_send_msg_ex_t)krnlsimHook.Functions[i++].OrigFn;
	g_reg_read = (reg_read_t)krnlsimHook.Functions[i++].OrigFn;
	g_reg_write = (reg_write_t)krnlsimHook.Functions[i++].OrigFn;
	g_SYS_pvg_var_ctrl = (SYS_pvg_var_ctrl_t)krnlsimHook.Functions[i++].OrigFn;

	
	return res;
}


#endif