#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "hooks.h"
#include "gnsTypes.h"
#include "sharedstruct.h"
#include "iopsimhooks.h"
#include "gdi32_hooks.h"
#include "comhooks.h"
#include "navhooks.h"
#include "krnlsimhooks.h"
#include "log.h"
#include "resource.h"
#include "udpsocketthread.h"
#include "udpsocket.h"
//#pragma warning TEST
#include "testhooks.h"


Hooks* Hooks::m_gInstance = NULL;


void UdpDataCallback(void* pData, int dataSize, void* context)
{
    Hooks* pHooks = (Hooks*)context;
    pHooks->processUdpData(pData, dataSize);

}



Hooks::Hooks()
{


}

Hooks::~Hooks()
{


}

Hooks* Hooks::instanace()
{
    if(NULL == m_gInstance)
    {
        m_gInstance = new Hooks();
    }

    return m_gInstance;


}



bool Hooks::hookGnsx30(HMODULE hModule)
{
    HBITMAP hBmp;
    BITMAP	bezelBMP;
    bool res = true;

    HMODULE h_sys_resources = GetModuleHandle("sys_resource.dll");

    m_pShared = new SharedStruct<GNSIntf>(SHMEM_NAME, false, 0);

    GNSIntf* pIntf =  m_pShared->get();

    //Start the serverThread
    m_pServerSocketThread = new UdpSocketThread(pIntf->garminTrainerPort);
    m_pServerSocketThread->create();
    m_pServerSocketThread->setCallback(UdpDataCallback, this);
    m_pServerSocketThread->resume();
    

    m_pClientSocket  = new UdpSocket();
    m_pClientSocket->openForSending("127.0.0.1", pIntf->proxyPort);


    pIntf->bezel_width = 0;
    pIntf->bezel_height = 0;

    pIntf->bezel_lcd_top = 0;
    pIntf->bezel_lcd_left = 0;
    pIntf->bezel_lcd_width = 0;
    pIntf->bezel_lcd_height = 0;

    if(TYPE_GNS430 == pIntf->gnsType)
    {
        //Read the bezel bitmaps from the Gns exe
        hBmp = (HBITMAP) ::LoadImage (hModule,
            MAKEINTRESOURCE(IDB_BEZEL_430), IMAGE_BITMAP, 0, 0,
            LR_CREATEDIBSECTION);

        pIntf->bezel_lcd_top = 26;
        pIntf->bezel_lcd_left = 110;
        pIntf->bezel_lcd_width = 240;
        pIntf->bezel_lcd_height = 128;


    }else  if(TYPE_GNS530 == pIntf->gnsType)
    {
        //Read the bezel bitmaps from the Gns exe
        hBmp = (HBITMAP) ::LoadImage (hModule,
            MAKEINTRESOURCE(IDB_BEZEL_530), IMAGE_BITMAP, 0, 0,
            LR_CREATEDIBSECTION);
        ::GetObject (hBmp, sizeof (bezelBMP), &bezelBMP);

        pIntf->bezel_lcd_left = 75;
        pIntf->bezel_lcd_top = 27;
        pIntf->bezel_lcd_width = 320;
        pIntf->bezel_lcd_height = 234;


    }

    ::GetObject (hBmp, sizeof (bezelBMP), &bezelBMP);

    pIntf->bezel_width = bezelBMP.bmWidth;
    pIntf->bezel_height = bezelBMP.bmHeight;

    //unsigned long* pData = (unsigned long*)pIntf->Bezel_data;
    //unsigned char* pBmpData = (unsigned char*)bezelBMP.bmBits;

    unsigned long* lData = (unsigned long*)bezelBMP.bmBits;
    int i=0;
    for(int y = 0; y < bezelBMP.bmHeight; y++)
    {
        for(int x = 0; x < bezelBMP.bmWidth; x++)
        {
            //logMessageEx("--- [%d][%d] = %08x", y, x, lData[y*bezelBMP.bmWidth + x]);;
            //replace ARGB (ffff00ff) with transparent color
            pIntf->Bezel_data[i] = 0xffff00ff == lData[i]?0x00000000:lData[i];
            i++;
        }
    }
    //memcpy(pIntf->Bezel_data, bezelBMP.bmBits, bezelBMP.bmWidthBytes*bezelBMP.bmHeight);

    pIntf->BezelUpdated = true;
    

    logMessageEx("--- bezelBMP=%dx%d widthBytes=%d", bezelBMP.bmWidth, bezelBMP.bmHeight, bezelBMP.bmWidthBytes);

#if 1
    //hook the krlnsim.dll
    //KrlnsimHooks::instanace()->hook(m_pShared);
    //hook the IOP_SIM
    IopSimHooks::instanace()->hook(m_pShared);
    //hook the cdp_com_box_sim COM
    ComHooks::instanace()->hook(m_pShared);
    //hook the cdp_vloc_box_sim NAV
    NavHooks::instanace()->hook(m_pShared);
	//hook the GDI
    hook_gdi(m_pShared);

#else

#pragma warning TESTHOOKS
    TestHooks::instanace()->hook(m_pShared);
#endif


	//Fix the SIMULATING in the sys_resources
	{
		//SIMULATING string begins at address 10038444 pointed from address 1002f4da
		char* simulating_addr =  (char*)h_sys_resources + 0x38444;
		strcpy(simulating_addr, "3D DIFF NAV");
	}
	
#if 0 // needed for the autopilot untill I create my own file mapping or simething else
	//Avoid starting of the hsi400wx.exe
	{
		//hsi400wx string begins at address 0x0040e678
		char* hsi400wx_addr =  (char*)h_gns530 + 0xe678;
		strcpy(hsi400wx_addr, "xxxxxx");
	}
#endif
	
	return res;
}


