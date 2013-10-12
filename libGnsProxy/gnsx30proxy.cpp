#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "gnsx30proxy.h"
#include "udpsocketthread.h"
#include "udpsocket.h"
#include "injector.h"
#include "crcfile.h"
#include "log.h"
#include "config.h"


void UdpDataCallback(void* pData, int dataSize, void* context)
{
    GNSx30Proxy* pGNSx30Proxy = (GNSx30Proxy*)context;
    pGNSx30Proxy->processUdpData(pData, dataSize);

}

GNSx30Proxy::GNSx30Proxy()
{
    
	m_pCRCFile = new CRCFile();

	m_win = NULL;

    m_pServerSocketThread = NULL;
    m_pClientSocket = NULL;

    //Default frquencies
    m_comActive = 134100;
    m_comStandby = 130825;
    m_navActive = 108700;
    m_navStandby = 111450;

    m_state = stateTerminated;

}


GNSx30Proxy::~GNSx30Proxy()
{
	delete m_pCRCFile;

    if(stateOpened == m_state)
    {
        close();
    }

    if(stateInitialized == m_state)
    {
        terminate();
    }


}



bool GNSx30Proxy::initialize(bool hideGUI)
{
	bool res = true;

    if(stateTerminated != m_state)
    {
        res = false;
        return res;
    }
	
	m_win = NULL; 

    m_hideGUI = hideGUI;

	//find the trainer path and copy the G530SIM.exe to th appropriate executable
	{
		HKEY hKey;
		DWORD dwSize = MAX_PATH;
		DWORD dwType;
		LONG lResult;
		WIN32_FIND_DATA FileData; 
		HANDLE hSearch; 
		//DWORD dwAttrs;   
		char szG530SIMFileName[MAX_PATH];   
		unsigned long CRCVal;

		char trainerpath[MAX_PATH];   


		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
					G530SIMEXE_PATH, 0, KEY_READ, &hKey);

		if(ERROR_SUCCESS != lResult)
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error opening key " G530SIMEXE_PATH);
			res = false;
			return res;
		}

		lResult = RegQueryValueEx(hKey, "Path", 0, &dwType,  (LPBYTE)trainerpath, &dwSize);
		RegCloseKey(hKey);


		if(ERROR_SUCCESS != lResult)
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error reading key " G530SIMEXE_PATH);

			res = false;
			return res;
		}

		strcpy(m_trainter_path, trainerpath);
		strcat(m_trainter_path, "\\Trainer");
		strcpy(m_trainter_exe, m_trainter_path);
		strcat(m_trainter_exe, "\\gnsx30.exe");
		
        strcpy(m_interface_lib, m_trainter_path);
        strcat(m_interface_lib, "\\gnsInterface.dll");

        

		//Check if the file gns530 already exists
		
		hSearch = FindFirstFile(m_trainter_exe, &FileData); 

		

	   if (hSearch == INVALID_HANDLE_VALUE) 
	   { 



			strcpy(szG530SIMFileName, m_trainter_path);
			strcat(szG530SIMFileName, "\\G530SIM.exe");
		   if(!CopyFile(szG530SIMFileName,m_trainter_exe,FALSE))
		   {
			   logMessageEx("??? GNSx30Proxy::Initialize Error copying file %s", szG530SIMFileName);

			   res = false;
			   return res;
		   }
	   }

		// Now calculate the CRC
		if(!m_pCRCFile->calcCRC(m_trainter_exe, &CRCVal))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error calculating CRC");
			res = false;
			return res;
		}

		if(GNSX30EXE_CRC != CRCVal)
		{

			// Invalid trainter version
			logMessageEx("??? GNSx30Proxy::Initialize Error checking CRC");

			//Delete the copied file
			//DeleteFile(m_trainter_exe);
			res = false;
			return res;
		}

	}////find the trainer path and copy the G530SIM.exe to th appropriate executable

	//Create the registry keys for the trainer
	{

		HKEY hKey;
		DWORD dwSize = MAX_PATH;
		DWORD dwValue;
        DWORD dwValue1;
		DWORD dwDisposition;


		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
			   logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL);

			   res = false;
			   return res;
		}

		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_SETTINGS, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
     		   logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL_SETTINGS);

			   res = false;
			   return res;
		}

