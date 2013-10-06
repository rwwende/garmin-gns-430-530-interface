#ifndef __LOG__
#define __LOG__

#include <windows.h>
#include <tchar.h>



int     logInit(char* fileName);
int     logTerminate();
void    logMessageEx(const char *fmt, ...);
void    logAdjustIndent(int indent);


#endif

