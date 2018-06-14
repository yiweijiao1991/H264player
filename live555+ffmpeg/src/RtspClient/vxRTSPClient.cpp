/*
* Copyright (c) 2014, 百年千岁
* All rights reserved.
* 
* 文件名称：vxRTSPClient.cpp
* 创建日期：2014年7月24日
* 文件标识：
* 文件摘要：RTSP 客户端工作的相关操作接口与数据类型。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2014年7月24日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* 
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "vxRTSPClient.h"
#include "stdafx.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
};
#endif // __cplusplus

#include <process.h>
#include <cassert>

#include "vxRTSPClient.inl"
#include "vxRTSPMemory.inl"

///////////////////////////////////////////////////////////////////////////

/**************************************************************************
* FunctionName:
*     destroy_thread
* Description:
*     销毁工作线程。
* Parameter:
*     @[in ] hthread: 线程句柄。
*     @[in ] timeout: 等待的超时时间。
* ReturnValue:
*     void
*/
static void destroy_thread(x_handle_t hthread, x_uint32_t xt_timeout)
{
	DWORD dwExitCode = 0;
	if ((X_NULL != hthread) && GetExitCodeThread(hthread, &dwExitCode) && (STILL_ACTIVE == dwExitCode))
	{
		DWORD dwWait = WaitForSingleObject(hthread, xt_timeout);
		if (WAIT_TIMEOUT == dwWait)
		{
			TerminateThread(hthread, -1);
		}
	}

	CloseHandle(hthread);
}
static char *get_time_string(void)   
{   
	time_t tt;
	struct tm *t;
	static char timebuf[64];

	time(&tt);
	t = gmtime(&tt); 
	sprintf(&timebuf[0],"%04d-%02d-%02d %02d:%02d:%02d ",
		t->tm_year + 1900, t->tm_mon+1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);

	return  &timebuf[0];  
}

//int log_write(fmt, args...)
int log_write(const char *pFormat, ...)
{	

	int 		ret;
	struct stat 	sgbuf;
	char 		*timestr;

	char		loginfo[1024] = {0};
	va_list		args;

	va_start(args, pFormat);
	vsprintf(loginfo,pFormat,args);
	va_end(args);
	FILE* fp = fopen("./H264.log", "ab+");
	if(fp == NULL)
	{
		return -1;
	}

	timestr = get_time_string();
	fwrite(timestr,strlen(timestr), 1, fp);
	fwrite(loginfo,strlen(loginfo), 1, fp);
	fwrite("\r\n",strlen("\r\n"), 1, fp);
	fclose(fp);

	return 0;	
}

///////////////////////////////////////////////////////////////////////////
// vxFFmpegDecode definition

class vxFFmpegDecode
{
	// constructor/destructor
public:
	vxFFmpegDecode(void);
	~vxFFmpegDecode(void);

	// public interfaces
public:
	/**************************************************************************
	* FunctionName:
	*     initial
	* Description:
	*     对象初始化操作。
	* Parameter:
	*     @[in ] xt_decode_id: 解码器 ID。
	* ReturnValue:
	*     成功，返回 x_err_ok；失败，返回 错误码。
	*/
	x_int32_t initial(x_int32_t xt_decode_id = AV_CODEC_ID_H264);

	/**************************************************************************
	* FunctionName:
	*     release
	* Description:
	*     对象反初始化操作。
	*/
	void release(void);

	/**************************************************************************
	* FunctionName:
	*     input_nalu_data
	* Description:
	*     输入单个 NALU 数据包执行解码。
	* Parameter:
	*     @[in ] xt_buf: 数据包缓存。
	*     @[in ] xt_size: 数据包缓存长度。
	*     @[in ] xt_flip: 标识是否对解码图像进行垂直翻转。
	* ReturnValue:
	*     返回 x_true，表示完成一帧完整的图像解码工作；返回 X_FALSE, 表示未完成。
	*/
	x_bool_t input_nalu_data(x_uint8_t * xt_buf, x_uint32_t xt_size, x_bool_t xt_flip,WT_ImageInfo_t	*imageInfo,enum WT_PixelFormat_t m_PixelFormat);

	/**************************************************************************
	* FunctionName:
	*     decode_buf
	* Description:
	*     返回 解码输出图像数据 的缓存地址。
	*/
	inline x_uint8_t * decode_data(void) { return (x_uint8_t *)m_xmem_decode.data(); }

