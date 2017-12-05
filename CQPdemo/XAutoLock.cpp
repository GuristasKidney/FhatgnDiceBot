#include "stdafx.h"
#include "XAutoLock.h"

XAutoLock::XAutoLock(XCritSec&rXCritSec)
	:m_rXCritSec(rXCritSec)
{
	m_rXCritSec.Lock();
}

XAutoLock::~XAutoLock()
{
	m_rXCritSec.UnLock();
}