#if 0
		//Create the trainer settings values
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_530AWT, strlen(GNS_530AWT) + 1))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key CDUType");

			   res = -1;
			   return res;
		}
#endif

		RegFlushKey(hKey);
		RegCloseKey(hKey);


		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_WINDOW, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL_WINDOW);

			   res = false;
			   return res;
		}

        dwValue = 50;

        if(m_hideGUI)
        {
            dwValue = 10000;  
        }
        //Create the window position values
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "x1", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
        {
            logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);

            res = false;
            return res;
        }

        if (ERROR_SUCCESS != RegSetValueEx(hKey, "y1", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
        {
            logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
            res = false;
            return res;
        }

        dwValue1 = dwValue + 480;
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "x2", 0, REG_DWORD, (LPBYTE)&dwValue1, sizeof(dwValue1)))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
			   res = false;
			   return res;
		}
        dwValue1 = dwValue + 630;
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "y2", 0, REG_DWORD, (LPBYTE)&dwValue1, sizeof(dwValue1)))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
			   res = false;
			   return res;
		}
		RegFlushKey(hKey);
		RegCloseKey(hKey);



	}

	//Create the shared data structure
    m_pShared = new SharedStruct<GNSIntf>(SHMEM_NAME, true, 0);

	m_pvData = m_pShared->get();

	memset(m_pvData->LDC_data,0x00, OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT*4);
    memset(m_pvData->Bezel_data,0x00, OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT*4);

    m_state = stateInitialized;

	return res;
}

bool GNSx30Proxy::terminate()
{
	bool res = true;

    if(stateInitialized != m_state)
    {
        res = false;
        return res;
    }

	delete 	m_pShared; 
    m_pvData = NULL;
    
    m_state = stateTerminated;

	return res;

}


bool GNSx30Proxy::open(int gnsType)
{
	bool res = true;

    if(stateInitialized != m_state)
    {
        res = false;
        return res;
    }


    HKEY hKey;
    DWORD dwDisposition;

    //Default frquencies
    m_comActive = 134100;
    m_comStandby = 130825;
    m_navActive = 108700;
    m_navStandby = 111450;


    m_pvData->gnsType = gnsType;

#pragma warning FIXME
    m_pvData->garminTrainerPort = 5000;
    m_pvData->proxyPort = 5001;


    //Start the serverThread
    m_pServerSocketThread = new UdpSocketThread(m_pvData->proxyPort);
    m_pServerSocketThread->create();
    m_pServerSocketThread->setCallback(UdpDataCallback, this);
    m_pServerSocketThread->resume();
    

    m_pClientSocket  = new UdpSocket();
    m_pClientSocket->openForSending("127.0.0.1", m_pvData->garminTrainerPort);


    if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_SETTINGS, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
        )
    {
        logMessageEx("??? GNSx30Proxy::open Error creating key " GARMIN_INTERNATIONAL_SETTINGS);

        res = false;
        return res;
    }

    if(TYPE_GNS430 == gnsType)
    {

        //Create the trainer settings values
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_430AWT, strlen(GNS_430AWT) + 1))
        {
            logMessageEx("??? GNSx30Proxy::open Error writing key CDUType");

            res = false;
            return res;
        }
    }else if(TYPE_GNS530 == gnsType)
    {
        //Create the trainer settings values
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_530AWT, strlen(GNS_530AWT) + 1))
        {
            logMessageEx("??? GNSx30Proxy::open Error writing key CDUType");

            res = false;
            return res;
        }
    }

    
    RegFlushKey(hKey);
    RegCloseKey(hKey);



	memset(m_pvData->LDC_data,0x00, OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT*4);

    startAndInject(m_trainter_exe, m_trainter_path, m_interface_lib, m_hideGUI);
    




	int count = 0;
	bool procesStarted = false;
	while(count < 20) // 10 sec
	{
		m_win = FindWindow("AfxFrameOrView42", "GARMIN 400W/500W Trainer");
		if(NULL != m_win)
		{
			procesStarted = true;
			break;
		}
		Sleep(500);
		count++;
	}

	if(!procesStarted)
	{
		logMessageEx("??? GNSx30Proxy::Open Error starting process %s", m_trainter_exe);
		res = false;
		return res;

	}


    m_state = stateOpened;

	return res;

}