	/**************************************************************************
	* FunctionName:
	*     decode_width
	* Description:
	*     返回 解码输出图像 的宽度。
	*/
	inline x_int32_t decode_width(void) const { return m_xt_width; }

	/**************************************************************************
	* FunctionName:
	*     decode_height
	* Description:
	*     返回 解码输出图像 的高度。
	*/
	inline x_int32_t decode_height(void) const { return m_xt_height; }

	// class data
protected:
	AVCodecContext    * m_avcodec_context_ptr;
	AVCodec           * m_avcodec_ptr;
	AVFrame           * m_avframe_decode;
	AVFrame           * m_avframe_rgb_buf;
	struct SwsContext * m_sws_context_ptr;
	AVPacket            m_avpacket_decode;

	x_int32_t           m_xt_width;
	x_int32_t           m_xt_height;
	xmemblock           m_xmem_decode;

};

///////////////////////////////////////////////////////////////////////////
// vxFFmpegDecode implementation

//=========================================================================

// 
// vxFFmpegDecode constructor/destructor
// 

vxFFmpegDecode::vxFFmpegDecode(void)
			: m_avcodec_context_ptr(X_NULL)
			, m_avcodec_ptr(X_NULL)
			, m_avframe_decode(X_NULL)
			, m_avframe_rgb_buf(X_NULL)
			, m_sws_context_ptr(X_NULL)
			, m_xt_width(0)
			, m_xt_height(0)
{

}

vxFFmpegDecode::~vxFFmpegDecode(void)
{
	release();
}

//=========================================================================

// 
// vxFFmpegDecode public interfaces
// 

/**************************************************************************
* FunctionName:
*     initial
* Description:
*     对象初始化操作。
* Parameter:
*     @[in ] xt_decode_id: 解码器 ID。
* ReturnValue:
*     成功，返回 x_err_ok；失败，返回 错误码。
*/
x_int32_t vxFFmpegDecode::initial(x_int32_t xt_decode_id /*= AV_CODEC_ID_H264*/)
{
	x_int32_t xt_err = X_ERR_UNKNOW;

	do 
	{
		// 参数有效判断
		if ((X_NULL != m_avcodec_context_ptr) ||
			(X_NULL != m_avcodec_ptr) ||
			(X_NULL != m_avframe_decode))
		{
			break;
		}

		// 初始化 ffmpeg 解码参量
		av_init_packet(&m_avpacket_decode);
/*
		m_avcodec_context_ptr = avcodec_alloc_context3(avcodec_find_decoder((AVCodecID)xt_decode_id));
		if (X_NULL == m_avcodec_context_ptr)
		{
			break;
		}
*/
		// 解码器
		m_avcodec_ptr = avcodec_find_decoder((AVCodecID)xt_decode_id);
		if (X_NULL == m_avcodec_ptr)
		{
			break;
		}

		m_avcodec_context_ptr = avcodec_alloc_context3(m_avcodec_ptr);
		if (X_NULL == m_avcodec_context_ptr)
		{
			break;
		}

		//if (avcodec_open(m_avcodec_context_ptr, m_avcodec_ptr) < 0)
		if (avcodec_open2(m_avcodec_context_ptr, m_avcodec_ptr,0) < 0)
		{
			break;
		}

		// 解码输出帧
		m_avframe_decode = avcodec_alloc_frame();
		if (X_NULL == m_avframe_decode)
		{
			break;
		}

		xt_err = X_ERR_OK;
	} while (0);

	return xt_err;
}

/**************************************************************************
* FunctionName:
*     release
* Description:
*     对象反初始化操作。
*/
void vxFFmpegDecode::release(void)
{
	av_free_packet(&m_avpacket_decode);

	if (X_NULL != m_avcodec_context_ptr)
	{
		avcodec_close(m_avcodec_context_ptr);
		av_free(m_avcodec_context_ptr);
		m_avcodec_context_ptr = X_NULL;
	}

	m_avcodec_ptr = X_NULL;



	if (X_NULL != m_avframe_decode)
	{
		av_free(m_avframe_decode);
		m_avframe_decode = X_NULL;
	}

	if (X_NULL != m_avframe_rgb_buf)
	{
		av_free(m_avframe_rgb_buf);
		m_avframe_rgb_buf = X_NULL;
	}
	if (X_NULL != m_sws_context_ptr)
	{
		sws_freeContext(m_sws_context_ptr);
		m_sws_context_ptr = X_NULL;
	}
}

