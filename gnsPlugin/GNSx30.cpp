#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"
#include "IniReader.h"
#include "log.h"
#include "gnsx30proxy.h"

/* defines */
#define PI					(3.14159265f)
#define MAX_FILE_NAME_LEN	512
#define PLUGIN_NAME			"gnsx30"

#define INI_SECTION_POSITION	"position"
//#define INI_SECTION_NETWORK	"network"

/* Forward declarations */
float	myFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * inRefcon);
void	myMenuHandlerCallback(void* inMenuRef, void* inItemRef);
void	myHandleKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus);
int		myHandleMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void*inRefcon);
void	myDrawWindowCallback(XPLMWindowID inWindowID, void* inRefcon);
int		myDrawGNSCallback(XPLMDrawingPhase inPhase, int inIsBefore, void *inRefcon);
void	motionCommandsThreadFunc(LPVOID lpvParam);

//Types
typedef struct _region
{
	//bottom left
	float x1; 
	float y1;
	//top right
	float x2; 
	float y2;


}region;

//Utiliti functions
int		loadSettings();
int		saveSettings();
int		openLogFile();
int		loadGNS();
int		unloadGNS();
int		initDataRefs();
int		createMenus();
int		initTextures();
int		createGNSWindow();
int		destroyGNSWindow();
int		initClickRegions();
int		coordInRect(int x, int y, region* reg);


region regionLCD;



/* Global variables */
//test
int mouse_x, mouse_y;



//textures
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_CLAMP_TO_EDGE 0x812F 
#define GL_CLAMP_TO_BORDER 0x812D 
#define TEX_LCD		0
#define TEX_BEZEL	1
XPLMTextureID gTexture[2]; //0-gns_lcd, 1-gns_bezel

GNSx30Proxy     gGNSx30Proxy;
GNSIntf*        gpGNSIntf; 
bool			gnsOpened=false;
bool			gnsVisible=false;

//autopilot
bool			autopilotConnected;
float			dtk;
unsigned short   lcdi;
unsigned short   vcdi;





int			gPluginEnabled = 0;
float		gUpdateInterval = (float)0.01;

//Menu
#define MENU_OPEN	1
#define MENU_SHOW	2
#define MENU_HIDE	3
#define MENU_CLOSE	4
#define MENU_SIZE_INC	5
#define MENU_SIZE_DEC	6
#define MENU_SIZE_RESTORE	7
#define MENU_AUTOPILOT_CONNECTED 8

#define MENU_NAME	"GNS 430/530 WAAS"
#define SUBMENU_NAME	"GNS 530W WAAS"

int				myMenu;
XPLMMenuID		mySubMenu;
int				myMenuItemWAASAutoipilot;


//Window
#define ZOOM_STEP (0.05f)
XPLMWindowID	    gWindow = NULL;
int g_pos_x;
int g_pos_y;
float g_zoom;

//sim/flightmodel/position/latitude	double	n	degrees	The latitude of the aircraft
//sim/flightmodel/position/longitude	double	n	degrees	The longitude of the aircraft
//sim/flightmodel/position/magpsi	float	n	degrees	The magnetic heading of the aircraft.
//sim/flightmodel/position/groundspeed	float	n	meters/sec	The ground speed of the aircraft
//sim/flightmodel/position/elevation	double	n	meters	The elevation above MSL of the aircraft
//sim/flightmodel/position/vh_ind	float	n	m/s	VVI (vertical velocity in meters per second)
//sim/cockpit2/radios/actuators/com1_standby_frequency_hz	int	y	10hertz	Com radio 1 standby frequency, hz
//sim/cockpit2/radios/actuators/com1_frequency_hz	int	y	10hertz	Com radio 1 frequency, hz
//sim/cockpit2/radios/actuators/nav1_standby_frequency_hz	int	y	10hertz	Nav radio 1 standby frequency, hz
//sim/cockpit2/radios/actuators/nav1_frequency_hz	int	y	10hertz	Nav radio 1 frequency, hz
//GPS
//sim/operation/override/override_gps
//sim/cockpit/autopilot/heading_mag
//sim/cockpit/radios/gps_fromto	int	y	enum	Whether we are heading to or from (or over) our nav2 beacon - pilot.
//sim/cockpit/radios/gps_course_degtm	float	y	degm	The localizer course for GPS or tuned in radial for a VOR (Magnetic, new) - pilot - use override_gps
//sim/cockpit/radios/gps_hdef_dot	float	y	prcnt	The deflection from the aircraft to the tuned in course in 'dots' on a VOR compass - pilot.  override_gps
//sim/cockpit/radios/gps_vdef_dot	float	y	prcnt	The deflection from the aircraft to the tuned in glide slope in dots on an ILS gauge - pilot. override_gps
//sim/cockpit/radios/gps_has_glideslope	int	y	int	does the GPS provde vertical guidance?  Write with override_gps
//sim/cockpit/radios/gps_slope_degt	float	y	deg	The glide slope slope for the GPS.  Writable with override_gps.
//sim/cockpit/radios/gps_dme_dist_m	float	y	meters	Our distance in meters from the beacon tuned in on the GPS.  override_gps
//sim/cockpit2/radios/actuators/HSI_source_select_pilot	int	y	enum	HSI source to display: 0 for Nav1, 1 for Nav2, 2 for GPS.
//sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot	float	y	degrees_magnetic	HSI OBS (pilot side) selection, in degrees magnetic.
//Autopilot
//sim/operation/override/override_autopilot	int	y	boolean	Override the autopilot's brains
//sim/cockpit/autopilot/autopilot_state	int	y	flags	Various autopilot engage modes, etc.  See docs for flags