bool GNSx30Proxy::close()
{
	bool res = true;
	LRESULT r;

    if(stateOpened != m_state)
    {
        res = false;
        return res;
    }

	//Find the window handle and close the window
    if(NULL == 	m_win)	
    {
    	m_win = FindWindow("AfxFrameOrView42", "GARMIN 400W/500W Trainer");
    }
	//r = SendMessage(win, WM_QUIT, 0,0);      
	if(NULL != m_win)
	{
	
		r = PostMessage(m_win, WM_QUIT, 0,0);      

    }else
	{
		logMessageEx("??? GNSx30Proxy::Close Error finding window %s:%s" "AfxFrameOrView42", "GARMIN 400W/500W Trainer");	
	}

	m_win = NULL;

    m_pServerSocketThread->terminate();
    delete m_pServerSocketThread;
    m_pServerSocketThread = NULL;

    delete m_pClientSocket;
    m_pClientSocket = NULL;
    

    m_state = stateInitialized;

	return res;

}


GNSIntf* GNSx30Proxy::getInterface()
{
	return m_pvData;
}

int GNSx30Proxy::sendMsg(int up, int x, int y )
{	
	int res = 0;
	
	DWORD lParam;

    if(NULL == 	m_win)	
    {
        m_win = FindWindow("AfxFrameOrView42", "GARMIN 400W/500W Trainer");
    }

	if(NULL != m_win)
	{
	
		lParam = y;
		lParam<<=16;
		lParam|=x;
		if(up)
		{
			PostMessage(m_win, WM_LBUTTONUP, MK_LBUTTON,lParam);  	
		}else
		{
			PostMessage(m_win, WM_LBUTTONDOWN, MK_LBUTTON,lParam);  	

		}
	}

	return res;
}

void GNSx30Proxy::processUdpData(void* pData, int dataSize)
{
    if(dataSize >0)
    {
        unsigned char msgType = ((unsigned char*)pData)[0];
        switch(msgType)
        {
            case MGS_COM_ACTIVE:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                m_comActive = pFreqInfo->freq;
                logMessageEx("--- GNSx30Proxy::processUdpData m_comActive %d", m_comActive);
                break;
            }
            case MGS_COM_STANDBY:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                m_comStandby = pFreqInfo->freq;

                logMessageEx("--- GNSx30Proxy::processUdpData m_comStandby %d", m_comStandby);

                break;
            }
            case MGS_NAV_ACTIVE:
                {
                    FreqInfo* pFreqInfo = (FreqInfo*)pData;
                    m_navActive = pFreqInfo->freq;
                    logMessageEx("--- GNSx30Proxy::processUdpData m_navActive %d", m_comActive);
                    break;
                }
            case MGS_NAV_STANDBY:
                {
                    FreqInfo* pFreqInfo = (FreqInfo*)pData;
                    m_navStandby = pFreqInfo->freq;

                    logMessageEx("--- GNSx30Proxy::processUdpData m_navStandby %d", m_comStandby);

                    break;
                }

        }
    }
}


//COM functions
void  GNSx30Proxy::setCOMActiveFrequency(unsigned long freq)
{
    FreqInfo msg;

    msg.msgType = MGS_COM_ACTIVE;
    msg.freq = freq;

    m_pClientSocket->send((char*)&msg, sizeof(msg));

}

void  GNSx30Proxy::setCOMStandbyFrequency(unsigned long freq)
{
    FreqInfo msg;

    msg.msgType = MGS_COM_STANDBY;
    msg.freq = freq;

    m_pClientSocket->send((char*)&msg, sizeof(msg));

}

//NAV functions
void  GNSx30Proxy::setNAVActiveFrequency(unsigned long freq)
{
    FreqInfo msg;

    msg.msgType = MGS_NAV_ACTIVE;
    msg.freq = freq;

    m_pClientSocket->send((char*)&msg, sizeof(msg));

}

void  GNSx30Proxy::setNAVStandbyFrequency(unsigned long freq)
{
    FreqInfo msg;

    msg.msgType = MGS_NAV_STANDBY;
    msg.freq = freq;

    m_pClientSocket->send((char*)&msg, sizeof(msg));

}

