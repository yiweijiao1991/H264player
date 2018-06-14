/*
* Copyright (c) 2014, 百年千岁
* All rights reserved.
* 
* 文件名称：vxRTSPClient.h
* 创建日期：2014年7月24日
* 文件标识：
* 文件摘要：RTSP 客户端工作的相关操作类接口与数据类型。
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

#ifndef __VXRTSPCLIENT_H__
#define __VXRTSPCLIENT_H__
#include "WT_H264.h"

///////////////////////////////////////////////////////////////////////////

#ifdef RTSPCLIENT_EXPORTS
#define RTSPCLIENT_API __declspec(dllexport)
#else
#define RTSPCLIENT_API __declspec(dllimport)
#endif

///////////////////////////////////////////////////////////////////////////



#define X_FALSE           0
#define X_TRUE            1
#define X_NULL            0
#define X_ERR_OK          0
#define X_ERR_UNKNOW      (-1)

#define X_REAL_TYPE_H264  1000  //264数据
#define X_REAL_TYPE_RGB   1001	//RGB数据


///////////////////////////////////////////////////////////////////////////

typedef char              x_int8_t;
typedef unsigned char     x_uint8_t;
typedef short             x_int16_t;
typedef unsigned short    x_uint16_t;
typedef int               x_int32_t;
typedef unsigned int      x_uint32_t;
typedef unsigned int      x_bool_t;
typedef void *            x_handle_t;

typedef char *            x_string_t;

#define  _RECONNECT_TIME_  30

class RTSPCLIENT_API vxRTSPClient
{
	// constructor/destructor
public: 
	vxRTSPClient(void);
	virtual ~vxRTSPClient(void);

	// static invoking methods
public:
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
	static x_int32_t initial_lib(x_handle_t xt_pv_param);

	/**************************************************************************
	* FunctionName:
	*     uninitial_lib
	* Description:
	*     库反初始化操作。
	*/
	static void uninitial_lib(void);

protected:
	/**************************************************************************
	* FunctionName:
	*     thread_work_recv
	* Description:
	*     数据采集的线程入口函数。
	*/
	static x_uint32_t __stdcall thread_work_recv(x_handle_t pv_param);

	/**************************************************************************
	* FunctionName:
	*     thread_work_decode
	* Description:
	*     数据解码线程入口函数。
	*/
	static x_uint32_t __stdcall thread_work_decode(x_handle_t pv_param);

	//add by yiweijiao
	/**************************************************************************
	* FunctionName:
	*     thread_work_decode
	* Description:
	*     数据解码线程入口函数。
	*/
	static x_uint32_t __stdcall thread_ReConnect(x_handle_t pv_param);

	/**************************************************************************
	* FunctionName:
	*     realframe_cbk_entry
	* Description:
	*     实时数据帧回调接口。
	*/
	static void realframe_cbk_entry(x_handle_t xt_handle, x_handle_t xt_buf, x_uint32_t xt_size, x_uint32_t xt_type, x_handle_t xt_user);

	// public interfaces
public:
	/**************************************************************************
	* FunctionName:
	*     open
	* Description:
	*     RTSP地址的打开操作。
	* Parameter:
	*     @[in ] xt_rtsp_url: RTSP URL 地址。
	*     @[in ] xfunc_realcbk: 解码数据回调函数接口。
	*    
	* ReturnValue:
	*     成功，返回 0；失败，返回 错误码。
	*/
	x_int8_t open(const x_string_t xt_rtsp_url,HWND hwnd,enum WT_PixelFormat_t pixelFormat, int nEnable,WT_H264HANDLE handle);

	/**************************************************************************
	* FunctionName:
	*     close
	* Description:
	*     对象关闭操作。
	*/
	void close(void);

	/**************************************************************************
	* FunctionName:
	*     is_working
	* Description:
	*     返回是否处于工作状态。
	*/
	inline x_bool_t is_working(void) const
	{
		return (!m_xt_bexitflag);
	}

	/**************************************************************************
	* FunctionName:
	*     real_decode_width
	* Description:
	*     实时解码时得到的图像宽度。
	*/
	inline x_int32_t real_decode_width(void) const { return m_xt_width; }

	/**************************************************************************
	* FunctionName:
	*     real_decode_height
	* Description:
	*     实时解码时得到的图像高度。
	*/
	inline x_int32_t real_decode_height(void) const { return m_xt_height; }

	/**************************************************************************
	* FunctionName:
	*     real_decode_enable_flip
	* Description:
	*     实时解码时，是否对图像进行垂直翻转。
	*/
	inline void real_decode_enable_flip(x_bool_t xt_enable) { m_xt_flip = xt_enable; }

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
	void set_max_cached_block_nums(x_uint32_t xt_max_nums);

	/**************************************************************************
	* FunctionName:
	*     get_max_cached_block_nums
	* Description:
	*     获取最大缓存内存块的数量。
	*/
	x_uint32_t get_max_cached_block_nums(void) const;

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
	x_bool_t get_realframe_context(x_uint8_t * xt_buf, x_uint32_t & xt_size);

	// inner invoking methods