//GPS/NAV1
//sim/operation/override/override_nav1_needles	int	y	boolean	Override nav1 receiver
//sim/cockpit/radios/nav1_hdef_dot	float	y	prcnt	The deflection from the aircraft to the tuned in course in 'dots' on a VOR compass - pilot.  override_navneedles
//sim/cockpit/radios/nav1_vdef_dot	float	y	prcnt	The deflection from the aircraft to the tuned in glide slope in dots on an ILS gauge - pilot.  override_navneedles
//sim/cockpit/radios/nav1_fromto	int	y	enum	Whether we are heading to or from (or over) our nav1 beacon - pilot.

XPLMDataRef		g_latitude_ref = NULL;
XPLMDataRef		g_longitude_ref = NULL;
XPLMDataRef		g_heading_ref = NULL;
XPLMDataRef		g_groundspeed_ref = NULL;
XPLMDataRef		g_elevation_ref = NULL;
XPLMDataRef		g_vert_speed_ref = NULL;
XPLMDataRef		g_com1_standby_ref = NULL;
XPLMDataRef		g_com1_active_ref = NULL;
XPLMDataRef		g_nav1_standby_ref = NULL;
XPLMDataRef		g_nav1_active_ref = NULL;
XPLMDataRef		g_override_gps_ref = NULL;
//XPLMDataRef		g_autopilot_mag_heading_ref = NULL;
XPLMDataRef		g_gps_fromto_ref = NULL;
XPLMDataRef		g_gps_course_degtm_ref = NULL;
XPLMDataRef		g_gps_hdef_dot_ref = NULL;
XPLMDataRef		g_gps_vdef_dot_ref = NULL;
//XPLMDataRef		g_gps_has_glideslope_ref = NULL;
//XPLMDataRef		g_gps_slope_degt_ref = NULL;
//XPLMDataRef		g_gps_dme_dist_m_ref = NULL;
//XPLMDataRef		g_HSI_source_select_pilot_ref = NULL;
XPLMDataRef		g_hsi_obs_deg_mag_pilot_ref = NULL;
XPLMDataRef		g_override_autopilot_ref = NULL;
XPLMDataRef		g_autopilot_state_ref = NULL;

/*
XPLMDataRef		g_override_nav1_needles_ref = NULL;
XPLMDataRef		g_nav1_hdef_dot_ref = NULL;
XPLMDataRef		g_nav1_vdef_dot_ref = NULL;
XPLMDataRef		g_nav1_fromto_ref = NULL;
*/



double			latitude;
double			longitude;
float			heading;
float			groundspeed;
float			elevation;
float			vert_speed;
int				override_gps;
//float			autopilot_mag_heading;
int				gps_fromto;
float			gps_course_degtm;
float			gps_hdef_dot;
float			gps_vdef_dot;
//int				gps_has_glideslope;
//float			gps_slope_degt;
//float			gps_dme_dist_m;
//int				HSI_source_select_pilot;
float			hsi_obs_deg_mag_pilot;
int				override_autopilot;
int				autopilot_state;
/*
int				override_nav1_needles;
float			nav1_hdef_dot;
float			nav1_vdef_dot;
int				nav1_fromto;
*/




XPLMDataRef myXPLMFindDataRef(const char * inDataRefName)
{
	
	XPLMDataRef DataRef=XPLMFindDataRef(inDataRefName);
	if (DataRef==NULL)
	{
		logMessageEx("*** Cannot find data reference [%s]", inDataRefName);
	}
	
	return DataRef;
}

