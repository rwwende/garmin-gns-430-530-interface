#ifndef __GNSX30PROXY__
#define __GNSX30PROXY__

#include "sharedstruct.h"
#include "gnsTypes.h"


class CRCFile;
class FileLog;

class GNSx30Proxy
{
public:
	GNSx30Proxy();
	virtual ~GNSx30Proxy();

	int initialize();
	int terminate();
	int open(int gnsType);
	int close();
	GNSIntf* getInterface();
	int sendMsg(int up, int x, int y );

private:

private:
	char					m_trainter_path[MAX_PATH];
	char					m_trainter_exe[MAX_PATH];
    char					m_interface_lib[MAX_PATH];
	HWND					m_win;
	
    SharedStruct<GNSIntf>*  m_pShared;
	GNSIntf*				m_pvData;

	CRCFile*				m_pCRCFile;
};	

#endif // __GNSX30PROXY__
