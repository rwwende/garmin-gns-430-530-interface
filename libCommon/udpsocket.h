#ifndef __UDP_SOCKET__
#define __UDP_SOCKET__

#include <windows.h>




class UdpSocket
{
	
public:
	UdpSocket();
	virtual ~UdpSocket();
public:
	static bool WSAInitialized;
	static int  InitWSA(); 
	int openForSending(char* dest_address, unsigned short dest_port);
	int openForReceiving(unsigned short port);
	int send(const char* data, unsigned short size);
	int recv(char* data, unsigned short size);
	
	int close();
private:
	SOCKET udp_socket;
	struct sockaddr_in peer;
	int peerlen;
	
};

#endif //__UDP_SOCKET__