/**************************************************************************
* FunctionName:
*     input_nalu_data
* Description:
*     输入单个 NALU 数据包执行解码。
* Parameter:
*     @[in ] xt_buf: 数据包缓存。
*     @[in ] xt_size: 数据包缓存长度。
*     @[in ] xt_flip: 标识是否对解码图像进行垂直翻转。
* ReturnValue:
*     返回 x_true，表示完成一帧完整的图像解码工作；返回 X_FALSE, 表示未完成。
*/
x_bool_t vxFFmpegDecode::input_nalu_data(x_uint8_t * xt_buf, x_uint32_t xt_size, x_bool_t xt_flip,WT_ImageInfo_t	*imageInfo,enum WT_PixelFormat_t m_PixelFormat)
{
	
	m_avpacket_decode.data = xt_buf;
	m_avpacket_decode.size = (int)xt_size;

	int decoded_frame = 0;
	avcodec_decode_video2(m_avcodec_context_ptr, m_avframe_decode, &decoded_frame, &m_avpacket_decode);
	if (0 != decoded_frame)
	{
		// 保存解码后的图像宽度和高度
		m_xt_width  = m_avcodec_context_ptr->width;
		m_xt_height = m_avcodec_context_ptr->height;
		imageInfo->uWidth=m_avcodec_context_ptr->width;
		imageInfo->uHeight= m_avcodec_context_ptr->height;
		if (m_PixelFormat == WT_PIX_FMT_YUV420P)
		{
			imageInfo->pBuffer[0] = m_avframe_decode->data[0];
			imageInfo->pBuffer[1] = m_avframe_decode->data[1];
			imageInfo->pBuffer[2] = m_avframe_decode->data[2];
			imageInfo->uPitch[0]= m_avframe_decode->linesize[0];
			imageInfo->uPitch[1]= m_avframe_decode->linesize[1];
			imageInfo->uPitch[2]= m_avframe_decode->linesize[2];
			imageInfo->uPixFmt = WT_PIX_FMT_YUV420P;
		}


		if (xt_flip)
		{
			// 设置为垂直翻转方式转换成 RGB 格式数据
			m_avframe_decode->data[0] = m_avframe_decode->data[0] + m_avframe_decode->linesize[0] * (m_avcodec_context_ptr->height - 1);
			m_avframe_decode->data[1] = m_avframe_decode->data[1] + m_avframe_decode->linesize[1] * (m_avcodec_context_ptr->height / 2 - 1);
			m_avframe_decode->data[2] = m_avframe_decode->data[2] + m_avframe_decode->linesize[2] * (m_avcodec_context_ptr->height / 2 - 1);
			m_avframe_decode->linesize[0] *= -1;
			m_avframe_decode->linesize[1] *= -1;
			m_avframe_decode->linesize[2] *= -1;
		}

		if (X_NULL == m_avframe_rgb_buf)
		{
			m_xmem_decode.auto_resize(4 * m_xt_width * m_xt_height);

			m_avframe_rgb_buf = avcodec_alloc_frame();
			if (X_NULL == m_avframe_rgb_buf)
			{
				return X_FALSE;
			}

			avpicture_fill((AVPicture *)m_avframe_rgb_buf, (uint8_t *)m_xmem_decode.data(), PIX_FMT_BGR24, m_avcodec_context_ptr->width, m_avcodec_context_ptr->height);
		}

		if (X_NULL == m_sws_context_ptr)
		{
			m_sws_context_ptr = sws_getContext(m_avcodec_context_ptr->width, m_avcodec_context_ptr->height, PIX_FMT_YUV420P,
										m_avcodec_context_ptr->width, m_avcodec_context_ptr->height, PIX_FMT_BGR24, SWS_BICUBIC, X_NULL, X_NULL, X_NULL);
			if (X_NULL == m_sws_context_ptr)
			{
				return X_FALSE;
			}
		}

		// 将 YUV 数据格式转换成 RGB 数据
		try
		{
			sws_scale(m_sws_context_ptr, m_avframe_decode->data, m_avframe_decode->linesize, 0, m_avcodec_context_ptr->height, m_avframe_rgb_buf->data, m_avframe_rgb_buf->linesize);
		}
		catch (CMemoryException* e)
		{
			log_write("input_nalu_data()::sws_scale error CMemoryException ");
			return X_FALSE;
		}
		catch (CFileException* e)
		{
				log_write("input_nalu_data()::sws_scale error CFileException ");
			return X_FALSE;
		}
		catch (CException* e)
		{
			log_write("input_nalu_data()::sws_scale error CException ");
			return X_FALSE;
		}
		if (m_PixelFormat == WT_PIX_FMT_BGR24)
		{
			imageInfo->pBuffer[0] =  (uint8_t *)m_xmem_decode.data();
			imageInfo->uPitch[0]= (int) m_avframe_rgb_buf->linesize;
			imageInfo->uPixFmt = WT_PIX_FMT_BGR24;
		}
	
	}

	return (0 != decoded_frame);

}

