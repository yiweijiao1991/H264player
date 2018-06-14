#ifndef _WT_H264_H
#define _WT_H264_H
#include "stdafx.h"
#include "targetver.h"
typedef unsigned int WT_H264HANDLE;

// 像素格式
enum WT_PixelFormat_t{
	WT_PIX_FMT_BGR24,
	WT_PIX_FMT_YUV420P,
};

/**图像信息*/
typedef struct
{
	unsigned		uWidth;			/**<宽度*/
	unsigned		uHeight;		/**<高度*/
	unsigned		uPitch[6];		/**<图像宽度的一行像素所占内存字节数
										EP_PIX_FMT_BGR24:
										uPitch[0]: BGR一行像素所占内存字节数
										
										EP_PIX_FMT_YUV420P：
										uPitch[0]: Y一行像素所占内存字节数
										uPitch[1]: U一行像素所占内存字节数
										uPitch[2]: V一行像素所占内存字节数
									*/
	unsigned		uPixFmt;		/**<图像像素格式，参考枚举定义图像格式（PixelFormat_t）*/
	unsigned char	*pBuffer[6];	/**<图像内存的首地址
										EP_PIX_FMT_BGR24:
										pBuffer[0]: BGR首地址
										
										EP_PIX_FMT_YUV420P：
										pBuffer[0]: Y首地址
										pBuffer[1]: U首地址
										pBuffer[2]: V首地址
									*/
}
WT_ImageInfo_t;

typedef struct WT_H264Decode_t{
	WT_H264HANDLE	handle;
	WT_ImageInfo_t	imageInfo;

	char			reserved[100];
}WT_H264Decode_t;

/* 启动H264接收

	return：
		成功：0
		失败：<0

	notes：
		此接口仅需调用一次
*/
int __stdcall WT_H264Init();


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
WT_H264HANDLE __stdcall WT_H264Start(char *chIp, HWND hwnd, enum WT_PixelFormat_t pixelFormat, int nEnable) ;


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
int __stdcall WT_H264End(WT_H264HANDLE handle);


/*  
	H264销毁，释放资源

	notes:
		销毁资源后，有做界面控件显示的，需要主动
		调用ShowWindow(TRUE)函数，使控件出来。
*/
void __stdcall WT_H264Destory();


/*  更改H264显示的窗口
	param:
		handle:			H264句柄
		hwnd:			窗体控件句柄

	return：
		成功：0
		失败：<0
*/
int __stdcall WT_ChangeH264ShowWindows(WT_H264HANDLE handle, HWND hwnd);


/*  
	通过回调的方式，获取接收、解码后的数据
	notes：
		具体的参数参照WT_H264Decode_t结构体
*/
typedef void (*WT_H264DecodeCallback)(WT_H264Decode_t *h264_decode);
void __stdcall WT_RegH264DecodeEvent(WT_H264DecodeCallback h264Decode);
/*为已经打开的单路解码库单独传入回调函数
param：
handle:			H264句柄
h264Decode 回调函数
*/
int __stdcall WT_RegH264DecodeEvent_EX(WT_H264DecodeCallback h264Decode,WT_H264HANDLE handle);

#endif