protected:
	/**************************************************************************
	* FunctionName:
	*     recv_loop
	* Description:
	*     数据接收的事件处理流程（仅由 thread_work_recv() 接口回调该操作）。
	*/
	void recv_loop(void);

	/**************************************************************************
	* FunctionName:
	*     decode_loop
	* Description:
	*     数据解码的事件处理流程（仅由 thread_work_recv() 接口回调该操作）。
	*/
	void decode_loop(void);

	/**************************************************************************
	* FunctionName:
	*     reconnect_loop
	* Description:
	*     数据解码的事件处理流程（仅由 thread_work_recv() 接口回调该操作）。
	*/
	void reconnect_loop(void);

	/**************************************************************************
	* FunctionName:
	*     realframe_proc
	* Description:
	*     实时数据帧回调接收处理流程（仅由 realframe_cbk_entry() 接口回调该操作）。
	*/
	void realframe_proc(x_handle_t xt_handle, x_handle_t xt_buf, x_uint32_t xt_size, x_uint32_t xt_type); 

	/**************************************************************************
	* FunctionName:
	*     is_exit_work
	* Description:
	*     返回工作线程的退出标识（是否退出工作状态）。
	*/
	inline x_bool_t is_exit_work(void) const { return (x_bool_t)m_xt_bexitflag; }

	// class data
protected:
	x_handle_t       m_xt_hthread_recv;         ///< RTSP 数据采集线程
	x_handle_t       m_xt_hthread_decode;       ///< H264 数据解码线程
	x_handle_t       m_xt_hthread_reconnect;       ///重连线程
	x_int8_t         m_xt_bexitflag;            ///< 工作线程的退出标识

	x_handle_t       m_xt_rtsp_client;          ///< RTSP 客户端操作对象
	x_handle_t       m_xt_realframe_queue;      ///< 接收回调的视频数据帧的环形队列

	x_bool_t         m_xt_real_context_valid;   ///< 标识当前接收视频头的描述信息是否已经有效
	x_handle_t       m_xt_real_context_info;    ///< 接收视频头的描述信息

	x_string_t       m_xt_rtsp_url;             ///< 打开的 RTSP URL 地址

	x_int32_t        m_xt_width;                ///< 解码时得到的图像宽度
	x_int32_t        m_xt_height;               ///< 解码时得到的图像高度
	x_bool_t         m_xt_flip;                 ///< 解码时是否对图像进行垂直翻转
public:
	WT_H264DecodeCallback   m_xfunc_realcbk;           ///< 实时解码后的数据回调函数接口
	
	//add by yiweijiao
	int m_disconnect_times;//断开连接的持续时间
	int m_getdata;//本次是否获取到数据 用来侦测是否断开连接
	char m_rtsp_url[200];


	HWND m_hwnd;
	enum WT_PixelFormat_t m_PixelFormat;
	int m_Enable;
	WT_H264HANDLE m_H264handle;
	//显示
	void ShowImage(unsigned char *pFrameRGB,UINT nSize, int nWidth, int nHeight);
	int m_WorkStatue;//0 用户期望终止工作 1 用户期望保持或者开始工作
	void DestroySource(void);
	/*
	接受线程和解码线程的运行状态
	*/
	int m_decodeThreadStatue; // 0停止中 1运行中
	int m_ReciveThreadStatue; // 0停止中 1 运行中
};

///////////////////////////////////////////////////////////////////////////

#endif // __VXRTSPCLIENT_H__

