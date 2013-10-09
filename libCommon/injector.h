#ifndef INJECTOR_H
#define INJECTOR_H
#include <windows.h>

int startAndInject(char* pExeName, char* pExePath, char* pLibFile);
int checkProcessTerminated();

#endif