///////////////////////////////////////////////////////////////////////////
// vxRTSPClient

//=========================================================================

// 
// vxRTSPClient static invoking methods
// 

/**************************************************************************
* FunctionName:
*     initial_lib
* Description:
*     库初始化操作。
* Parameter:
*     @[in ] xt_pv_param: 预留参数。
* ReturnValue:
*     成功，返回 x_err_ok；失败，返回 错误码。
*/
x_int32_t vxRTSPClient::initial_lib(x_handle_t xt_pv_param)
{
	x_int32_t xt_err = X_ERR_UNKNOW;

	do 
	{
		av_register_all();

		xt_err = X_ERR_OK;
	} while (0);

	return xt_err;
}

/**************************************************************************
* FunctionName:
*     uninitial_lib
* Description:
*     库反初始化操作。
*/
void vxRTSPClient::uninitial_lib(void)
{

}

/**************************************************************************
* FunctionName:
*     thread_work_recv
* Description:
*     数据采集的线程入口函数。
*/
x_uint32_t __stdcall vxRTSPClient::thread_work_recv(x_handle_t pv_param)
{
	((vxRTSPClient *)pv_param)->recv_loop();
	return 0;
}

/**************************************************************************
* FunctionName:
*     thread_work_decode
* Description:
*     数据解码线程入口函数。
*/
x_uint32_t __stdcall vxRTSPClient::thread_work_decode(x_handle_t pv_param)
{
	((vxRTSPClient *)pv_param)->decode_loop();
	return 0;
}

/**************************************************************************
* FunctionName:
*     thread_work_decode
* Description:
*     数据解码线程入口函数。
*/
x_uint32_t __stdcall vxRTSPClient::thread_ReConnect(x_handle_t pv_param)
{
	((vxRTSPClient *)pv_param)->reconnect_loop();
	return 0;
}

/**************************************************************************
* FunctionName:
*     realframe_cbk_entry
* Description:
*     实时数据帧回调接口。
*/
void vxRTSPClient::realframe_cbk_entry(x_handle_t xt_handle, x_handle_t xt_buf, x_uint32_t xt_size, x_uint32_t xt_type, x_handle_t xt_user)
{
	vxRTSPClient * pv_this = (vxRTSPClient *)xt_user;
	pv_this->realframe_proc(xt_handle, xt_buf, xt_size, xt_type);
}

//=========================================================================

// 
// vxRTSPClient constructor/destructor
// 

