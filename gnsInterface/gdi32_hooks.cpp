#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "apihijack.h"
#include "gdi32_hooks.h"
#include "log.h"

static GNSIntf* pIntf = NULL;
static HANDLE memMap = NULL;

//GDI
HBITMAP my_bitmap = NULL;
HDC		top_dc = NULL; // this is the DC of the LCD window.BitBlt to this D will be interecpted
HDC		my_dc = NULL;

typedef int (WINAPI *ChoosePixelFormat_t)(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
ChoosePixelFormat_t		g_ChoosePixelFormat;
int WINAPI my_ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
	int res =0;

	res = g_ChoosePixelFormat(hdc, ppfd);


	top_dc = hdc; 
	
    //logMessageEx("--- my_ChoosePixelFormat %08x -> %d", hdc, res);

	return res;
}

typedef HDC (WINAPI *CreateCompatibleDC_t)(HDC hdc);
CreateCompatibleDC_t	g_CreateCompatibleDC;
HDC WINAPI my_CreateCompatibleDC(HDC hdc)
{
	HDC res;
	
    res = g_CreateCompatibleDC(hdc);

	return res;
}


typedef HGDIOBJ (WINAPI *SelectObject_t)(HDC hdc, HGDIOBJ hgdiobj); 
SelectObject_t	g_SelectObject;
HGDIOBJ WINAPI my_SelectObject(HDC hdc, HGDIOBJ hgdiobj)
{
    HGDIOBJ res;

    res = g_SelectObject(hdc, hgdiobj);

    return res;
}


typedef BOOL (WINAPI *BitBlt_t)(HDC hdcDest,int nXDest,int nYDest,int nWidth,int nHeight,HDC hdcSrc,int nXSrc,int nYSrc,DWORD dwRop);
BitBlt_t	g_BitBlt;
BOOL WINAPI my_BitBlt(HDC hdcDest,int nXDest,int nYDest,int nWidth,int nHeight,HDC hdcSrc,int nXSrc,int nYSrc,DWORD dwRop)
{
	BOOL res;

    if(hdcDest == top_dc)
    {
        if(NULL == my_bitmap)
        {

            pIntf->lcd_width = nWidth;
            pIntf->lcd_height = nHeight;

            my_dc = CreateCompatibleDC(top_dc);

            
            BITMAPINFOHEADER BMI;
            // Fill in the header info.
            BMI.biSize = sizeof(BITMAPINFOHEADER);
            BMI.biWidth = nWidth;
            BMI.biHeight = nHeight;
            BMI.biPlanes = 1;
            BMI.biBitCount = 24; //32
            BMI.biCompression = BI_RGB;   // No compression
            BMI.biSizeImage = 0;
            BMI.biXPelsPerMeter = 0;
            BMI.biYPelsPerMeter = 0;
            BMI.biClrUsed = 0;           // Always use the whole palette.
            BMI.biClrImportant = 0;

            BYTE * pSrcBits;
            // Create DIB section in shared memory
            my_bitmap = CreateDIBSection (my_dc, (BITMAPINFO *)&BMI,
                DIB_RGB_COLORS, (void **)&pSrcBits, memMap, 0);


            SelectObject(my_dc, my_bitmap);

            logMessageEx("--- my_BitBlt LCD bitmap= %dx%d", nWidth, nHeight);
            

        }

        res = g_BitBlt(my_dc, nXDest, nYDest, nWidth,  nHeight,  hdcSrc, nXSrc, nYSrc, dwRop);
        //res = g_BitBlt(hdcDest, nXDest, nYDest, nWidth,  nHeight,  hdcSrc, nXSrc, nYSrc, dwRop);

        pIntf->LCDUpdated = true;

    }else
    {
        res = g_BitBlt(hdcDest, nXDest, nYDest, nWidth,  nHeight,  hdcSrc, nXSrc, nYSrc, dwRop);
        //logMessageEx("--- my_BitBlt hdcDest=%08x, nXDest=%d, nYDest=%d, nWidth=%d,  nHeight=%d,  hdcSrc=%08x, nXSrc=%d, nYSrc=%d, dwRop=%08x", hdcDest, nXDest, nYDest, nWidth,  nHeight,  hdcSrc, nXSrc, nYSrc, dwRop);
    
    }

	return res;
}


