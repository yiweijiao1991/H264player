#include "stdafx.h"
#include "GFFX.h"
#include <vfw.h>
#pragma comment(lib, "vfw32.lib")

BOOL CGFFX::m_fInitialize = FALSE;
CGFFX::CGFFX(void)
{
	m_hPlayThread = NULL;
	m_hH264Thread = NULL;
	m_fRunning    = FALSE;
	m_pFormatCtx  = NULL;
	m_hWndtest = NULL;
	m_flag = 0;
	strcpy(m_szURL, "");
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeCriticalSection(&m_csSync);

	Initialize();
}


CGFFX::~CGFFX(void)
{
	Stop();
	DeleteCriticalSection(&m_csSync);
	CloseHandle(m_hEvent);
}

HRESULT CGFFX::Initialize(void)
{
	if(!m_fInitialize)
	{
		m_fInitialize = TRUE;
		avcodec_register_all();
		av_register_all();
	}
	return S_OK;
}

HRESULT CGFFX::Play(HWND hWnd, LPCSTR szURL)
{	
	Stop();
	//配置窗口信息
	m_hWndtest = hWnd;
	//URL
	strcpy(m_szURL, szURL);
	m_fRunning = TRUE;
	m_pFormatCtx = NULL;
	//删除链表
	for(list<BUFFER*>::iterator itr = m_lstImage.begin(); itr != m_lstImage.end(); itr++)
	{
		BUFFER* buf = *itr;
		delete []buf->pbImage;
		delete buf;
	}
	m_lstImage.clear();
	//创建RTSP接收线程
	DWORD id;
	m_flag = 1;
	m_hPlayThread = CreateThread(NULL, 0, OnRTSPReceiveThread, this, 0, &id);
	//创建解压H264线程
	m_hH264Thread = CreateThread(NULL, 0, OnH264DecodeThread, this, 0, &id);
	return NULL != m_hPlayThread && NULL != m_hH264Thread ? S_OK : E_FAIL;
}

VOID CGFFX::Stop(VOID)
{
	MSG msg;
	m_flag = 0;
	Sleep(1000);
	m_fRunning = FALSE;
	//停止播放线程
	
	while(NULL != m_hPlayThread && WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &m_hPlayThread, FALSE, 1000, QS_ALLINPUT))
	{			
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	while(NULL != m_hH264Thread && WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &m_hH264Thread, FALSE, 1000, QS_ALLINPUT))
	{			
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(NULL != m_pFormatCtx)
	{
		av_close_input_file(m_pFormatCtx);
		m_pFormatCtx = NULL;
	}
	//还原默认值
	m_hPlayThread = NULL;
	m_hH264Thread = NULL;
	m_fRunning = FALSE;
	//删除链表
	for(list<BUFFER*>::iterator itr = m_lstImage.begin(); itr != m_lstImage.end(); itr++)
	{
		BUFFER* buf = *itr;
		delete []buf->pbImage;
		delete buf;
	}
	m_lstImage.clear();
}

DWORD WINAPI CGFFX::OnRTSPReceiveThread(VOID* pContext)
{
	CGFFX* pThis = (CGFFX *)pContext;
	AVPacket  packet;
	AVInputFormat *fmt = NULL;
	BOOL fConnect = FALSE;
	while(pThis->m_fRunning)
	{	
		if (pThis->m_flag == 0)
		{
			Sleep(1000);
			continue;
		}
		//连接RTSP
		if(!fConnect && !(fConnect = SUCCEEDED(pThis->Connect())))
		{
			continue;
		}
		//接收每一帧视频
		if(0 > av_read_frame(pThis->m_pFormatCtx, &packet))
		{
			fConnect = FALSE;
			//等待队列为空			
			for(;;)
			{
				EnterCriticalSection(&pThis->m_csSync);
				INT size = pThis->m_lstImage.size();
				LeaveCriticalSection(&pThis->m_csSync);
				if(0 < size)
				{
					Sleep(1500);
				}
				else
				{
					break;
				}
			}
			//删除连接
			Sleep(2000);
			av_close_input_file(pThis->m_pFormatCtx);
			pThis->m_pFormatCtx = NULL;
			continue;
		}
		if(NULL != packet.data && 0 < packet.size)
		{	//判断队列长度
			EnterCriticalSection(&pThis->m_csSync);
			INT size = pThis->m_lstImage.size();
			LeaveCriticalSection(&pThis->m_csSync);
			if(10 > size)
			{	
				BUFFER* buf = NULL;
				//从回收队列中那内存
				EnterCriticalSection(&pThis->m_csSync);
				if(0 < pThis->m_lstUnusedImage.size())
				{
					buf = pThis->m_lstUnusedImage.front();
					pThis->m_lstUnusedImage.pop_front();
				}
				LeaveCriticalSection(&pThis->m_csSync);
				//生成节点
				if(NULL == buf)
				{
					buf = new BUFFER;
					memset(buf, 0, sizeof(BUFFER));
				}
				if(buf->iTotalSize < packet.size)
				{
					if(NULL != buf->pbImage)
					{
						delete buf->pbImage;
						buf->pbImage = NULL;
					}
					buf->iTotalSize = packet.size;
					buf->iSize = packet.size;
					buf->pbImage = new BYTE[buf->iTotalSize];
				}
				else
				{
					buf->iSize = packet.size;
				}
				//拷贝内存
				memcpy(buf->pbImage, packet.data, buf->iSize);
				//压入队列
				EnterCriticalSection(&pThis->m_csSync);
				pThis->m_lstImage.push_back(buf);
				LeaveCriticalSection(&pThis->m_csSync);
				SetEvent(pThis->m_hEvent);
			}
		}
		//释放一个rtsp包
		av_free_packet(&packet);
	}
	return 0;
}

