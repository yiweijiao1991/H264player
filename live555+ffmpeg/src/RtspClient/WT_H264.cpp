#include "WT_H264.h"
#include "vxRTSPClient.h"


/* 启动H264接收

	return：
		成功：0
		失败：<0

	notes：
		此接口仅需调用一次
*/
vxRTSPClient       m_rtspClient[16];
int Find_Decode()
{
	int i = 0;
	for (i = 0; i< 16;i++)
	{
		if(!m_rtspClient[i].is_working())
			return i;
	} 
	return -1;
}
int __stdcall WT_H264Init()
{
	vxRTSPClient::initial_lib(0);
	return 0;
}
/*  
	H264销毁，释放资源

	notes:
		销毁资源后，有做界面控件显示的，需要主动
		调用ShowWindow(TRUE)函数，使控件出来。
*/
void __stdcall WT_H264Destory()
{
	int i = 0;
	for(i = 0; i< 16;i++)
	{
		if(m_rtspClient[i].is_working())
			m_rtspClient[i].close();
	}

	vxRTSPClient::uninitial_lib();
}
/* 启动H264接收
	param:
		chIp:			要连接的设备IP地址
		hwnd:			要显示的控件句柄
		pixelFormat:	设置H264解码后的像素格式
		nEnable:		设置解码后是否主动显示到控件上:nEnable:1显示;0不显示
						注：设置pixelFormat==WT_PIX_FMT_BGR24时，才会主动此显示
	
	return：
		成功：返回H264句柄
		失败：0
*/
WT_H264HANDLE __stdcall WT_H264Start(char *chIp, HWND hwnd, enum WT_PixelFormat_t pixelFormat, int nEnable) 
{
	char str[200] = {0};
	WT_H264HANDLE handle = 0;
	sprintf(str,"rtsp://%s/stream1",chIp);
	WT_H264HANDLE ID = Find_Decode();
	if (ID < 0 || ID > 15)
	{
	return 0;
	}
	if(m_rtspClient[ID].is_working())
		return -1;
	handle = ID+1;
	m_rtspClient[ID].real_decode_enable_flip(0);
	if( 0 != m_rtspClient[ID].open(str,hwnd,pixelFormat, nEnable,handle))
		return 0;
	return handle;

}

/*  结束单个设备的H264的链接
	param:
		handle:			H264句柄

	return：
		成功：0
		失败：<0

	notes:
		断开H.264接收后，有做界面控件显示的，需要主动
		调用ShowWindow(TRUE)函数，使控件出来。
*/
int __stdcall WT_H264End(WT_H264HANDLE handle)
{
	int ID = handle -1;
	if(ID < 0 || ID >15)
		return -1;
	if(!m_rtspClient[ID].is_working())
		return -1;
	m_rtspClient[ID].close();
	return 0;

}

/*  更改H264显示的窗口
	param:
		handle:			H264句柄
		hwnd:			窗体控件句柄

	return：
		成功：0
		失败：<0
*/
int __stdcall WT_ChangeH264ShowWindows(WT_H264HANDLE handle, HWND hwnd)
{
	int ID = handle -1;
	m_rtspClient[ID].m_hwnd = hwnd;
	return 0;
}
	
/*  
	通过回调的方式，获取接收、解码后的数据
	notes：
		具体的参数参照WT_H264Decode_t结构体
*/
void __stdcall WT_RegH264DecodeEvent(WT_H264DecodeCallback h264Decode)
{
	int i = 0;
	for (i =0 ;i<16;i++)
	{
		m_rtspClient[i].m_xfunc_realcbk = h264Decode;
	}
	
}

/*为已经打开的单路解码库单独传入回调函数
param：
handle:			H264句柄
h264Decode 回调函数
*/
int __stdcall WT_RegH264DecodeEvent_EX(WT_H264DecodeCallback h264Decode,WT_H264HANDLE handle)
{
	int ID = handle -1;
	if(ID < 0 || ID >15)
	return -1;
	m_rtspClient[ID].m_xfunc_realcbk = h264Decode;
	return 1;

}