void myMenuHandlerCallback(void* inMenuRef, void* inItemRef)
{
	//	 This is our handler for the menu item.  Our inItemRef is the refcon
	//	 we registered in our XPLMAppendMenuItem calls.  It is either 0 or
	//	 1 depending on which menu item is picked. 
	long val = (long) inItemRef;

	int outLeft;
	int outTop;
	int outRight;
	int	outBottom;

	if (MENU_OPEN == val)
	{
		if(!gnsOpened)
		{
			createGNSWindow();
			gnsVisible = true;
		}
	}
	else if (MENU_SHOW == val)
	{
		if(gnsOpened)
		{
			gnsVisible = true;
		}else
		{
			gnsVisible = false;
		}
	}
	else if(MENU_HIDE == val)
	{
		if(gnsOpened)
		{
			gnsVisible = false;
		}
	}
	else if(MENU_CLOSE == val)
	{
		if(gnsOpened)
		{
			gnsVisible = false;
			XPLMCheckMenuItem(mySubMenu, myMenuItemWAASAutoipilot,xplm_Menu_Unchecked);
			autopilotConnected = false;
			destroyGNSWindow();	

		}
	}
	else if(MENU_SIZE_INC == val)
	{
        
		if(gnsVisible)
		{
			g_zoom+=ZOOM_STEP;
			XPLMGetWindowGeometry(gWindow, &outLeft, &outTop, &outRight, &outBottom); 
			outTop =  outBottom + (int)(gpGNSIntf->bezel_height*g_zoom);
			outRight =  outLeft + (int)(gpGNSIntf->bezel_width*g_zoom);
			XPLMSetWindowGeometry(gWindow, outLeft, outTop, outRight, outBottom); 
			initClickRegions();
		}
	}
	else if(MENU_SIZE_DEC == val)
	{
		if(gnsVisible)
		{
			g_zoom-=ZOOM_STEP;
			XPLMGetWindowGeometry(gWindow, &outLeft, &outTop, &outRight, &outBottom); 
			outTop =  outBottom + (int)(gpGNSIntf->bezel_height*g_zoom);
			outRight =  outLeft + (int)(gpGNSIntf->bezel_width*g_zoom);
			XPLMSetWindowGeometry(gWindow, outLeft, outTop, outRight, outBottom); 
			initClickRegions();
		}
	}
	else if(MENU_SIZE_RESTORE == val)
	{
		if(gnsVisible)
		{
			g_zoom=1.0;
			XPLMGetWindowGeometry(gWindow, &outLeft, &outTop, &outRight, &outBottom); 
			outTop =  outBottom + (int)(gpGNSIntf->bezel_height*g_zoom);
			outRight =  outLeft + (int)(gpGNSIntf->bezel_width*g_zoom);
			XPLMSetWindowGeometry(gWindow, outLeft, outTop, outRight, outBottom); 
			initClickRegions();
		}
	}
	else if(MENU_AUTOPILOT_CONNECTED == val)
	{
		if(gnsOpened)
		{
			int state;

			XPLMCheckMenuItemState(mySubMenu, myMenuItemWAASAutoipilot, &state);
			if(xplm_Menu_Unchecked== state)
			{
				XPLMCheckMenuItem(mySubMenu, myMenuItemWAASAutoipilot,xplm_Menu_Checked);
				autopilotConnected = true;
			}else
			{
				XPLMCheckMenuItem(mySubMenu, myMenuItemWAASAutoipilot,xplm_Menu_Unchecked);
				autopilotConnected = false;
			}

		}
	}



	
}


// This callback does the work of drawing our window once per sim cycle each time
// it is needed.  It dynamically changes the text depending on  MOTI DATA
// Note that we don't have to tell X-Plane to redraw us when our text
// changes; we are redrawn by the sim continuously.

void myDrawWindowCallback(XPLMWindowID inWindowID, void* inRefcon)
{
    // Empty
    return;
}                                   


// MyHandleKeyCallback
// Our key handling callback does nothing in this plugin.  This is ok; 
// we simply don't use keyboard input.
void myHandleKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus)
{
    // Empty
    return;

}                                   

