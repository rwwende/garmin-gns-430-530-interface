#ifndef INJECTOR_H
#define INJECTOR_H
#include <windows.h>

int startAndInject(TCHAR* pExeName, TCHAR* pExePath, TCHAR* pLibFile);
int checkProcessTerminated();

#endif