bool Hooks::notifyFreqencyChange(FreqInfo* pInfo)
{
    bool res = true;
    int r = m_pClientSocket->send((char*)pInfo, sizeof(FreqInfo));
    if(r != sizeof(FreqInfo))
    {
        res = false;
    }

    logMessageEx("--- notifyFreqencyChange[%d] %d -> %d",pInfo->msgType,  pInfo->freq,  res);

    return res;
}


void Hooks::processUdpData(void* pData, int dataSize)
{
    if(dataSize >0)
    {
        unsigned char msgType = ((unsigned char*)pData)[0];
        switch(msgType)
        {
        case MSG_COM_ACTIVE:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                ComHooks::instanace()->setActiveFrequency(pFreqInfo->freq);
                //logMessageEx("--- Hooks::processUdpData MSG_COM_ACTIVE pFreqInfo->freq %d", pFreqInfo->freq);
                break;
            }
        case MSG_COM_STANDBY:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                ComHooks::instanace()->setStandbyFrequency(pFreqInfo->freq);
                //logMessageEx("--- Hooks::processUdpData MSG_COM_STANDBY pFreqInfo->freq %d", pFreqInfo->freq);

                break;
            }
        case MSG_NAV_ACTIVE:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                NavHooks::instanace()->setActiveFrequency(pFreqInfo->freq);
                //logMessageEx("--- Hooks::processUdpData MSG_NAV_ACTIVE pFreqInfo->freq %d", pFreqInfo->freq);
                break;
            }
        case MSG_NAV_STANDBY:
            {
                FreqInfo* pFreqInfo = (FreqInfo*)pData;
                NavHooks::instanace()->setStandbyFrequency(pFreqInfo->freq);
                //logMessageEx("--- Hooks::processUdpData MSG_NAV_STANDBY pFreqInfo->freq %d", pFreqInfo->freq);

                break;
            }
        case MSG_GPS_INFO:
            {
                GPSInfo* pGPSInfo = (GPSInfo*)pData;
                IopSimHooks::instanace()->setGPSInfo(pGPSInfo->latitude,
                                                     pGPSInfo->longitude,
                                                     pGPSInfo->speed,
                                                     pGPSInfo->heading,
                                                     pGPSInfo->verticalSpeed,
                                                     pGPSInfo->altitude);
                //logMessageEx("--- Hooks::processUdpData MSG_GPS_INFO");

                break;
            }


        }
    }
}

