#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "cdp_vloc_box_sim_hooks.h"

static GNSIntf* pIntf;


///////////////////////////////////////////////////////////
typedef unsigned long (__cdecl *TSK_pvg_send_msg_ex_t)(unsigned long p1, unsigned long p2);
static TSK_pvg_send_msg_ex_t		g_TSK_pvg_send_msg_ex;

static unsigned long __cdecl my_TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2)
{
	unsigned long res=0;
	res =  g_TSK_pvg_send_msg_ex(p1, p2);

	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
static reg_read_t		g_reg_read;

static unsigned long __cdecl my_reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
	unsigned long res=0;
	
	res = g_reg_read(num, addr, size, p4);

	if(0x7f2 == num)
	{
		pIntf->nav1_active = *(unsigned long*)addr / 10;
		pIntf->nav1_changed = true;
	}else if(0x7f6 == num)
	{
		pIntf->nav1_standby = *(unsigned long*)addr /10;
		pIntf->nav1_changed = true;
	}


	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4);
static reg_write_t		g_reg_write;

static unsigned long __cdecl my_reg_write(unsigned long num, unsigned long *addr, unsigned long val, unsigned long p4)
{
	unsigned long res=0;

	res = g_reg_write(num, addr, val, p4);

	return res;
}

///////////////////////////////////////////

typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);
static SYS_pvg_var_ctrl_t		g_SYS_pvg_var_ctrl;

static unsigned long __cdecl my_SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
	unsigned long res=0;

    static bool first_time = true;
	if(first_time)
	{
		unsigned long freq;

		freq = pIntf->nav1_active*10;
		g_reg_write(0x7f2, &freq, 4,0);
		freq = pIntf->nav1_standby*10;
		g_reg_write(0x7f6, &freq, 4,0);

		first_time = false;
	}


	res = g_SYS_pvg_var_ctrl(p1, p2);

	return res;
}



static SDLLHook krnlsimHook = 
{
	"krnlsim.dll",
	false, NULL, // Default hook disabled, NULL function pointer.
	{
		//{ "TSK_pvg_send_msg_ex", my_TSK_pvg_send_msg_ex},
		{ "reg_read", my_reg_read},
		{ "reg_write", my_reg_write},
		{ "SYS_pvg_var_ctrl" , my_SYS_pvg_var_ctrl},
		{ NULL, NULL }
	}
};


int hook_cdp_vloc_box_sim(GNSIntf* pSharedInterface)
{
	int res = 0;
	int i;
	
	pIntf= pSharedInterface;


	HMODULE h_cdp_com_box_sim = GetModuleHandle("cdp_vloc_box_sim.dll");

	i=0;
	HookAPICallsMod(&krnlsimHook, h_cdp_com_box_sim);
	//g_TSK_pvg_send_msg_ex = (TSK_pvg_send_msg_ex_t)krnlsimHook.Functions[i++].OrigFn;
	g_reg_read = (reg_read_t)krnlsimHook.Functions[i++].OrigFn;
	g_reg_write = (reg_write_t)krnlsimHook.Functions[i++].OrigFn;
	g_SYS_pvg_var_ctrl = (SYS_pvg_var_ctrl_t)krnlsimHook.Functions[i++].OrigFn;


	
	return res;
}


