#pragma once

#include "../Common/decoder_typedef.h"

class CYUVQueue
{
	YUV_FRAME**	m_ppYUVFramePool = nullptr;
	YUV_FRAME**	m_ppYUVFrameReadList = nullptr;
	

	SRWLOCK		m_LockAlloc;
	SRWLOCK		m_LockReadBuffer;
	

	DWORD		m_dwMaxFrameNum = 0;
	DWORD		m_dwAvailableFrameNum = 0;
	DWORD		m_dwReadableFrameNum = 0;

	DWORD		m_dwWidth = 0;
	DWORD		m_dwHeight = 0;
	DWORD		m_dwStride = 0;
	
	

	BOOL		UpdateYUVFrame(YUV_FRAME* pDest,DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp);

	YUV_FRAME*	AllocFrame();
	void		FreeFrame(YUV_FRAME* pFrame);

	void		AddBufferToRenderQueue(YUV_FRAME* pFrame);

	YUV_FRAME*	CreateYUVFrame(DWORD Width,DWORD Height,DWORD Stride);
	void		CleanupYUVFrame(YUV_FRAME* pFrame);
	
	void		FreeAllReadableFrames();
	

	void	Cleanup();
public:
	BOOL	Initialize(DWORD dwMaxBufferNum,DWORD Width,DWORD Height,DWORD Stride);
	BOOL	UpdateAndNext(DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp);
	
	YUV_FRAME*		AcquireBufferWithSec(__int64 time_stamp);
	void			ReleaseBuffer(YUV_FRAME* pFrame);
	void			Empty();

	DWORD		GetWidth()	{return m_dwWidth;}
	DWORD		GetHeight()	{return m_dwHeight;}
	DWORD		GetStride()	{return m_dwStride;}
	CYUVQueue();
	~CYUVQueue();
};