// MyHandleMouseClickCallback
// Our key handling callback does nothing in this plugin.  This is ok; 
// we simply don't use keyboard input.
// Returning 1 tells X-Plane that we 'accepted' the click; otherwise
// it would be passed to the next window behind us.  If we accept
// the click we get mouse moved and mouse up callbacks, if we don't
// we do not get any more callbacks.  It is worth noting that we 
// will receive mouse moved and mouse up even if the mouse is dragged
// out of our window's box as long as the click started in our window's 
// box. 
int myHandleMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void*inRefcon)
{
	//char temp[200];

	static	int	dX = 0, dY = 0;
	static	int	Width = 0, Height = 0;
	int	Left, Top, Right, Bottom;
    int mouse_pos_x;
    int mouse_pos_y;

	static	int	gDragging = 0;

	float mx, my;

	if(!gnsVisible)
	{
		return 0;
	}
	if (!gnsOpened)
	{
		return 0;
	}
	/// Get the windows current position
	XPLMGetWindowGeometry(inWindowID, &Left, &Top, &Right, &Bottom);

	switch(inMouse) {
	case xplm_MouseDown:
		/// Test for the mouse in the top part of the window
		dX = x - Left;
		dY = y - Bottom;
		

		mx =  (float)dX/(float)(Right - Left);
		my =  1.0f - (float)dY/(float)(Top - Bottom);
		
		
        //logMessageEx("### xplm_MouseDown %d,%d -> %f,%f", dX, dY, mx, my);

		// The window is left, top (0,0) right bottom (BEZEL_WIDTH, BEZEL_HEIGHT)
        mouse_pos_x = (int)(mx*gpGNSIntf->bezel_width);
        mouse_pos_y = (int)(my*gpGNSIntf->bezel_height);

		if (coordInRect(mouse_pos_x, mouse_pos_y, &regionLCD))
		{		
			Width = Right - Left;
			Height = Top - Bottom;
			gDragging = 1;

            //logMessageEx("### xplm_MouseDown gDragging=1");

		}else
		{
			

			gGNSx30Proxy.sendMsg(0,mouse_pos_x ,mouse_pos_y );

            //logMessageEx("### xplm_MouseDown sendMsg(%d,%d)", mouse_pos_x ,mouse_pos_y);
		}


		break;
	case xplm_MouseDrag:
		/// We are dragging so update the window position
		if (gDragging)
		{

    
			Left+= (x - Left) - dX;
			Right = Left + Width;
			Bottom += (y - Bottom) - dY;
			Top = Bottom + Height;

			g_pos_x = Left;
			g_pos_y = Bottom;

			XPLMSetWindowGeometry(inWindowID, Left, Top, Right, Bottom);

            //logMessageEx("### xplm_MouseDrag");

		}
		break;
	case xplm_MouseUp:

        //logMessageEx("### xplm_MouseUp gDragging=%d", gDragging);

		if(!gDragging)
		{
            dX = x - Left;
            dY = y - Bottom;


            mx =  (float)dX/(float)(Right - Left);
            my =  1.0f - (float)dY/(float)(Top - Bottom);

            mouse_pos_x = (int)(mx*gpGNSIntf->bezel_width);
            mouse_pos_y = (int)(my*gpGNSIntf->bezel_height);

			gGNSx30Proxy.sendMsg(1, mouse_pos_x, mouse_pos_y);

            //logMessageEx("### xplm_MouseUp sendMsg(%d,%d)", mouse_pos_x, mouse_pos_y);

		}

        

		gDragging = 0;



		break;
	}

	


	return 1;

}                                      


XPLMCursorStatus myHandleCursorCallback(XPLMWindowID inWindowID, int x, int y, void *inRefcon)
{
	XPLMCursorStatus res = xplm_CursorDefault;

	if(coordInRect(x,y, &regionLCD))
	{
		res = xplm_CursorCustom;
	}

	return res;

}

int myHandleMouseWheelFunc(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks, void *inRefcon)   
{
	int res = 0;

	return res;
}


PLUGIN_API int XPluginStart(
							char *		outName,
							char *		outSig,
							char *		outDesc)
{
	

	
	// Provide our plugin's profile to the plugin system.
	strcpy(outName, PLUGIN_NAME);
	strcpy(outSig, "xplane.fly.elise-ng." PLUGIN_NAME);
	strcpy(outDesc, "Plugin interface to the Garmin GNS430/530 trainer.");
	

	if(0 != openLogFile())
	{
		return 0;
	}
	
	if(0 != loadSettings())
	{
		return 0;
	}

	if(0 != loadGNS())
	{
		return 0;
	}

	if(0 != initDataRefs())
	{
		return 0;
	}

	if(0 != createMenus())
	{
		return 0;
	}

	if(0 != initClickRegions())
	{
		return 0;
	}

	logMessageEx("--- Plugin started");
	
	
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	
	if(gnsOpened)
	{
		destroyGNSWindow();
	}

	unloadGNS();
	
	saveSettings();

	logMessageEx("--- Plugin stopped");
	
	
	
}

PLUGIN_API void XPluginDisable(void)
{
	gPluginEnabled = 0;
	logMessageEx("--- Plugin disabled");
}


PLUGIN_API int XPluginEnable(void)
{
	gPluginEnabled = 1;
	
	logMessageEx("--- Plugin enabled");
	
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
									  XPLMPluginID	inFromWho,
									  long			inMessage,
									  void *			inParam)
{
	
	char*	msg;
	
	if(XPLM_MSG_PLANE_CRASHED == inMessage)
		msg = "XPLM_MSG_PLANE_CRASHED";
	else if(XPLM_MSG_PLANE_LOADED == inMessage)
		msg = "XPLM_MSG_PLANE_LOADED";
	else if(XPLM_MSG_AIRPORT_LOADED == inMessage)
		msg = "XPLM_MSG_AIRPORT_LOADED";
	else if(XPLM_MSG_SCENERY_LOADED == inMessage)
		msg = "XPLM_MSG_SCENERY_LOADED";
	else if(XPLM_MSG_AIRPLANE_COUNT_CHANGED == inMessage)
		msg = "XPLM_MSG_AIRPLANE_COUNT_CHANGED";
	else if(XPLM_MSG_PLANE_UNLOADED == inMessage)
		msg = "XPLM_MSG_PLANE_UNLOADED";
	else 
		msg = "UNKNOWN";
	
	
	logMessageEx("--- XPluginReceiveMessage %d [%s]", inMessage, msg);

}

