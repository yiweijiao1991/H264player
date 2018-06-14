#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "inttypes.h"
#include "stdint.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
}
#endif
#include <list>
using namespace std;

class CGFFX
{
	typedef struct tagBUFFER
	{
		BYTE* pbImage;
		INT   iSize;
		INT   iTotalSize;
	}BUFFER;
public:
	         CGFFX(void);
	virtual ~CGFFX(void);
	static HRESULT Initialize(void);
	HRESULT  Play(HWND hWnd, LPCSTR szURL);
	VOID     Stop(VOID);
	void H264control(int cmd);
protected:
	HRESULT  Connect(VOID);
	int stream_component_open(AVFormatContext *ic, int stream_index);
protected:
	static DWORD WINAPI OnRTSPReceiveThread(VOID* pContext);
	static DWORD WINAPI OnH264DecodeThread(VOID* pContext);
private:
	static BOOL m_fInitialize;
	HANDLE m_hPlayThread;
	HANDLE m_hH264Thread;
	BOOL   m_fRunning;
	CHAR   m_szURL[MAX_PATH];
	AVFormatContext* m_pFormatCtx;
	HANDLE m_hEvent;
	CRITICAL_SECTION m_csSync;
	list<BUFFER *>m_lstImage;
	list<BUFFER *>m_lstUnusedImage;
	int m_flag;
	HWND m_hWndtest;
};