vxRTSPClient::vxRTSPClient(void)
			: m_xt_hthread_recv(X_NULL)
			, m_xt_hthread_decode(X_NULL)
			, m_xt_bexitflag(X_TRUE)
			, m_xt_rtsp_client(X_NULL)
			, m_xt_realframe_queue(X_NULL)
			, m_xt_real_context_valid(X_FALSE)
			, m_xt_real_context_info(X_NULL)
			, m_xt_rtsp_url(X_NULL)
			, m_xt_width(0)
			, m_xt_height(0)
			, m_xt_flip(X_FALSE)
			, m_xfunc_realcbk(X_NULL)
{
	// 构建环形内存队列
	m_xt_realframe_queue = (x_handle_t)(new xmemblock_cirqueue());
	if (X_NULL == m_xt_realframe_queue)
	{
		assert(false);
	}

	m_xt_real_context_info = (x_handle_t)(new xmemblock());
	if (X_NULL == m_xt_real_context_info)
	{
		assert(false);
	}

	x_uint32_t xt_tid;
	m_disconnect_times=0;
	m_getdata=0;
	memset(m_rtsp_url,0,200);
	m_WorkStatue = 0; //停止工作中

	 m_decodeThreadStatue = 0; // 0停止中 
	 m_ReciveThreadStatue = 0; // 0停止中 


	// 创建重连线程
	m_xt_hthread_reconnect = (x_handle_t)_beginthreadex(X_NULL, 0, &vxRTSPClient::thread_ReConnect, this, 0, &xt_tid);


	 m_hwnd = NULL;

	 m_Enable = 0;
	 m_H264handle = 0;
}

vxRTSPClient::~vxRTSPClient(void)
{
	xmemblock * xt_mblk = (xmemblock *)m_xt_real_context_info;
	if (X_NULL != xt_mblk)
	{
		delete xt_mblk;
		xt_mblk = X_NULL;

		m_xt_real_context_info = X_NULL;
	}

	// 清理环形队列缓存
	xmemblock_cirqueue * xt_cirqueue = (xmemblock_cirqueue *)m_xt_realframe_queue;
	if (X_NULL != xt_cirqueue)
	{
		xt_cirqueue->clear_cir_queue();
		delete xt_cirqueue;
		xt_cirqueue = X_NULL;

		m_xt_realframe_queue = X_NULL;
	}

	close();

	// 销毁重连线程
	if (X_NULL != m_xt_hthread_reconnect)
	{
		destroy_thread(m_xt_hthread_reconnect, INFINITE);
		m_xt_hthread_reconnect = X_NULL;
	}


}

//=========================================================================

// 
// vxRTSPClient public interfaces
// 

/**************************************************************************
* FunctionName:
*     open
* Description:
*     RTSP地址的打开操作。
* Parameter:
*     @[in ] xt_rtsp_url: RTSP URL 地址。
*     @[in ] xfunc_realcbk: 解码数据回调函数接口。

* ReturnValue:
*     成功，返回 0；失败，返回 错误码。
*/
x_int8_t vxRTSPClient::open(const x_string_t xt_rtsp_url,HWND hwnd,enum WT_PixelFormat_t pixelFormat, int nEnable,WT_H264HANDLE handle)
{
	x_int8_t xt_err = X_ERR_UNKNOW;
	m_WorkStatue = 1; //用户要求开始工作
	do 
	{
		// 参数有效判断
		if ((X_NULL != m_xt_hthread_recv)    ||
			(X_NULL != m_xt_hthread_decode)  ||
			(X_NULL != m_xt_rtsp_client))
		{
			break;
		}

		// 保存参数
		m_xt_rtsp_url   = _strdup(xt_rtsp_url);
		memset(m_rtsp_url,0,200);
		strcpy(m_rtsp_url,m_xt_rtsp_url);	
		m_hwnd = hwnd;
		m_PixelFormat = pixelFormat;
		m_xt_bexitflag  = X_FALSE;
		m_Enable = nEnable;
		m_H264handle = handle;
		// 构建 RTSP 数据接收的客户端工作对象
		m_xt_rtsp_client = (x_handle_t)vxRtspCliHandle::create(xt_rtsp_url);
		if (X_NULL == m_xt_rtsp_client)
		{
			break;
		}

		// 设置数据回调接口
		((vxRtspCliHandle *)m_xt_rtsp_client)->set_recved_realframe_cbk((CLI_REALFRAME_CBK)&vxRTSPClient::realframe_cbk_entry, this);

		x_uint32_t xt_tid;

		// 创建数据接收的工作线程
		m_xt_hthread_recv = (x_handle_t)_beginthreadex(X_NULL, 0, &vxRTSPClient::thread_work_recv, this, 0, &xt_tid);
		if (X_NULL == m_xt_hthread_recv)
		{
			break;
		}
		CloseHandle(m_xt_hthread_recv);
		m_xt_hthread_recv = X_NULL;
		// 创建数据解码的工作线程
		m_xt_hthread_decode = (x_handle_t)_beginthreadex(X_NULL, 0, &vxRTSPClient::thread_work_decode, this, 0, &xt_tid);
		if (X_NULL == m_xt_hthread_decode)
		{
			break;
		}
		CloseHandle(m_xt_hthread_decode);
		m_xt_hthread_decode = X_NULL;
		// 重置 视频头的描述信息
		m_xt_real_context_valid = X_FALSE;
		if (X_NULL != m_xt_real_context_info)
		{
			((xmemblock *)m_xt_real_context_info)->reset();
		}

		xt_err = X_ERR_OK;
	} while (0);

	if (X_ERR_OK != xt_err)
	{
		DestroySource();
	}

	return xt_err;
}