int	myDrawGNSCallback(XPLMDrawingPhase inPhase, int inIsBefore, void *inRefcon)
{
	

	if(!gnsVisible)
	{
		return -1;
	}
	if(!gnsOpened)
	{
		return -1;
	}
	
	/// Tell Xplane what we are doing
	XPLMSetGraphicsState(0/*Fog*/, 1/*TexUnits*/, 0/*Lighting*/, 0/*AlphaTesting*/, 0/*AlphaBlending*/, 0/*DepthTesting*/, 0/*DepthWriting*/);
	
	/// Handle day/night
	glColor3f(1.0, 1.0, 1.0);
	
	// Draw Panel
    glPushMatrix();

	glTranslated(g_pos_x, g_pos_y, 0);
	glScalef(g_zoom, g_zoom, 0);

	XPLMBindTexture2d(gTexture[TEX_BEZEL], GL_TEXTURE_2D);
	glBegin(GL_QUADS);

    glTexCoord2f(1, 0.0f); 
	glVertex2f((float)gpGNSIntf->bezel_width, 0);	// Bottom Right Of The Texture and Quad
	
	glTexCoord2f(0, 0.0f); 
	glVertex2f(0, 0);	// Bottom Left Of The Texture and Quad
	
	glTexCoord2f(0, 1.0f); 
	glVertex2f(0, (float)gpGNSIntf->bezel_height);	// Top Left Of The Texture and Quad
	
	glTexCoord2f(1, 1.0f); 
	glVertex2f((float)gpGNSIntf->bezel_width, (float)gpGNSIntf->bezel_height);	// Top Right Of The Texture and Quad
	glEnd();

    
	
	XPLMBindTexture2d(gTexture[TEX_LCD], GL_TEXTURE_2D);

	if(gpGNSIntf->LCDUpdated)
	{


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gpGNSIntf->lcd_width, gpGNSIntf->lcd_height, 0, GL_BGR, GL_UNSIGNED_BYTE, gpGNSIntf->LCD_data);


	    gpGNSIntf->LCDUpdated = false;
	}


	//float gltexture_x = (float)gpGNSIntf->lcd_width/(float)gpGNSIntf->lcd_width	;
	//float gltexture_y = (float)gpGNSIntf->lcd_height/(float)gpGNSIntf->lcd_height;
	

    //glPopMatrix();
    //glPushMatrix();
    //glTranslated(g_pos_x, g_pos_y 0);
    //glScalef(g_zoom, g_zoom, 0);
    glTranslated(gpGNSIntf->bezel_lcd_left, gpGNSIntf->bezel_height - gpGNSIntf->bezel_lcd_height - gpGNSIntf->bezel_lcd_top, 0);

	glBegin(GL_QUADS);
	
    glTexCoord2f(1, 0.0f); 
    glVertex2f((float)gpGNSIntf->bezel_lcd_width, 0);	// Bottom Right Of The Texture and Quad

    glTexCoord2f(0, 0.0f); 
    glVertex2f(0, 0);	// Bottom Left Of The Texture and Quad

    glTexCoord2f(0, 1.0f); 
    glVertex2f(0, (float)gpGNSIntf->bezel_lcd_height);	// Top Left Of The Texture and Quad

    glTexCoord2f(1, 1.0f); 
    glVertex2f((float)gpGNSIntf->bezel_lcd_width, (float)gpGNSIntf->bezel_lcd_height);	// Top Right Of The Texture and Quad


    glEnd();
	

	glPopMatrix();
	
	glFlush();
	
	return 1;
}


float	myFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * inRefcon)
{
	
	
	if(gPluginEnabled && gnsOpened)
	{
#if 0 //FIXME
        if(autopilotConnected)
		{
			XPLMSetDatai(g_override_autopilot_ref, 1); 
			XPLMSetDatai(g_autopilot_state_ref, 512); //see doc (HNAV engaged)
		
			
			override_gps = 1;//;pIntf->override_gps;
			XPLMSetDatai(g_override_gps_ref, override_gps); 

			gps_fromto = 1;
			XPLMSetDatai(g_gps_fromto_ref, gps_fromto); 

			gps_course_degtm = pIntf->dtk;
			XPLMSetDataf(g_gps_course_degtm_ref, gps_course_degtm); 

			hsi_obs_deg_mag_pilot = pIntf->dtk;
			XPLMSetDataf(g_hsi_obs_deg_mag_pilot_ref, hsi_obs_deg_mag_pilot); 

			
			XPLMSetDataf(g_gps_hdef_dot_ref, pIntf->cdi_horizontal_offtrack);

		}else
		{
			XPLMSetDatai(g_override_autopilot_ref, 0); 	
		}
#endif

        
        
        //Set the COM frequencies			
		XPLMSetDatai(g_com1_active_ref, gGNSx30Proxy.getCOMActiveFrequency()/10);
        XPLMSetDatai(g_com1_standby_ref, gGNSx30Proxy.getCOMStandbyFrequency()/10);
		
		XPLMSetDatai(g_nav1_active_ref, gGNSx30Proxy.getNAVActiveFrequency()/10);
        XPLMSetDatai(g_nav1_standby_ref, gGNSx30Proxy.getNAVStandbyFrequency()/10);


		latitude = XPLMGetDatad(g_latitude_ref);
		longitude = XPLMGetDatad(g_longitude_ref);
		heading = XPLMGetDataf(g_heading_ref);
		groundspeed = XPLMGetDataf(g_groundspeed_ref);
		elevation = (float)XPLMGetDatad(g_elevation_ref);
		vert_speed = XPLMGetDataf(g_vert_speed_ref);

		if(heading > 180.0f) // from 0 t pi and from -pi to 0
			heading = -360.0f + heading;
		

        gGNSx30Proxy.setGPSInfo(latitude * PI / 180.0f, // rad
                                longitude * PI / 180.0f, // rad
                                groundspeed, 
                                heading*PI/180.0f, 
                                vert_speed, 
                                elevation);
		
	}
	
	return gUpdateInterval; //update every 0.01 sec
}






