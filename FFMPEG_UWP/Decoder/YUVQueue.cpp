#include "pch.h"
#include "YUVQueue.h"


CYUVQueue::CYUVQueue()
{
	InitializeSRWLock(&m_LockAlloc);
	InitializeSRWLock(&m_LockReadBuffer);
	
}
BOOL CYUVQueue::Initialize(DWORD dwMaxBufferNum,DWORD Width,DWORD Height,DWORD Stride)
{
	m_ppYUVFramePool = new YUV_FRAME*[dwMaxBufferNum];
	m_ppYUVFrameReadList = new YUV_FRAME*[dwMaxBufferNum];
	m_dwMaxFrameNum = dwMaxBufferNum;

	m_dwWidth = Width;
	m_dwHeight = Height;
	m_dwStride = Stride;

	for (DWORD i=0; i<m_dwMaxFrameNum; i++)
	{
		m_ppYUVFramePool[i] = CreateYUVFrame(m_dwWidth,m_dwHeight,Stride);
		m_dwAvailableFrameNum++;
	}

	return TRUE;
}



YUV_FRAME* CYUVQueue::CreateYUVFrame(DWORD Width,DWORD Height,DWORD Stride)
{
	YUV_FRAME*	pYUVFrame = NULL;

	DWORD	YSize = Stride*Height;
	DWORD	UVSize = YSize / 4;
	
	
	size_t	size = sizeof(YUV_FRAME) + 
		(Stride* Height) + 
		((Stride* Height)/4) +
		((Stride* Height)/4) - 
		sizeof(BYTE);

	pYUVFrame = (YUV_FRAME*)malloc(size);

	pYUVFrame->Width = Width;
	pYUVFrame->Height = Height;
	pYUVFrame->Stride = Stride;
	
	pYUVFrame->pYBuffer = pYUVFrame->pBuffer;
	pYUVFrame->pUBuffer = pYUVFrame->pYBuffer + YSize;
	pYUVFrame->pVBuffer = pYUVFrame->pUBuffer + UVSize;

	return pYUVFrame;
}

BOOL CYUVQueue::UpdateYUVFrame(YUV_FRAME* pDest,DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp)
{
	DWORD	YSize = Stride*Height;
	DWORD	UVSize = YSize / 4;

	if (pDest->Width != Width || pDest->Height != Height || pDest->Stride != Stride)
		__debugbreak();


lb_update:
	pDest->time_stamp = time_stamp;
	memcpy(pDest->pYBuffer,pYBuffer,YSize);
	memcpy(pDest->pUBuffer,pUBuffer,UVSize);
	memcpy(pDest->pVBuffer,pVBuffer,UVSize);

	return TRUE;
}
YUV_FRAME* CYUVQueue::AllocFrame()
{
	YUV_FRAME*	pBuffer = NULL;

	AcquireSRWLockExclusive(&m_LockAlloc);

	if (!m_dwAvailableFrameNum)
		goto lb_exit;

	m_dwAvailableFrameNum--;
	
	pBuffer = m_ppYUVFramePool[m_dwAvailableFrameNum];
	m_ppYUVFramePool[m_dwAvailableFrameNum] = NULL;

lb_exit:
	ReleaseSRWLockExclusive(&m_LockAlloc);

	return pBuffer;
}
void CYUVQueue::FreeFrame(YUV_FRAME* pFrame)
{
	AcquireSRWLockExclusive(&m_LockAlloc);
	
	m_ppYUVFramePool[m_dwAvailableFrameNum] = pFrame;
	m_dwAvailableFrameNum++;

	ReleaseSRWLockExclusive(&m_LockAlloc);
	
	
}
void CYUVQueue::FreeAllReadableFrames()
{
	// 마지막 하나 남기고 몽땅 해제
	AcquireSRWLockExclusive(&m_LockReadBuffer);
	AcquireSRWLockExclusive(&m_LockAlloc);
	
	for (DWORD i=0; i<m_dwReadableFrameNum; i++)
	{

		m_ppYUVFramePool[m_dwAvailableFrameNum] = m_ppYUVFrameReadList[i];
		m_dwAvailableFrameNum++;

		m_ppYUVFrameReadList[i] = NULL;
	}
	m_dwReadableFrameNum = 0;

	ReleaseSRWLockExclusive(&m_LockAlloc);
	ReleaseSRWLockExclusive(&m_LockReadBuffer);
}

