#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "hooks.h"
#include "iopsimhooks.h"
#include "log.h"

#define RAD2DEG (57.2957795f)

#pragma pack(push, 1)

typedef struct _send_msg_ex_0x12
{
    unsigned char unknown1[4];
    unsigned short hfom;
    unsigned char unknown2[2];
    unsigned short vfom;
    unsigned char unknown3[6];
    float		  altitude; // m
    float			speed; //m/s
    float			heading; //0=> pi, -pi=>0
    float			verticalSpeed; //0=>
    float		  altitude2; // m
    double		  latitude; //radians
    double		  longitude; //radians
    unsigned char	month;
    unsigned char	day;
    unsigned short	year;
    unsigned short	hour;
    unsigned char	minute;
    unsigned char	sec;
}send_msg_ex_0x12;

#pragma pack(pop)




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

    m_latitude = 0;
    m_longitude = 0;
    m_speed = 0;
    m_heading = 0;
    m_verticalSpeed = 0;
    m_altitude = 0;

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

    HMODULE hiop_sim_sim = GetModuleHandle("IOP_SIM.dll");

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
    HookAPICallsMod(&krnlsimHook, hiop_sim_sim);
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


    //logMessageEx("--- IopSimHooks::TSK_pvg_send_msg_ex IN %08x, %08x", p1, p2);

    if(0x12 == p1)
    {

        static bool first_message=true;

        unsigned long* addr = (unsigned long* )p2;
        unsigned long addr_msg = *addr;

        unsigned long  msg_type = *(unsigned long*)(addr_msg+1*4);

        //logMessageEx("--- IopSimHooks::TSK_pvg_send_msg_ex msg_type %08x", msg_type);

        if(0 == msg_type)
        {
            send_msg_ex_0x12* pmsg  = (send_msg_ex_0x12*)(addr_msg+0x0b);
#if 0
            logMessageEx("--- IopSimHooks::TSK_pvg_send_msg_ex lat=%f, lon=%f, spe=%f head=%f vs=%f, alt=%f, alt2=%f", 
                pmsg->latitude*RAD2DEG,
                pmsg->longitude*RAD2DEG,
                pmsg->speed,
                pmsg->heading*RAD2DEG, 
                pmsg->verticalSpeed,
                pmsg->altitude,
                pmsg->altitude2);
#endif



            if(first_message)
            {
                m_altitude2 = pmsg->altitude2;
                first_message = false;
            }

            pmsg->altitude = m_altitude;
            pmsg->altitude2 = m_altitude + m_altitude2;

            pmsg->speed = m_speed;
            pmsg->heading = m_heading;

            pmsg->latitude = m_latitude;
            pmsg->longitude = m_longitude;


        }

    }


    res =  m_TSK_pvg_send_msg_ex_fn(p1, p2);

    //logMessageEx("--- IopSimHooks::TSK_pvg_send_msg_ex OUT %08x, %08x -> %08x", p1, p2, res);


    unlock();

    return res;
}


unsigned long IopSimHooks::reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{

    unsigned long res=0;

    lock();


    res = m_reg_read_fn(num, addr, size, p4);



    //unsigned long val = *(unsigned long*)addr;
    //unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- IopSimHooks::reg_read %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);



    unlock();

    return res;

}

unsigned long IopSimHooks::reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4)
{
    unsigned long res=0;

    lock();

    res = m_reg_write_fn(num, addr, size, p4);


    //unsigned long val = *(unsigned long*)addr;
    //unsigned char ucval = *(unsigned char*)addr;
    //logMessageEx("--- IopSimHooks::reg_write %08x, %08x [%d][%d], %08x, %08x -> %08x", num, addr,  val, ucval, size, p4, res);


    unlock();

    return res;

}

unsigned long IopSimHooks::SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2)
{
    unsigned long res=0;

    lock();

    res = m_SYS_pvg_var_ctrl_fn(p1, p2);

    //logMessageEx("--- IopSimHooks::SYS_pvg_var_ctrl %08x, %08x", p1, p2);


    unlock();

    return res;
}

void  IopSimHooks::setGPSInfo(double	latitude, double longitude, float speed, float	heading, float verticalSpeed, float altitude)
{
    lock();

    m_latitude = latitude;
    m_longitude = longitude;
    m_speed = speed;
    m_heading = heading;
    m_verticalSpeed = verticalSpeed;
    m_altitude = altitude;


    //logMessageEx("--- IopSimHooks::setGPSInfo %f, %f, %f %f %f, %f", 
    //               m_latitude,
    //               m_longitude,
    //               m_speed,
    //               m_heading, 
    //               m_verticalSpeed,
    //               m_altitude);

    unlock();
}