int initTextures()
{
	int res = 0;


	
	
	XPLMGenerateTextureNumbers(&gTexture[0], 2);
	
	XPLMBindTexture2d(gTexture[TEX_BEZEL], GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gpGNSIntf->bezel_width, gpGNSIntf->bezel_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, gpGNSIntf->Bezel_data);
	
    // Set up all the the texture mapping params.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



	XPLMBindTexture2d(gTexture[TEX_LCD], GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gpGNSIntf->lcd_width, gpGNSIntf->lcd_height, 0, GL_BGR, GL_UNSIGNED_BYTE, gpGNSIntf->LCD_data);
	
    // Set up all the the texture mapping params.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	return res;
}




int   loadSettings()
{
	int res = 0;

	char	iniFilePath[MAX_FILE_NAME_LEN];

	// Load the configuration data
	XPLMGetSystemPath(iniFilePath);
	strcat(iniFilePath, "Resources\\plugins\\" PLUGIN_NAME "\\" PLUGIN_NAME ".ini");
	
	// Read data from ini file
	CIniReader configReader(iniFilePath);
	logMessageEx("--- Configuration data loaded");
	

    g_pos_x = configReader.ReadInteger(INI_SECTION_POSITION, "x", 0);
	g_pos_y = configReader.ReadInteger(INI_SECTION_POSITION, "y", 0);
	g_zoom = configReader.ReadFloat(INI_SECTION_POSITION, "zoom", 100);

	logMessageEx("--- x=%d", g_pos_x);
	logMessageEx("--- y=%d", g_pos_y);
	logMessageEx("--- zoom=%f", g_zoom);


	return res;

}

int   saveSettings()
{
	int res = 0;

	//char	temp[100];
	char	iniFilePath[MAX_FILE_NAME_LEN];
	FILE*	configFile;

	// Save the configuration data
	XPLMGetSystemPath(iniFilePath);
	strcat(iniFilePath, "Resources\\plugins\\" PLUGIN_NAME "\\" PLUGIN_NAME ".ini");
	
	configFile = fopen(iniFilePath, "w+t");
	
	if(NULL == configFile)
	{
		logMessageEx("*** Error saving configuration data");
		return -1;

	}

	fprintf(configFile, "%s\n", "[" INI_SECTION_POSITION "]");
	fprintf(configFile, "x=%d\n", g_pos_x);
	fprintf(configFile, "y=%d\n", g_pos_y);
	fprintf(configFile, "zoom=%f\n", g_zoom);

	fclose(configFile);

	logMessageEx("--- Configuration data saved");

	return res;

}


int   openLogFile()
{
	int res = 0;

	char	outputPath[MAX_FILE_NAME_LEN];

	// Open the plugin's log file
	XPLMGetSystemPath(outputPath);
	strcat(outputPath, "Resources\\plugins\\" PLUGIN_NAME "\\" PLUGIN_NAME ".log");
	logInit(outputPath);

	return res;

}


int   loadGNS()
{
	int res = 0;

    bool hideGNSWindow = true; //Set this to true to hide the GNS window
    gGNSx30Proxy.initialize(hideGNSWindow);
    gGNSx30Proxy.open(TYPE_GNS530);

    gpGNSIntf = gGNSx30Proxy.getInterface(); 
	
	logMessageEx("--- GNS initialized");


	return res;
}


int   unloadGNS()
{
	int res = 0;
	
    gpGNSIntf = NULL;
    gGNSx30Proxy.close();
    gGNSx30Proxy.terminate();
	
	logMessageEx("--- GNS uninitialized ");
	


	return res;
}


