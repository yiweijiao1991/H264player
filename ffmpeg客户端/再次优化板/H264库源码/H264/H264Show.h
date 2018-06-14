/*
函数功能：打开视频流
hWnd:显示的窗口
szURL：流地址 例如：rtsp://192.168.0.98/stream1
返回值：返回句柄
*/
HANDLE __stdcall H264_Play(HWND hWnd, LPCSTR szURL);
/*
关闭流
hPlay  ：H264_Play返回的句柄
*/
VOID __stdcall H264_Destroy(HANDLE hPlay);
/*
播放控制
hPlay:H264_Play返回的句柄
cmd : 0 暂停播放 1 开始播放
*/
VOID __stdcall H264_Control(HANDLE hPlay,int cmd);