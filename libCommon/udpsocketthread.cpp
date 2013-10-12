#include "udpsocketthread.h"
#include "log.h"

UdpSocketThread::UdpSocketThread(unsigned short port)
:Win32Thread(),
m_port(port)
{
    UdpSocket::InitWSA();
    m_socket.openForReceiving(m_port);

    m_callback = NULL;
    m_pCallbackContext = NULL; 

}

UdpSocketThread::~UdpSocketThread()
{

}

void UdpSocketThread::terminate()
{
    //logMessageEx("--- UdpSocketThread::terminate ENTER");
    shutdown();
    m_socket.close();
    join();
    //logMessageEx("--- UdpSocketThread::terminate EXIT");

}

void UdpSocketThread::run()
{
#define MAX_PACKET_SUZE 1024
    char data[MAX_PACKET_SUZE];

    while(canRun())
    {
        int r = m_socket.recv(data, MAX_PACKET_SUZE);

        if(r > 0)
        {
            //data received
            logMessageEx("--- UdpSocketThread::run r=%d", r);
            if(NULL != m_callback)
            {
                m_callback(data, r, m_pCallbackContext);
            }
        }
    }

}

void UdpSocketThread::setCallback(UdpDataCallback_t callback, void* pContext)
{
    m_callback = callback;
    m_pCallbackContext = pContext; 

}