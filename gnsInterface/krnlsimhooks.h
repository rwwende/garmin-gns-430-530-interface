#ifndef __KRNLSIM_HOOKS___
#define  __KRNLSIM_HOOKS___

#include <windows.h>
#include "sharedstruct.h"
#include "gnsTypes.h"

typedef unsigned long (__cdecl *SIM_proc_rqst_t)(unsigned long p1);
//typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
//typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
//typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);


class KrlnsimHooks
{
public:
    KrlnsimHooks();
    virtual ~KrlnsimHooks();

    static KrlnsimHooks* instanace();

    bool hook(SharedStruct<GNSIntf>*  pShared);


    unsigned long  SIM_proc_rqst(unsigned long p1);

private:
    void            lock();
    void            unlock();

private:
    CRITICAL_SECTION    m_cs;

    static KrlnsimHooks* m_gInstance;

    SharedStruct<GNSIntf>*  m_pShared;
    GNSIntf*                m_pData;

    SIM_proc_rqst_t	            m_SIM_proc_rqst_fn;



};


#endif //#ifndef __KRNLSIM_HOOKS___