void vxRTSPClient::DestroySource(void)
{
	// 设置退出标识
	m_xt_bexitflag = X_TRUE;
	//m_getdata=0;
	int times = 0;
	//判断这两个线程是否已经退出
	while(m_decodeThreadStatue || m_ReciveThreadStatue)
	{
		Sleep(10);
		times++;
		if(times > 300) // 如果3秒了那么强制退出
			break;
	}
	try{

		// 删除 RTSP 数据接收的客户端对象
		if (X_NULL != m_xt_rtsp_client)
		{
			vxRtspCliHandle::destroy((vxRtspCliHandle *)m_xt_rtsp_client);
			m_xt_rtsp_client = X_NULL;
		}
		// 重置 视频头的描述信息
		m_xt_real_context_valid = X_FALSE;
		if (X_NULL != m_xt_real_context_info)
		{
			((xmemblock *)m_xt_real_context_info)->reset();
		}

		if (X_NULL != m_xt_rtsp_url)
		{
			free(m_xt_rtsp_url);
			m_xt_rtsp_url = X_NULL;
		}

	}
	catch (CMemoryException* e)
	{
		log_write("DestroySource()::DestroySource error CMemoryException ");
		
	}
	catch (CFileException* e)
	{
		log_write("DestroySource()::DestroySource error CFileException ");
		
	}
	catch (CException* e)
	{
		log_write("DestroySource()::DestroySource error CException ");
		
	}
	m_xt_width  = 0;
	m_xt_height = 0;
}
/**************************************************************************
* FunctionName:
*     close
* Description:
*     对象关闭操作。
*/
void vxRTSPClient::close(void)
{
 
	m_WorkStatue = 0; //用户要求停止工作
	DestroySource();//销毁
}

/**************************************************************************
* FunctionName:
*     set_max_cached_block_nums
* Description:
*     设置最大缓存内存块的数量。
* Parameter:
*     @[in ] xt_max_nums: 最大缓存内存块的数量。
* ReturnValue:
*     void
*/
void vxRTSPClient::set_max_cached_block_nums(x_uint32_t xt_max_nums)
{
	if (X_NULL != m_xt_realframe_queue)
	{
		((xmemblock_cirqueue *)m_xt_realframe_queue)->resize_max_blocks(xt_max_nums);
	}
}

/**************************************************************************
* FunctionName:
*     get_max_cached_block_nums
* Description:
*     获取最大缓存内存块的数量。
*/
x_uint32_t vxRTSPClient::get_max_cached_block_nums(void) const
{
	if (X_NULL != m_xt_realframe_queue)
	{
		return (x_uint32_t)((xmemblock_cirqueue *)m_xt_realframe_queue)->max_blocks_size();
	}

	return 0;
}

/**************************************************************************
* FunctionName:
*     get_realframe_context
* Description:
*     获取 视频编码描述信息。
* Parameter:
*     @[out] xt_buf: 信息输出缓存。
*     @[in out] xt_size: 入参，信息输出缓存的大小；回参，视频编码描述信息的大小。
* ReturnValue:
*     成功，返回 X_TRUE；失败，返回 X_FALSE。
*/
x_bool_t vxRTSPClient::get_realframe_context(x_uint8_t * xt_buf, x_uint32_t & xt_size)
{
	xmemblock * xmblk = (xmemblock *)m_xt_real_context_info;
	if ((X_NULL == xmblk) || (X_TRUE != m_xt_real_context_valid))
	{
		return X_FALSE;
	}

	if ((X_NULL != xt_buf) && (xt_size >= xmblk->size()))
	{
		memcpy(xt_buf, xmblk->data(), xmblk->size());
	}

	xt_size = xmblk->size();

	return X_TRUE;
}

