#ifndef __TEST_HOOKS___
#define  __TEST_HOOKS___

#include <windows.h>
#include "sharedstruct.h"
#include "gnsTypes.h"

//#pragma warning FIXME (move this to a separate file)

//HWM
typedef unsigned long (__cdecl *HWM_pvg_get_obs_t)(unsigned long p1);

//TSK
typedef unsigned long (__cdecl *TSK_pvg_wait_evnt_t)(unsigned long p1);
typedef unsigned long (__cdecl *TSK_pvg_get_timer_t)(void);
typedef unsigned long (__cdecl *TSK_pvg_send_msg_ex_t)(unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *TSK_pvg_get_msg_t)(unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *TSK_pvg_proc_status_t)(unsigned long p1);
typedef unsigned long (__cdecl *reg_read_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
typedef unsigned long (__cdecl *reg_write_t)(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);

//SYS
typedef unsigned long (__cdecl *SYS_pvg_var_ctrl_t)(unsigned long p1, unsigned long p2);

//FILE
typedef unsigned long (__cdecl *FIL_vfs_open_t)(char* name, unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *FIL_vfs_mmap_t)(unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4, unsigned long p5, unsigned long p6);
typedef unsigned long (__cdecl *mem_unmap_t)(unsigned long p1, unsigned long p2);
typedef unsigned long (__cdecl *FIL_vfs_seek_t)(unsigned long p1, unsigned long p2, unsigned long p3);
typedef void          (__cdecl *FIL_vfs_close_t)(unsigned long p1);
typedef unsigned long (__cdecl *FIL_vfs_rename_t)(char* p1, char* p2);
typedef unsigned long (__cdecl *FIL_vfs_write_t)(unsigned long p1, unsigned long p2, unsigned long p3);

///
typedef char* (__cdecl *TXT_get_string_t)(unsigned long p1);


class TestHooks
{
public:
    TestHooks();
    virtual ~TestHooks();

    static TestHooks* instanace();

    bool hook(SharedStruct<GNSIntf>*  pShared);

    //COM functions
    void  setActiveFrequency(unsigned long freq);
    void  setStandbyFrequency(unsigned long freq);

    unsigned long   HWM_pvg_get_obs(unsigned long p1);


    unsigned long   TSK_pvg_get_timer(void);
    unsigned long   TSK_pvg_wait_evnt(unsigned long p1);
    unsigned long   TSK_pvg_send_msg_ex(unsigned long p1, unsigned long p2);
    unsigned long   TSK_pvg_get_msg(unsigned long p1, unsigned long p2);
    unsigned long   TSK_pvg_proc_status(unsigned long p1);
    unsigned long   reg_read(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   reg_write(unsigned long num, unsigned long *addr, unsigned long size, unsigned long p4);
    unsigned long   SYS_pvg_var_ctrl(unsigned long p1, unsigned long p2);

    unsigned long   FIL_vfs_open(char* name, unsigned long p1, unsigned long p2);
    unsigned long   FIL_vfs_mmap(unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4, unsigned long p5, unsigned long p6);
    unsigned long   mem_unmap(unsigned long p1, unsigned long p2);
    unsigned long   FIL_vfs_seek(unsigned long p1, unsigned long p2, unsigned long p3);
    void            FIL_vfs_close(unsigned long p1);
    unsigned long   FIL_vfs_rename(char* p1, char* p2);
    unsigned long   FIL_vfs_write(unsigned long p1, unsigned long p2, unsigned long p3);

    char*           TXT_get_string(unsigned long p1);
    

private:
    void            lock();
    void            unlock();

private:
    CRITICAL_SECTION    m_cs;

    static TestHooks* m_gInstance;

    SharedStruct<GNSIntf>*  m_pShared;
    GNSIntf*                m_pData;

    HWM_pvg_get_obs_t       m_HWM_pvg_get_obs_fn;

    TSK_pvg_get_timer_t     m_TSK_pvg_get_timer_fn;
    TSK_pvg_wait_evnt_t     m_TSK_pvg_wait_evnt_fn; 
    TSK_pvg_send_msg_ex_t	m_TSK_pvg_send_msg_ex_fn;
    TSK_pvg_get_msg_t	    m_TSK_pvg_get_msg_fn;
    TSK_pvg_proc_status_t   m_TSK_pvg_proc_status_fn;
    reg_read_t		        m_reg_read_fn;
    reg_write_t		        m_reg_write_fn;
    SYS_pvg_var_ctrl_t		m_SYS_pvg_var_ctrl_fn;
    FIL_vfs_open_t          m_FIL_vfs_open_fn;
    FIL_vfs_mmap_t          m_FIL_vfs_mmap_fn;
    mem_unmap_t             m_mem_unmap_fn;
    FIL_vfs_seek_t          m_FIL_vfs_seek_fn;
    FIL_vfs_close_t         m_FIL_vfs_close_fn;
    FIL_vfs_rename_t        m_FIL_vfs_rename_fn;
    FIL_vfs_write_t         m_FIL_vfs_write_fn;


    TXT_get_string_t        m_TXT_get_string_fn;

};


#endif //#ifndef __TEST_HOOKS___