void CYUVQueue::AddBufferToRenderQueue(YUV_FRAME* pFrame)
{
	AcquireSRWLockExclusive(&m_LockReadBuffer);

	m_ppYUVFrameReadList[m_dwReadableFrameNum] = pFrame;
	m_dwReadableFrameNum++;


	ReleaseSRWLockExclusive(&m_LockReadBuffer);
}

BOOL CYUVQueue::UpdateAndNext(DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp)
{
	BOOL	bResult = FALSE;

	YUV_FRAME*	pBuffer = AllocFrame();
	if (!pBuffer)
		goto lb_return;

	UpdateYUVFrame(pBuffer,Width,Height,pYBuffer,pUBuffer,pVBuffer,Stride,time_stamp);
	
	AddBufferToRenderQueue(pBuffer);
	bResult = TRUE;

lb_return:
	return bResult;
}

YUV_FRAME* CYUVQueue::AcquireBufferWithSec(__int64 time_stamp)
{
	BOOL	bResult = FALSE;
	
	YUV_FRAME*	pSelectedFrame = NULL;
	DWORD	dwSelectedIndex = -1;

	AcquireSRWLockShared(&m_LockReadBuffer);

	if (!m_dwReadableFrameNum)
	{
		// no yuv buffers to read
		goto lb_exit;
	}
	if (time_stamp <= m_ppYUVFrameReadList[0]->time_stamp)
	{
		dwSelectedIndex = 0;

	}
	else
	{
		for (DWORD i=0; i<m_dwReadableFrameNum; i++)
		{
			YUV_FRAME*	pCurFrame = m_ppYUVFrameReadList[i];
			
			if (time_stamp >= pCurFrame->time_stamp)
			{
				dwSelectedIndex = i;
			}
			else
			{
				break;
			}
		}
	}
	if (-1 == dwSelectedIndex)
	{
		goto lb_exit;
	}
	
	pSelectedFrame = m_ppYUVFrameReadList[dwSelectedIndex];

	AcquireSRWLockExclusive(&m_LockAlloc);
	
	for (DWORD i=0; i<dwSelectedIndex; i++)
	{
		m_ppYUVFramePool[m_dwAvailableFrameNum] = m_ppYUVFrameReadList[i];
		m_dwAvailableFrameNum++;
		m_dwReadableFrameNum--;
	}
	memcpy(m_ppYUVFrameReadList,m_ppYUVFrameReadList+dwSelectedIndex,sizeof(YUV_FRAME*)*m_dwReadableFrameNum);
	
	ReleaseSRWLockExclusive(&m_LockAlloc);
lb_exit:
	ReleaseSRWLockShared(&m_LockReadBuffer);

	return pSelectedFrame;

}
void CYUVQueue::ReleaseBuffer(YUV_FRAME* pFrame)
{
	FreeFrame(pFrame);
}
void CYUVQueue::CleanupYUVFrame(YUV_FRAME* pFrame)
{
	if (pFrame)
	{
		free(pFrame);
		pFrame = NULL;
	}
}
void CYUVQueue::Empty()
{
	FreeAllReadableFrames();
}
void CYUVQueue::Cleanup()
{
	Empty();

#ifdef _DEBUG
	if (m_dwReadableFrameNum)
		__debugbreak();

#endif
	if (m_ppYUVFramePool)
	{
		for (DWORD i=0; i<m_dwMaxFrameNum; i++)
		{
			CleanupYUVFrame(m_ppYUVFramePool[i]);
			m_ppYUVFramePool[i] = NULL;
		}
		delete [] m_ppYUVFramePool;
		m_ppYUVFramePool = NULL;
	}
	if (m_ppYUVFrameReadList)
	{
		delete [] m_ppYUVFrameReadList;
		m_ppYUVFrameReadList = NULL;
	}
}
CYUVQueue::~CYUVQueue()
{
	Cleanup();
}