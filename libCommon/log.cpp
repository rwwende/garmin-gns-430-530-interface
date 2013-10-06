#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <share.h>

#include "log.h"



#define SERIALIZE_LOG 1
#define MAX_LOG_LINE_LEN    512


#if SERIALIZE_LOG
static CRITICAL_SECTION gSC;
#endif

static FILE*  gLogFile=NULL;
static bool   gLogDebug = false;

static int gIndentCount = 0;


char    temp[MAX_LOG_LINE_LEN];
char    tempindent[MAX_LOG_LINE_LEN];
char    indentstr[MAX_LOG_LINE_LEN];


int logInit(char* fileName)
{  
    int res = 0;
    
#if SERIALIZE_LOG    
    InitializeCriticalSection(&gSC);
#endif    
		
	if(NULL != gLogFile)
	{
		fclose(gLogFile);
		gLogFile = NULL;
	}


	gLogFile = fopen(fileName, "w+t");
	
	return res;
    
}

int logTerminate()
{
    int res = 0;
    if(NULL != gLogFile)
    {
        fflush(gLogFile);
        fclose(gLogFile);
        res = 1;
    }  
    
#if SERIALIZE_LOG    
    DeleteCriticalSection(&gSC);
#endif
    
    return res;  
}



static void logMessage(char* msg)
{
    if(NULL != gLogFile)
    {
	    SYSTEMTIME  st;
	    GetLocalTime(&st);

	    fprintf(gLogFile, "%02d.%02d.%04d %02d:%02d:%02d.%03d [%08x] %s\n", 
		    st.wDay, st.wMonth, st.wYear,
		    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		    GetCurrentThreadId(),
		    msg);
	    fflush(gLogFile);
	}
}


void logMessageEx(const char *fmt, ...)
{
    if(NULL != gLogFile)
    {
    
#if SERIALIZE_LOG    
    EnterCriticalSection(&gSC);
#endif
    
        va_list ap;
        va_start(ap,fmt);
       
        //_vstprintf(temp, fmt,ap);
        vsnprintf(temp, MAX_LOG_LINE_LEN, fmt, ap);
       
        strncpy(indentstr, "", MAX_LOG_LINE_LEN);
        if(gIndentCount>0)
        {
            for(int i = 0; i < gIndentCount; i++)
            {
               strncat(indentstr, "\t", MAX_LOG_LINE_LEN); 
            }
        }
        
        sprintf(tempindent, "%s%s", indentstr, temp);
       
        
       
        logMessage(tempindent);
       
        va_end(ap);
#if SERIALIZE_LOG
    LeaveCriticalSection(&gSC);
#endif    
    
    }
    
}


void  logAdjustIndent(int indent)
{
    gIndentCount+=indent;
}