typedef HBITMAP  (WINAPI *CreateCompatibleBitmap_t)(HDC hdc, int nWidth, int nHeight);
CreateCompatibleBitmap_t	g_CreateCompatibleBitmap;
HBITMAP  WINAPI my_CreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight)
{
	HBITMAP  res;

    res = g_CreateCompatibleBitmap(hdc, nWidth, nHeight);
    logMessageEx("--- g_CreateCompatibleBitmap hdc=%08x, nWidth=%d, nHeight=%d", hdc, nWidth, nHeight);

	return res;
}



typedef int (WINAPI *StretchDIBits_t)(HDC hdc,int XDest,int YDest,int nDestWidth,int nDestHeight,int XSrc,int YSrc,int nSrcWidth,int nSrcHeight,const VOID *lpBits,const BITMAPINFO *lpBitsInfo,UINT iUsage,DWORD dwRop);
StretchDIBits_t g_StretchDIBits;
int WINAPI my_StretchDIBits(HDC hdc,int XDest,int YDest,int nDestWidth,int nDestHeight,int XSrc,int YSrc,int nSrcWidth,int nSrcHeight,const VOID *lpBits,const BITMAPINFO *lpBitsInfo,UINT iUsage,DWORD dwRop)
{
	int res;
    res = g_StretchDIBits(hdc,XDest,YDest,nDestWidth,nDestHeight,XSrc,YSrc,nSrcWidth,nSrcHeight,lpBits,lpBitsInfo,iUsage,dwRop);
    //logMessageEx("--- g_StretchDIBits hdc=%08x,XDest=%d,YDest=%d,nDestWidth=%d,nDestHeight=%d,XSrc=%d,YSrc=%d,nSrcWidth=%d,nSrcHeight=%d,lpBits=%08x,lpBitsInfo=%08x,iUsage=%d,dwRop=%08x", 
    //    hdc,XDest,YDest,nDestWidth,nDestHeight,XSrc,YSrc,nSrcWidth,nSrcHeight,lpBits,lpBitsInfo,iUsage,dwRop);
	return res;
}



static SDLLHook krnlsimGDIHook = 
{
	"gdi32.dll",
	false, NULL, // Default hook disabled, NULL function pointer.
	{
        { "ChoosePixelFormat", my_ChoosePixelFormat},
		//{ "CreateCompatibleDC", my_CreateCompatibleDC},
		{ "BitBlt", my_BitBlt},
		//{ "CreateCompatibleBitmap", my_CreateCompatibleBitmap},
		//{ "SelectObject", my_SelectObject},
		//{ "StretchDIBits", my_StretchDIBits},
		{ NULL, NULL }
	}
};


int hook_gdi(SharedStruct<GNSIntf>* pSharedStruct)
{
	int res = 0;
	int i;
	
    
	pIntf = pSharedStruct->get();
    memMap = pSharedStruct->getHandle();

    //logMessageEx("--- hook_gdi=%08x", pIntf);


	HMODULE h_krnlsim = GetModuleHandle("krnlsim.dll");


	i = 0;
	HookAPICallsMod(&krnlsimGDIHook, h_krnlsim);
	g_ChoosePixelFormat = (ChoosePixelFormat_t)krnlsimGDIHook.Functions[i++].OrigFn;
	//g_CreateCompatibleDC = (CreateCompatibleDC_t)krnlsimGDIHook.Functions[i++].OrigFn;
	g_BitBlt = (BitBlt_t)krnlsimGDIHook.Functions[i++].OrigFn;
	//g_CreateCompatibleBitmap = (CreateCompatibleBitmap_t)krnlsimGDIHook.Functions[i++].OrigFn;
	//g_SelectObject = (SelectObject_t)krnlsimGDIHook.Functions[i++].OrigFn;
	//g_StretchDIBits = (StretchDIBits_t)krnlsimGDIHook.Functions[i++].OrigFn;

	

	return res;
}


