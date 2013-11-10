#ifndef __IOP_SIM_HOOKS___
#define  __IOP_SIM_HOOKS___

#include <windows.h>
#include "sharedstruct.h"
#include "gnsTypes.h"

typedef unsigned long (__cdecl *TSK_pvg_send_msg_ex_t)(unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);


class IopSimHooks
{
public:
    IopSimHooks();
    virtual ~IopSimHooks();

    static IopSimHooks* instanace();

    bool hook(SharedStruct<GNSIntf>*  pShared);


    //GPS functions
    //latitude; //radians
    //longitude; //radians
    //speed; //m/s
    //heading; //0=> pi, -pi=>0
    //verticalSpeed; //0=>
    //altitude; // m
    void  setGPSInfo(double	latitude, double longitude, float speed, float	heading, float verticalSpeed, float altitude);


    unsigned long   TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2);
    unsigned long   reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2);

private:
    void            lock();
    void            unlock();

private:
    CRITICAL_SECTION    m_cs;

    static IopSimHooks* m_gInstance;

    SharedStruct<GNSIntf>*  m_pShared;
    GNSIntf*                m_pData;


    double		            m_latitude; //radians
    double		            m_longitude; //radians
    float		            m_speed; //m/s
    float		            m_heading; //0=> pi, -pi=>0
    float		            m_verticalSpeed; //0=>
    float		            m_altitude; // m
    //this will be set the first time inside IOP_SIM_hooks 
    // this is used as an offset to set the altitude
    float		            m_altitude2; // m
    

    TSK_pvg_send_msg_ex_t	m_TSK_pvg_send_msg_ex_fn;
    reg_read_t		        m_reg_read_fn;
    reg_write_t		        m_reg_write_fn;
    SYS_pvg_var_ctrl_t		m_SYS_pvg_var_ctrl_fn;

};

#endif //__IOP_SIM_HOOKS___