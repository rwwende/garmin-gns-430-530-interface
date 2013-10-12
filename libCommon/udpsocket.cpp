#include <windows.h>
#include <stdio.h>
#include "udpsocket.h"

bool UdpSocket::WSAInitialized = false;

int UdpSocket::InitWSA()
{
	int res = 0;
	
	if(!WSAInitialized)
	{
		WSADATA ws;
		WSAStartup(MAKEWORD(2, 2),&ws);
	}

	return res;
}

UdpSocket::UdpSocket()
{

}

UdpSocket::~UdpSocket()
{

}

int UdpSocket::openForSending(char* dest_address, unsigned short dest_port)
{
	int res = 0;

	peer.sin_family = AF_INET;
	peer.sin_port = htons(dest_port);
	peer.sin_addr.s_addr = inet_addr(dest_address);

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);


	return res;
}


int UdpSocket::openForReceiving(unsigned short port)
{
	int res = 0;

	peer.sin_family = AF_INET;
	peer.sin_port = htons(port);
	peer.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//INADDR_ANY);
	
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	// SOCK_STREAM is for TCP, SOCK_DGRAM is for UDP.
	
	res = bind(udp_socket,(struct sockaddr *)&peer,sizeof(peer));
	
	peerlen=sizeof(peer);


	

	return res;

}

int UdpSocket::send(const char* data, unsigned short size)
{
	int res = 0;
	
	res = sendto(udp_socket, data, size, 0, (struct sockaddr *)&peer, sizeof(peer));

	return res;
}

int UdpSocket::recv(char* data, unsigned short size)
{
	int retval; 
	retval = recvfrom(udp_socket, data, size, 0, (struct sockaddr *)&peer, &peerlen);
		
		//recvfrom returns -1 if it didn't receive anything.
		//If retval is NOT -1, then the recvfrom must have worked
	if (retval == -1) 
	{
		//printf(recvfrom error\n");
	}
	return retval;
	
}

int UdpSocket::close()
{
	int res = 0;

	::closesocket(udp_socket);

	return res;

}
