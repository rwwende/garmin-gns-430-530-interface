#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "gnsTypes.h"
#include "sharedstruct.h"
#include "IOP_SIM_hooks.h"
#include "gdi32_hooks.h"
#include "cdp_com_box_sim_hooks.h"
#include "cdp_vloc_box_sim_hooks.h"
#include "log.h"
#include "resource.h"

static SharedStruct<GNSIntf>* pShared;


bool hook_gnsx30(HMODULE hModule)
{
    HBITMAP hBmp;
    BITMAP	bezelBMP;
    bool res = true;

    HMODULE h_sys_resources = GetModuleHandle("sys_resource.dll");

    pShared = new SharedStruct<GNSIntf>(SHMEM_NAME, false, 0);

    GNSIntf* pIntf =  pShared->get();


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

        pIntf->bezel_lcd_top = 27;
        pIntf->bezel_lcd_left = 75;
        pIntf->bezel_lcd_width = 320;
        pIntf->bezel_lcd_height = 234;


    }

    ::GetObject (hBmp, sizeof (bezelBMP), &bezelBMP);

    pIntf->bezel_width = bezelBMP.bmWidth;
    pIntf->bezel_height = bezelBMP.bmHeight;

    unsigned char* pData = (unsigned char*)pIntf->Bezel_data;
    unsigned char* pBmpData = (unsigned char*)bezelBMP.bmBits;

    memcpy(pIntf->Bezel_data, bezelBMP.bmBits, bezelBMP.bmWidthBytes*bezelBMP.bmHeight);

    pIntf->BezelUpdated = true;
    

    logMessageEx("--- bezelBMP=%dx%d widthBytes=%d", bezelBMP.bmWidth, bezelBMP.bmHeight, bezelBMP.bmWidthBytes);

    //hook the IOP_SIM
    hook_IOP_SIM(pIntf);
	//hook the cdp_com_box_sim COM1
	hook_cdp_com_box_sim(pIntf);
	//hook the cdp_vloc_box_sim NAV1
	hook_cdp_vloc_box_sim(pIntf);
	//hook the GDI
    hook_gdi(pShared);


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