DWORD WINAPI CGFFX::OnH264DecodeThread(VOID* pContext)
{
	CGFFX* pThis = (CGFFX *)pContext;
	AVFrame *frame = avcodec_alloc_frame();
	INT got_picture, w, h;
	RECT rc1, rc2;
	GetClientRect(pThis->m_hWndtest, &rc1);
	SwsContext* sws = NULL;
	BYTE* bmp = NULL;
	BITMAPINFOHEADER hdr = 
	{
		sizeof(BITMAPINFOHEADER), 0, 0, 1, 24, BI_RGB, 0, 0, 0, 0, 0
	};
	BUFFER* buf = NULL;
	while(pThis->m_fRunning)
	{
		if (pThis->m_flag == 0)
		{
			Sleep(1000);
			continue;
		}
		//等待数据
		EnterCriticalSection(&pThis->m_csSync);
		if(!pThis->m_lstImage.size())
		{
			LeaveCriticalSection(&pThis->m_csSync);
			WaitForSingleObject(pThis->m_hEvent, 1000);
			continue;
		}
		buf = pThis->m_lstImage.front();
		pThis->m_lstImage.pop_front();
		LeaveCriticalSection(&pThis->m_csSync);

		if(NULL != buf && NULL != pThis->m_pFormatCtx)
		{	//解压数据
			avcodec_decode_video(pThis->m_pFormatCtx->streams[0]->codec, frame, &got_picture, buf->pbImage, buf->iSize);
			//回收内存
			EnterCriticalSection(&pThis->m_csSync);
			pThis->m_lstUnusedImage.push_back(buf);
			LeaveCriticalSection(&pThis->m_csSync);
			//解压成功
			if(got_picture)
			{
				GetClientRect(pThis->m_hWndtest, &rc2);
				if(NULL == sws || (rc1.right - rc1.left)/4*4 != (rc2.right - rc2.left)/4*4 || (rc1.bottom - rc1.top) != (rc2.bottom - rc2.top))

				{

					rc1 = rc2;
					w = (rc1.right - rc1.left)/4*4;
					h = rc1.bottom - rc1.top;
					hdr.biWidth = w;
					hdr.biHeight = -h;
					hdr.biSizeImage  = 3*w*h;
					//创建位图
					if(NULL != bmp)
					{
						delete []bmp;
					}
					bmp = new BYTE[hdr.biSizeImage];
					//缩放
					if(NULL != sws)
					{
						sws_freeContext(sws);
					}
					sws = sws_getContext(pThis->m_pFormatCtx->streams[0]->codec->width, pThis->m_pFormatCtx->streams[0]->codec->height, pThis->m_pFormatCtx->streams[0]->codec->pix_fmt, w, h, PIX_FMT_BGR24, 2, NULL, NULL, NULL);
				}
				if(NULL != sws)
				{	//缩放
					BYTE *data[4] = {bmp, 0, 0, 0};
					INT  pitch[4] = {3*w, 0, 0, 0};
					sws_scale(sws, frame->data, frame->linesize, 0, pThis->m_pFormatCtx->streams[0]->codec->height, data, pitch);
				}
				//显示
				if(NULL != bmp)
				{
					HDC hDC = ::GetDC(pThis->m_hWndtest);
					TRACE("%d",GetLastError());
					SetDIBitsToDevice(hDC, 0, 0, w, h, 0, 0, 0, h, bmp, (BITMAPINFO *)&hdr, DIB_RGB_COLORS);
					::ReleaseDC(pThis->m_hWndtest, hDC);
				}
			}
		}
	}
	if(NULL != frame)
	{
		//av_free(frame);
	}
	if(NULL != sws)
	{
		sws_freeContext(sws);
	}
	if(NULL != bmp)
	{
		delete []bmp;
	}
	if(NULL != pThis->m_pFormatCtx)
	{
		avcodec_flush_buffers(pThis->m_pFormatCtx->streams[0]->codec);
	}	
	return 0;
}

HRESULT CGFFX::Connect(VOID)
{
	if (!stricmp(m_szURL, "rtsp://0.0.0.0"))
	{
		return E_FAIL;
	}

	if(av_open_input_file(&m_pFormatCtx, m_szURL, NULL, 0, NULL))
	{				
		m_pFormatCtx = NULL;
		return E_FAIL;
	}

	m_pFormatCtx->max_analyze_duration = 1000;
	if(0 > av_find_stream_info(m_pFormatCtx) || !m_pFormatCtx->nb_streams || CODEC_TYPE_VIDEO != m_pFormatCtx->streams[0]->codec->codec_type)
	{
		av_close_input_file(m_pFormatCtx);
		m_pFormatCtx = NULL;
		return E_FAIL;
	}
	stream_component_open(m_pFormatCtx, 0);
	return S_OK;
}

int CGFFX::stream_component_open(AVFormatContext *ic, int stream_index)
{
	if (stream_index < 0 || stream_index >= (int)ic->nb_streams)
		return -1;

	AVCodecContext* enc = ic->streams[stream_index]->codec;
	AVCodec* codec = avcodec_find_decoder(enc->codec_id);

	enc->idct_algo			= FF_IDCT_AUTO;
	enc->flags2            |= CODEC_FLAG2_FAST;
	enc->skip_frame			= AVDISCARD_DEFAULT;
	enc->skip_idct			= AVDISCARD_DEFAULT;
	enc->skip_loop_filter	= AVDISCARD_DEFAULT;
	enc->error_concealment	= 3;

	if (!codec || avcodec_open(enc, codec) < 0)
		return -1;

	avcodec_thread_init(enc, 1);	
	enc->thread_count= 1;
	ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;

	return 0;
}

void CGFFX::H264control(int cmd)
{
	m_flag = cmd;
}