//=========================================================================

// 
// vxRTSPClient inner invoking methods
// 

/**************************************************************************
* FunctionName:
*     recv_loop
* Description:
*     数据接收的事件处理流程（仅由 thread_work_recv() 接口回调该操作）。
*/
void vxRTSPClient::recv_loop(void)
{
	
	vxRtspCliHandle * rtsp_client_handle = (vxRtspCliHandle *)m_xt_rtsp_client;
	if (X_NULL == rtsp_client_handle)
	{
		return;
	}
	m_ReciveThreadStatue = 1;
	rtsp_client_handle->do_event_loop(&m_xt_bexitflag);
	rtsp_client_handle->shutdown_stream();

/*
	// 删除 RTSP 数据接收的客户端对象
	if (X_NULL != m_xt_rtsp_client)
	{
		vxRtspCliHandle::destroy((vxRtspCliHandle *)m_xt_rtsp_client);
		m_xt_rtsp_client = X_NULL;
	}
	
	//m_xt_hthread_recv = X_NULL;
	*/
	m_xt_bexitflag = X_TRUE;
	m_ReciveThreadStatue = 0;
}

/**************************************************************************
* FunctionName:
*     decode_loop
* Description:
*     数据解码的事件处理流程（仅由 thread_work_recv() 接口回调该操作）。
*/
void vxRTSPClient::decode_loop(void)
{

	WT_H264Decode_t h264_decode;
	memset(&h264_decode,0,sizeof(h264_decode));
	h264_decode.handle = m_H264handle;


	xmemblock * x_block_ptr = X_NULL;
	// 数据帧接收的环形缓存队列
	xmemblock_cirqueue * x_cirqueue_ptr = (xmemblock_cirqueue *)m_xt_realframe_queue;
	if (X_NULL == x_cirqueue_ptr)
	{
		return;
	}

	// ffmpeg 解码操作对象
	vxFFmpegDecode ffdecode;

	// 初始化 ffmpeg 解码参量
	if (X_ERR_OK != ffdecode.initial(AV_CODEC_ID_H264))
	{
		return;
	}
	m_decodeThreadStatue = 1;
	// 循环处理接收到的数据帧，执行解码、回调等工作
	while (!is_exit_work())
	{
		// 取帧操作
		x_block_ptr = x_cirqueue_ptr->pop_front_from_saved_queue();
		if (X_NULL == x_block_ptr)
		{
			Sleep(5);
			//m_getdata = 0;
			continue;
		}
		m_getdata = 1;
		//m_disconnect_times = 0;
		// 输入数据帧
		if (ffdecode.input_nalu_data((x_uint8_t *)x_block_ptr->data(), x_block_ptr->size(), m_xt_flip,&(h264_decode.imageInfo),m_PixelFormat))
		{
			m_xt_width  = ffdecode.decode_width();
			m_xt_height = ffdecode.decode_height();
			// 数据回调
			if (X_NULL != m_xfunc_realcbk)
			{

				m_xfunc_realcbk(&h264_decode);
			}
			if(m_Enable && m_PixelFormat == WT_PIX_FMT_BGR24)
			{
				//显示
				ShowImage(ffdecode.decode_data(), 3 * m_xt_width * m_xt_height,m_xt_width,m_xt_height);

			}
		}

		// 回收帧内存块
		x_cirqueue_ptr->recyc(x_block_ptr);
		x_block_ptr = X_NULL;


	}
	if (X_NULL != x_block_ptr)
	{
		x_cirqueue_ptr->recyc(x_block_ptr);
	}

	x_cirqueue_ptr->clear_cir_queue();
	//CloseHandle(m_xt_hthread_decode);
	//m_xt_hthread_decode = X_NULL;
	m_xt_width  = 0;
	m_xt_height = 0;
	m_xt_bexitflag = X_TRUE;
	m_decodeThreadStatue = 0;

}

