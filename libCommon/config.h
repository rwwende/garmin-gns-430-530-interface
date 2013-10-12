#ifndef __CONFIG__
#define __CONFIG__

#include <windows.h>

#define SHMEM_NAME "__GNS__"

#define TYPE_GNS430	430
#define TYPE_GNS530	530


#define OFFSCREEN_BUFFER_WIDTH 512
#define OFFSCREEN_BUFFER_HEIGHT 512

#define G530SIMEXE_PATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\G530SIM.exe"
#define GARMIN_INTERNATIONAL "Software\\GARMIN International\\gnsx30"
#define GARMIN_INTERNATIONAL_SETTINGS "Software\\GARMIN International\\gnsx30\\Trainer Settings"
#define GARMIN_INTERNATIONAL_WINDOW "Software\\GARMIN International\\gnsx30\\WindowPosition"
#define GNS_430AWT "GNS 430AW"
#define GNS_530AWT "GNS 530AWT"

#define GNSX30EXE_CRC 0x192813d1

#define PI					(3.14159265f)

#endif //__CONFIG__