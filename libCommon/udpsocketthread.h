#ifndef __UDP_SOCKET_THREAD_CLASS_H__
#define __UDP_SOCKET_THREAD_CLASS_H__

#include "thread.h"
#include "udpsocket.h"

typedef void (*UdpDataCallback_t)(void* pData, int dataSize, void* context); 

class UdpSocketThread : public Win32Thread
{
public:
	UdpSocketThread(unsigned short port);
	virtual ~UdpSocketThread();

    void    setCallback(UdpDataCallback_t callback, void* pContext);

    void    terminate();
	    
protected:
	virtual void run();

private:
    UdpSocket       m_socket;
    unsigned short  m_port;

    UdpDataCallback_t m_callback;
    void*             m_pCallbackContext;

};

#endif //__UDP_SOCKET_THREAD_CLASS_H__