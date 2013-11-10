#ifndef __GNSX30PROXY__
#define __GNSX30PROXY__

#include "sharedstruct.h"
#include "gnsTypes.h"


class CRCFile;
class FileLog;
class UdpSocketThread;
class UdpSocket;


class GNSx30Proxy
{
    enum GnsState
    {
        stateTerminated = 0,
        stateInitialized,
        stateOpened,
    };

public:
	GNSx30Proxy();
	virtual ~GNSx30Proxy();

	bool initialize(bool hideGUI);
	bool terminate();
	bool open(int gnsType);
	bool close();
	GNSIntf* getInterface();
	int sendMsg(int up, int x, int y );

    void processUdpData(void* pData, int dataSize);


    //COM functions
    unsigned long getCOMActiveFrequency() { return m_comActive;};
    void  setCOMActiveFrequency(unsigned long freq);

    unsigned long getCOMStandbyFrequency() { return m_comStandby;};
    void  setCOMStandbyFrequency(unsigned long freq);

    //NAV functions
    unsigned long getNAVActiveFrequency() { return m_navActive;};
    void  setNAVActiveFrequency(unsigned long freq);

    unsigned long getNAVStandbyFrequency() { return m_navStandby;};
    void  setNAVStandbyFrequency(unsigned long freq);


    //GPS functions
    //latitude; //radians
    //longitude; //radians
    //speed; //m/s
    //heading; //0=> pi, -pi=>0
    //verticalSpeed; //0=>
    //altitude; // m
    void  setGPSInfo(double	latitude, double longitude, float speed, float	heading, float verticalSpeed, float altitude);


private:
    //COM/NAV frequencies
    unsigned long           m_comActive;
    unsigned long           m_comStandby;
    unsigned long           m_navActive;
    unsigned long           m_navStandby;


    GnsState                m_state;

	char					m_trainter_path[MAX_PATH];
	char					m_trainter_exe[MAX_PATH];
    char					m_interface_lib[MAX_PATH];
	HWND					m_win;
	
    SharedStruct<GNSIntf>*  m_pShared;
	GNSIntf*				m_pvData;

	CRCFile*				m_pCRCFile;
    bool                    m_hideGUI;

    UdpSocketThread*        m_pServerSocketThread; //This socket thread will receive data form the Garming Sim
    UdpSocket*              m_pClientSocket; //This socket thread will send data to the Garming Sim


};	

#endif // __GNSX30PROXY__
