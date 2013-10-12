#ifndef __COM_HOOKS___
#define  __COM_HOOKS___

#include <windows.h>
#include "sharedstruct.h"
#include "gnsTypes.h"

typedef unsigned long (__cdecl *TSK_pvg_send_msg_ex_t)(unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);


class ComHooks
{
public:
    ComHooks();
    virtual ~ComHooks();

    static ComHooks* instanace();

    bool hook(SharedStruct<GNSIntf>*  pShared);

    //COM functions
    void  setActiveFrequency(unsigned long freq);
    void  setStandbyFrequency(unsigned long freq);

    
    unsigned long   TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2);
    unsigned long   reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2);

private:
    void            lock();
    void            unlock();
    void            checkFrequencies();

private:
    CRITICAL_SECTION    m_cs;

    static ComHooks* m_gInstance;

    SharedStruct<GNSIntf>*  m_pShared;
    GNSIntf*                m_pData;

    unsigned long           m_activeComSet;
    unsigned long           m_standbyComSet;

    unsigned long           m_activeComVal;
    unsigned long           m_standbyComVal;

    TSK_pvg_send_msg_ex_t	m_TSK_pvg_send_msg_ex_fn;
    reg_read_t		        m_reg_read_fn;
    reg_write_t		        m_reg_write_fn;
    SYS_pvg_var_ctrl_t		m_SYS_pvg_var_ctrl_fn;



};


#endif //#ifndef __COM_HOOKS___