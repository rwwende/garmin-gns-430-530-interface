#include "thread.h"

Win32Thread::Win32Thread()
{
	m_hThread = 0;
	m_threadId = 0;
	m_canRun = true;
	m_suspended = true;
}

Win32Thread::~Win32Thread()
{
	if (m_hThread)
		CloseHandle(m_hThread);
}

bool Win32Thread::canRun()
{
	Lock guard(m_mutex);
	return m_canRun;
}

bool Win32Thread::create(unsigned int stackSize)
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(0, stackSize, 
		threadFunc, this, CREATE_SUSPENDED, &m_threadId));
	
	if (m_hThread)
		return true;
	
	return false;
}

void Win32Thread::join()
{
	WaitForSingleObject(m_hThread, INFINITE);
}

void Win32Thread::resume()
{
	if (m_suspended)
	{
		Lock guard(m_mutex);
		
		if (m_suspended)
		{
			ResumeThread(m_hThread);
			m_suspended = false;
		}
	}
}

void Win32Thread::shutdown()
{
	if (m_canRun)
	{
		Lock guard(m_mutex);

		if (m_canRun)
			m_canRun = false;

		resume();
	}
}

void Win32Thread::start()
{
	resume();
}

void Win32Thread::suspend()
{
	if (!m_suspended)
	{
		Lock guard(m_mutex);

		if (!m_suspended)
		{
			SuspendThread(m_hThread);
			m_suspended = true;
		}
	}
}

unsigned int Win32Thread::threadId() const
{
	return m_threadId;
}

unsigned int __stdcall Win32Thread::threadFunc(void *args)
{
	Win32Thread *pThread = reinterpret_cast<Win32Thread*>(args);
	
	if (pThread)
		pThread->run();

	_endthreadex(0);
	return 0;
}