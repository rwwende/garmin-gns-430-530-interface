#ifndef __GNS_TYPES__
#define __GNS_TYPES__

#include <windows.h>
#include "config.h"

enum MSG_TYPE 
{
    MSG_COM_ACTIVE = 0,
    MSG_COM_STANDBY,
    MSG_NAV_ACTIVE,
    MSG_NAV_STANDBY,
    MSG_GPS_INFO
};


typedef struct _FreqInfo
{
    unsigned char msgType;
    unsigned long freq;
}FreqInfo;

typedef struct _GPSInfo
{
    unsigned char msgType;
    double		  latitude; //radians
    double		  longitude; //radians
    float		  speed; //m/s
    float		  heading; //0=> pi, -pi=>0
    float		  verticalSpeed; //0=>
    float		  altitude; // m
}GPSInfo;







//0 - 0D 00 3F 00 
//4 - 05 00 short hfom
//6 - 64 00 ??
//8 - 07 00  short vfom
//10 - 00 00 00 00 00 00 
//16 - 0A D7 F3 41 (altitude) float
//20 - 99 FB 3A 43 (speed) float
//24 - 3F 68 4C 3F (current heading) float (0=> pi, -pi=>0)
//28 - 00 00 00 00 (vertical speed)
//32 - D2 EB 9A 42 (altitude2= altitude1 + xxx ) (float)
//36 - 06 A1 CA 08 53 C4 EC 3F (double N xx.yy.zz)
//44 - EB 0C DD B3 59 64 B8 3F (double E xx.yy.zz)
//52 - 0C (month) 
//53 - 1B (may) 
//54 - D9 07 (year) 
//56 - 0B 00 (hour)
//58 - 35 (min) 
//59 - 16 (sec)

#pragma pack(push, 1)

typedef struct _SharedObjShr
{
	//0 -- 13
	unsigned char unknown000[14];
	//14 - 15
	unsigned short cdi_mode; //??
	//16 - 17
	unsigned short vcdi; //??
	//18 - 19
	unsigned short lcdi; //??
	//20 -23
	float	current_altitude;
	//24 -27
	unsigned char unknown100[4];
	//28 -31
	float	current_speed;
	//32 -35
	float cdi_horizontal_offtrack;
	//36 -39
	unsigned char unknown200[4];
	//40 -43
	float dtk;
	//44 -47
	unsigned char unknown300[4];
	//48 - 51
	float current_hdg;
	//52 -55
	//unsigned char unknown400[4];
	unsigned long fpl_enabled;

	//56 - 59
	float unknown500;
	//60 -63
	unsigned char unknown600[4];
	//64-67
	float	requested_altitude;
	//68-71
	float	obs;
	//72-75
	float	requested_speed;
	//76-79
	float	requested_hdg; // only in hdg mode



}SharedObjShr;


#pragma pack(pop)


typedef struct _GNSIntf
{
	// must be first
	unsigned long LDC_data[OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT]; // this should eb the exact size as the BITMAP_WIDTH in the info
    unsigned long lcd_width;
    unsigned long lcd_height;
    bool          LCDUpdated;

    unsigned long bezel_width;
    unsigned long bezel_height;

    unsigned long bezel_lcd_left;
    unsigned long bezel_lcd_top;
    unsigned long bezel_lcd_width;
    unsigned long bezel_lcd_height;

    unsigned long Bezel_data[OFFSCREEN_BUFFER_WIDTH*OFFSCREEN_BUFFER_HEIGHT]; // this should eb the exact size as the BITMAP_WIDTH in the info
    bool          BezelUpdated;

    
    unsigned long gnsType;
    unsigned short garminTrainerPort;
    unsigned short proxyPort;



	
	//Auto pilot 
	//Override xplane gps, will be set to 1 when valid desired track is set to TrainerShrObj
	bool	 override_gps;
	//desired track
	float dtk;

	unsigned short vcdi; //2048 is in the middle
	unsigned short lcdi; 
	float			cdi_horizontal_offtrack;

	
}GNSIntf;



#endif __GNS_TYPES__