int   initDataRefs()
{
	int res = 0;

	bool	dataRefsFound = true;
	//data references
	g_latitude_ref = myXPLMFindDataRef("sim/flightmodel/position/latitude");
	if(NULL == g_latitude_ref) dataRefsFound=false;
	
	g_longitude_ref = myXPLMFindDataRef("sim/flightmodel/position/longitude");
	if(NULL == g_longitude_ref) dataRefsFound=false;
	
	g_heading_ref = myXPLMFindDataRef("sim/flightmodel/position/magpsi");
	if(NULL == g_heading_ref) dataRefsFound=false;
	
	g_groundspeed_ref = myXPLMFindDataRef("sim/flightmodel/position/groundspeed");
	if(NULL == g_groundspeed_ref) dataRefsFound=false;
	
	g_elevation_ref = myXPLMFindDataRef("sim/flightmodel/position/elevation");
	if(NULL == g_elevation_ref) dataRefsFound=false;
	
	g_vert_speed_ref = myXPLMFindDataRef("sim/flightmodel/position/vh_ind");
	if(NULL == g_vert_speed_ref) dataRefsFound=false;
	
	g_com1_standby_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/com1_standby_frequency_hz");
	if(NULL == g_com1_standby_ref) dataRefsFound=false;

	g_com1_active_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/com1_frequency_hz");
	if(NULL == g_com1_active_ref) dataRefsFound=false;

	g_nav1_standby_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/nav1_standby_frequency_hz");
	if(NULL == g_com1_standby_ref) dataRefsFound=false;

	g_nav1_active_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/nav1_frequency_hz");
	if(NULL == g_com1_active_ref) dataRefsFound=false;

	g_override_gps_ref = myXPLMFindDataRef("sim/operation/override/override_gps");
	if(NULL == g_override_gps_ref) dataRefsFound=false;

	//g_autopilot_mag_heading_ref = myXPLMFindDataRef("sim/cockpit/autopilot/heading_mag");
	//if(NULL == g_autopilot_mag_heading_ref) dataRefsFound=false;

	g_gps_fromto_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_fromto");
	if(NULL == g_gps_fromto_ref) dataRefsFound=false;

	g_gps_course_degtm_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_course_degtm");
	if(NULL == g_gps_course_degtm_ref) dataRefsFound=false;

	g_gps_hdef_dot_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_hdef_dot");
	if(NULL == g_gps_hdef_dot_ref) dataRefsFound=false;

	g_gps_vdef_dot_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_vdef_dot");
	if(NULL == g_gps_vdef_dot_ref) dataRefsFound=false;

	//g_gps_has_glideslope_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_has_glideslope");
	//if(NULL == g_gps_has_glideslope_ref) dataRefsFound=false;

	//g_gps_slope_degt_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_slope_degt");
	//if(NULL == g_gps_slope_degt_ref) dataRefsFound=false;

	//g_gps_dme_dist_m_ref = myXPLMFindDataRef("sim/cockpit/radios/gps_dme_dist_m");
	//if(NULL == g_gps_dme_dist_m_ref) dataRefsFound=false;
	
	//g_HSI_source_select_pilot_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/HSI_source_select_pilot");
	//if(NULL == g_HSI_source_select_pilot_ref) dataRefsFound=false;

	//g_hsi_obs_deg_mag_pilot_ref = myXPLMFindDataRef("sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot");
	//if(NULL == g_hsi_obs_deg_mag_pilot_ref) dataRefsFound=false;

	g_override_autopilot_ref = myXPLMFindDataRef("sim/operation/override/override_autopilot");
	if(NULL == g_override_autopilot_ref) dataRefsFound=false;

	g_autopilot_state_ref = myXPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");
	if(NULL == g_autopilot_state_ref) dataRefsFound=false;

	

/*
	g_override_nav1_needles_ref = myXPLMFindDataRef("sim/operation/override/override_nav1_needles");
	if(NULL == g_override_nav1_needles_ref) dataRefsFound=false;

	g_nav1_hdef_dot_ref = myXPLMFindDataRef("sim/cockpit/radios/nav1_hdef_dot");
	if(NULL == g_nav1_hdef_dot_ref) dataRefsFound=false;

	g_nav1_vdef_dot_ref = myXPLMFindDataRef("sim/cockpit/radios/nav1_vdef_dot");
	if(NULL == g_nav1_vdef_dot_ref) dataRefsFound=false;

	g_nav1_fromto_ref = myXPLMFindDataRef("sim/cockpit/radios/nav1_fromto");
	if(NULL == g_nav1_fromto_ref) dataRefsFound=false;
*/	

	if(dataRefsFound)
	{
		logMessageEx("--- Data References retrieved");
	}else
	{
		logMessageEx("*** Error retrieving Data References");
		res = -1;
	}


	return res;
}


