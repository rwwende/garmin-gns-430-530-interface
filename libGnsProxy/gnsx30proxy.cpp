#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "gnsx30proxy.h"
#include "injector.h"
#include "crcfile.h"
#include "log.h"
#include "config.h"


GNSx30Proxy::GNSx30Proxy()
{

    
	m_pCRCFile = new CRCFile();

	m_win = NULL;

}


GNSx30Proxy::~GNSx30Proxy()
{
	delete m_pCRCFile;
}



int GNSx30Proxy::initialize(/*int type,  GNSInfo* pInfo*/)
{
	int res = 0;
	
	m_win = NULL; 

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
			res = -1;
			return res;
		}

		lResult = RegQueryValueEx(hKey, "Path", 0, &dwType,  (LPBYTE)trainerpath, &dwSize);
		RegCloseKey(hKey);


		if(ERROR_SUCCESS != lResult)
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error reading key " G530SIMEXE_PATH);

			res = -1;
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

			   res = -1;
			   return res;
		   }
	   }

		// Now calculate the CRC
		if(!m_pCRCFile->calcCRC(m_trainter_exe, &CRCVal))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error calculating CRC");
			res = -1;
			return res;
		}

		if(GNSX30EXE_CRC != CRCVal)
		{

			// Invalid trainter version
			logMessageEx("??? GNSx30Proxy::Initialize Error checking CRC");

			//Delete the copied file
			//DeleteFile(m_trainter_exe);
			res = -1;
			return res;
		}

	}////find the trainer path and copy the G530SIM.exe to th appropriate executable

	//Create the registry keys for the trainer
	{

		HKEY hKey;
		DWORD dwSize = MAX_PATH;
		DWORD dwValue;
		DWORD dwDisposition;


		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
			   logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL);

			   res = -1;
			   return res;
		}

		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_SETTINGS, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
     		   logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL_SETTINGS);

			   res = -1;
			   return res;
		}

		//Create the trainer settings values
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_530AWT, strlen(GNS_530AWT) + 1))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key CDUType");

			   res = -1;
			   return res;
		}


		RegFlushKey(hKey);
		RegCloseKey(hKey);


		if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_WINDOW, 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
			)
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error creating key " GARMIN_INTERNATIONAL_WINDOW);

			   res = -1;
			   return res;
		}


        //Create the window position values
		dwValue = 5000;
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "x1", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
		{
				logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);

			   res = -1;
			   return res;
		}
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "x2", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
			   res = -1;
			   return res;
		}
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "y1", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
			   res = -1;
			   return res;
		}
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "y2", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
		{
			logMessageEx("??? GNSx30Proxy::Initialize Error writing key " GARMIN_INTERNATIONAL_WINDOW);
			   res = -1;
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

	return res;
}

int GNSx30Proxy::terminate()
{
	int res = 0;

	delete 	m_pShared; 
    m_pvData = NULL;


	return res;

}


int GNSx30Proxy::open(int gnsType)
{
	int res = 0;


    HKEY hKey;
    DWORD dwDisposition;

    m_pvData->gnsType = gnsType;

    if (ERROR_SUCCESS!= RegCreateKeyEx(HKEY_CURRENT_USER, GARMIN_INTERNATIONAL_SETTINGS, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,&dwDisposition)
        )
    {
        logMessageEx("??? GNSx30Proxy::open Error creating key " GARMIN_INTERNATIONAL_SETTINGS);

        res = -1;
        return res;
    }

    if(TYPE_GNS430 == gnsType)
    {

        //Create the trainer settings values
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_430AWT, strlen(GNS_430AWT) + 1))
        {
            logMessageEx("??? GNSx30Proxy::open Error writing key CDUType");

            res = -1;
            return res;
        }
    }else if(TYPE_GNS530 == gnsType)
    {
        //Create the trainer settings values
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "CDUType", 0, REG_SZ, (LPBYTE)GNS_530AWT, strlen(GNS_530AWT) + 1))
        {
            logMessageEx("??? GNSx30Proxy::open Error writing key CDUType");

            res = -1;
            return res;
        }
    }

    
    RegFlushKey(hKey);
    RegCloseKey(hKey);



	memset(m_pvData->LDC_data,0x00, OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT*4);

    startAndInject(m_trainter_exe, m_trainter_path, m_interface_lib);
    




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
		res = -1;
		return res;

	}


	return res;

}

int GNSx30Proxy::close()
{
	int res = 0;
	LRESULT r;

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

