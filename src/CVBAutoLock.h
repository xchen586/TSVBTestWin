#pragma once
//////////////////////////////////////////////////////////////////////////
//  CritSec
//  Description: Wraps a critical section.
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

class CVBCritSec
{
private:
	CRITICAL_SECTION m_criticalSection;
public:
	CVBCritSec()
	{
		InitializeCriticalSection(&m_criticalSection);
	}

	~CVBCritSec()
	{
		DeleteCriticalSection(&m_criticalSection);
	}

	void Lock()
	{
		EnterCriticalSection(&m_criticalSection);
	}

	void Unlock()
	{
		LeaveCriticalSection(&m_criticalSection);
	}
};


//////////////////////////////////////////////////////////////////////////
//  AutoLock
//  Description: Provides automatic locking and unlocking of a 
//               of a critical section.
//
//  Note: The AutoLock object must go out of scope before the CritSec.
//////////////////////////////////////////////////////////////////////////

class CVBAutoLock
{
private:
	CVBCritSec *m_pCriticalSection;
public:
	CVBAutoLock(CVBCritSec& crit)
	{
		m_pCriticalSection = &crit;
		m_pCriticalSection->Lock();
	}
	~CVBAutoLock()
	{
		m_pCriticalSection->Unlock();
	}
};