void vxRTSPClient::reconnect_loop(void)
{
	// 循环处理接收到的数据帧，执行解码、回调等工作
	while (1)
	{
		
		if ((m_WorkStatue == 1))//判断用户是否期望它工作如果是那么就进行重连   ；如果用户主动关闭了那么就是不期望继续工作
		{
			if(m_getdata == 0)//判断是否获取到了数据
			{
				m_disconnect_times++;//没有数据那么累计
			}else{
			
				m_disconnect_times = 0;//有数据那么清空累计
			}
				
			m_getdata = 0;//清空标识
			if(m_disconnect_times > _RECONNECT_TIME_)//累计超过了一定时间
			{
				//重新连接
				DestroySource();//销毁资源
				Sleep(5000);
				real_decode_enable_flip(0);
				open(m_rtsp_url,m_hwnd,m_PixelFormat,m_Enable,m_H264handle);//重新打开流
				m_disconnect_times = 0;//重新计数
			} 
			
		}
		Sleep(1000);//线程时间每秒一次
	}
}
/**************************************************************************
* FunctionName:
*     realframe_proc
* Description:
*     实时数据帧回调接收处理流程（仅由 realframe_cbk_entry() 接口回调该操作）。
*/
void vxRTSPClient::realframe_proc(x_handle_t xt_handle, x_handle_t xt_buf, x_uint32_t xt_size, x_uint32_t xt_type)
{
	if (m_xt_rtsp_client != xt_handle)
	{
		return;
	}

	xmemblock_cirqueue * x_cirqueue_ptr = (xmemblock_cirqueue *)m_xt_realframe_queue;
	if (X_NULL == x_cirqueue_ptr)
	{
		return;
	}

	xmemblock * x_block_ptr = x_cirqueue_ptr->alloc();
	if (X_NULL == x_block_ptr)
	{
		return;
	}

	x_uint8_t xt_start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	x_block_ptr->write_block(xt_start_code, 4 * sizeof(x_uint8_t));
	x_block_ptr->append_data(xt_buf, xt_size);

	if (RTSP_FRAMETYPE_RCDPARAM == xt_type)
	{

		
		if ((X_NULL != m_xt_real_context_info) && (X_NULL != xt_buf))
		{
			((xmemblock *)m_xt_real_context_info)->append_data(x_block_ptr->data(), x_block_ptr->size());
		}

		m_xt_real_context_valid = (X_NULL == xt_buf);


	}

	// 实时 H264 码流回调
	if (X_NULL != m_xfunc_realcbk)
	{
		//m_xfunc_realcbk(X_REAL_TYPE_H264, (x_uint8_t *)x_block_ptr->data(), x_block_ptr->size(), 0, 0, m_xt_user);
	}

	x_cirqueue_ptr->push_back_to_saved_queue(x_block_ptr);
}


void vxRTSPClient::ShowImage(unsigned char *pFrameRGB,UINT nSize, int nWidth, int nHeight)
{


	CRect rc;
	HDC hDC = GetDC(m_hwnd);
	CDC *pDC = NULL;
	BITMAPINFO m_bmphdr={0};
	DWORD dwBmpHdr = sizeof(BITMAPINFO);
	if(hDC !=NULL)
	{
		pDC = CDC::FromHandle(hDC);
	

		m_bmphdr.bmiHeader.biBitCount = 24;
		m_bmphdr.bmiHeader.biClrImportant = 0;
		m_bmphdr.bmiHeader.biSize = dwBmpHdr;
		m_bmphdr.bmiHeader.biSizeImage = 0;
		m_bmphdr.bmiHeader.biWidth = nWidth;
		m_bmphdr.bmiHeader.biHeight = -nHeight;
		m_bmphdr.bmiHeader.biXPelsPerMeter = 0;
		m_bmphdr.bmiHeader.biYPelsPerMeter = 0;
		m_bmphdr.bmiHeader.biClrUsed = 0;
		m_bmphdr.bmiHeader.biPlanes = 1;
		m_bmphdr.bmiHeader.biCompression = BI_RGB;
 
		if(hDC !=NULL)
		{
			GetWindowRect(m_hwnd,rc);
			pDC->SetStretchBltMode(HALFTONE);

			StretchDIBits(hDC,
				0,0,
				rc.Width(),rc.Height(),
				0, 0,
				nWidth, nHeight,
				pFrameRGB,
				&m_bmphdr,
				DIB_RGB_COLORS,
				SRCCOPY);
		}
		//pDC->DeleteDC();	
		ReleaseDC(m_hwnd,hDC);		
	}

}