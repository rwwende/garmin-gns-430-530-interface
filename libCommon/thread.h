#ifndef __WIN32_THREAD_CLASS_H__
#define __WIN32_THREAD_CLASS_H__

#include <windows.h>
#include <process.h>

//------------------------------------------------------------------------
// Class   : Mutex
// Extends : none
//
// A Mutex allows threads mutually exclusive access to a resource.
//------------------------------------------------------------------------
class Mutex
{
public:
	Mutex() {InitializeCriticalSection(&m_mutex);}
	~Mutex() {DeleteCriticalSection(&m_mutex);}

	void acquire() {EnterCriticalSection(&m_mutex);}
	void release() {LeaveCriticalSection(&m_mutex);}

private:
	CRITICAL_SECTION m_mutex;
};

//------------------------------------------------------------------------
// Class   : Lock
// Extends : none
//
// A Lock provides a safe way to acquire and release a Mutex. The Mutex 
// is acquired when the Lock it created. The Mutex is released when the
// Lock goes out of scope.
//------------------------------------------------------------------------
class Lock
{
public:
	Lock(Mutex &mutex) : m_mutex(mutex) {m_mutex.acquire();}
	~Lock() {m_mutex.release();}

private:
	Mutex &m_mutex;
};

//------------------------------------------------------------------------
// Class   : Win32Thread
// Extends : none
//
// WIN32 thread class. The Win32Thread is always created in a suspended
// state. The thread is not running until start() is called. If zero is
// used as the stack size of the new thread, then Windows will use the
// stack size of the main thread.
//
// To create your own thread, subclass Win32Thread and provide an
// implementation for the run() method. If you want to give other threads
// the ability to cleanly shutdown your thread (recommended), then your
// thread's run() method should periodically call canRun() to check if 
// another thread has made a requested to shutdown your thread. canRun()
// will return false if another thread has requested that your thread
// shutdown.
//------------------------------------------------------------------------
class Win32Thread
{
public:
	Win32Thread();
	virtual ~Win32Thread();

	bool create(unsigned int stackSize = 0);
	unsigned int threadId() const;
	void start();
	void join();

	void resume();
	void suspend();
	void shutdown();
	
protected:
	bool canRun();
	virtual void run() = 0;

private:
	static unsigned int __stdcall threadFunc(void *args);
	
	HANDLE m_hThread;
	unsigned int m_threadId;
	volatile bool m_canRun;
	volatile bool m_suspended;
	Mutex m_mutex;
};

#endif //__WIN32_THREAD_CLASS_H__