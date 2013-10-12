#ifndef __HOOKS___
#define  __HOOKS___

#include <windows.h>
#include "sharedstruct.h"
#include "gnsTypes.h"


class UdpSocketThread;
class UdpSocket;

class Hooks
{
public:
    Hooks();
    virtual ~Hooks();
    
    static Hooks* instanace();

    bool hookGnsx30(HMODULE hModule);

    bool notifyFreqencyChange(FreqInfo* pInfo);

    void processUdpData(void* pData, int dataSize);
public:
    static Hooks* m_gInstance;

    SharedStruct<GNSIntf>*  m_pShared;
    UdpSocketThread*        m_pServerSocketThread; //This socket thread will receive data form the proxy
    UdpSocket*              m_pClientSocket; //This socket thread will send data to the proxy

};

#endif //__HOOKS___