int createMenus()
{
	int res = 0;

	//Create the Show/Hide Data Menu
	//First we put a new menu item into the plugin menu.
	//This menu item will contain a submenu for us.
	myMenu = XPLMAppendMenuItem(
		XPLMFindPluginsMenu(),	// Put in plugins menu 
		MENU_NAME,		// Item Title 
		0,						// Item Ref
		1);						// Force English 
	
	// Now create a submenu attached to our menu item. */
	mySubMenu = XPLMCreateMenu(
		SUBMENU_NAME, 
		XPLMFindPluginsMenu(), 
		myMenu, 				// Menu Item to attach to.
		myMenuHandlerCallback,	// The handler 
		0);						// Handler Ref 
	
	// Append a few menu items to our submenu.  We will use the refcon to
	XPLMAppendMenuItem(
		mySubMenu,
		"Open",
		(void *) MENU_OPEN,
		1);

	XPLMAppendMenuItem(
		mySubMenu,
		"Close",
		(void *) MENU_CLOSE,
		1);


	XPLMAppendMenuSeparator(mySubMenu);

	XPLMAppendMenuItem(
		mySubMenu,
		"Show",
		(void *) MENU_SHOW,
		1);

	XPLMAppendMenuItem(
		mySubMenu,
		"Hide",
		(void *) MENU_HIDE,
		1);


	XPLMAppendMenuSeparator(mySubMenu);
	
	XPLMAppendMenuItem(
		mySubMenu,
		"Increase size",
		(void *) MENU_SIZE_INC,
		1);


	XPLMAppendMenuItem(
		mySubMenu,
		"Decrease size",
		(void *) MENU_SIZE_DEC,
		1);

	XPLMAppendMenuItem(
		mySubMenu,
		"Restore size",
		(void *) MENU_SIZE_RESTORE,
		1);


	XPLMAppendMenuSeparator(mySubMenu);
	
	myMenuItemWAASAutoipilot = XPLMAppendMenuItem(
		mySubMenu,
		"Autopilot connected",
		(void *) MENU_AUTOPILOT_CONNECTED,
		1);

	XPLMCheckMenuItem(mySubMenu, myMenuItemWAASAutoipilot, xplm_Menu_Unchecked);    	
	autopilotConnected = false;

	
	logMessageEx("--- Menu created");
	

	return res;
}


int	createGNSWindow()
{
	int res = 0;


	XPLMCreateWindow_t win;
	win.structSize = sizeof(XPLMCreateWindow_t);
	win.left = g_pos_x;
	win.top = g_pos_y + (int)(gpGNSIntf->bezel_height*g_zoom);
	win.right = g_pos_x+(int)(gpGNSIntf->bezel_width*g_zoom);
	win.bottom = g_pos_y;
	win.visible = true;
	win.drawWindowFunc = myDrawWindowCallback;
	win.handleMouseClickFunc = myHandleMouseClickCallback;
	win.handleKeyFunc = myHandleKeyCallback;
	win.handleCursorFunc = myHandleCursorCallback;
	win.handleMouseWheelFunc = myHandleMouseWheelFunc;
	win.refcon = NULL;



	//Register the Draw Window
	// Create a windows for displaying the projection info 
	gWindow = XPLMCreateWindowEx(&win);
	
	logMessageEx("--- GNS Window created");

	initTextures();
	logMessageEx("--- Textures created");
	
	/// Register so that our gauge is drawing during the Xplane gauge phase
	XPLMRegisterDrawCallback(myDrawGNSCallback, xplm_Phase_Window, 1, NULL); // before window draw
	logMessageEx("--- DrawCallback registered");
	
	//Register the callback for gUpdateInterval.  Positive intervals
	//are in seconds, negative are the negative of sim frames.  Zero
	//registers but does not schedule a callback for time. 
	XPLMRegisterFlightLoopCallback(myFlightLoopCallback, gUpdateInterval, NULL);
	logMessageEx("--- FlightLoopCallback registered");

    //Set the initial values of the COM and NAV
    gGNSx30Proxy.setCOMActiveFrequency(XPLMGetDatai(g_com1_active_ref)*10);
    gGNSx30Proxy.setCOMStandbyFrequency(XPLMGetDatai(g_com1_standby_ref)*10);
    gGNSx30Proxy.setNAVActiveFrequency(XPLMGetDatai(g_nav1_active_ref)*10);
    gGNSx30Proxy.setNAVStandbyFrequency(XPLMGetDatai(g_nav1_standby_ref)*10);


	gnsOpened = true;

	return res;
}

int	destroyGNSWindow()
{
	int res = 0;


	// Register so that our gauge is drawing during the Xplane gauge phase
	XPLMUnregisterDrawCallback(myDrawGNSCallback, xplm_Phase_Window, 0, NULL);
	logMessageEx("--- DrawCallback unregistered");

	// Unregister the callback 
	XPLMUnregisterFlightLoopCallback(myFlightLoopCallback, NULL);
	logMessageEx("--- FlightLoopCallback unregistered");

	// Destroy GNS window
	XPLMDestroyWindow(gWindow);    
	logMessageEx("--- GNS Window destroyed");

	//Gns500Intf.Close();

	gnsOpened = false;

	return res;

}

int	initClickRegions()
{
    

	int res = 0;
	regionLCD.x1 = (float)gpGNSIntf->bezel_lcd_left;
	regionLCD.y1 = (float)gpGNSIntf->bezel_lcd_top;
	regionLCD.x2 = (regionLCD.x1 + (float)gpGNSIntf->bezel_lcd_width);
	regionLCD.y2 = (regionLCD.y1 + (float)gpGNSIntf->bezel_lcd_height);


	return res;
}

int	coordInRect(int x, int y, region* reg)
{
	return ((x >= reg->x1) && (x < reg->x2) && (y > reg->y1) && (y <= reg->y2)); 
}
