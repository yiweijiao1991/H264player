// H264videoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "H264video.h"
#include "H264videoDlg.h"


#pragma comment(lib, "libvlc.lib")
#pragma comment(lib, "libvlccore.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CH264videoDlg 对话框




CH264videoDlg::CH264videoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CH264videoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_vlc_ins    = NULL;
	m_vlc_player = NULL;
	m_vlc_media  = NULL;
	m_hwnd = NULL;
	m_x = 500;
	m_y = 250;
	m_wigth = 600;
	m_higth = 600;


}

void CH264videoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_startplay);
	DDX_Control(pDX, IDC_BUTTON1, m_stopplay);
	DDX_Control(pDX, IDC_EDIT1, m_urlEdi);
	DDX_Control(pDX, IDC_H264_SHOW, m_PicVideo);
	DDX_Control(pDX, IDC_BUTTON2, m_Fullscreen);
	DDX_Control(pDX, IDC_RUL_, m_staticUrl);
}

BEGIN_MESSAGE_MAP(CH264videoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CH264videoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CH264videoDlg::OnBnClickedButton1)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, &CH264videoDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CH264videoDlg 消息处理程序

BOOL CH264videoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 设置界面
	MoveWindow(m_x,m_y,m_wigth,m_higth);
	m_PicVideo.MoveWindow(4,60,m_wigth - 15,m_higth - 130, TRUE);
	m_urlEdi.MoveWindow(60,30,m_wigth - 120,28);
	m_staticUrl.MoveWindow(10,30,50,28);
	m_staticUrl.SetWindowText("URL:");
	m_startplay.MoveWindow(80,m_higth - 60,50,30);
	m_Fullscreen.MoveWindow(m_wigth/2 - 25,m_higth - 60,50,30);
	m_stopplay.MoveWindow(m_wigth - 80 -50,m_higth - 60,50,30);
	//  显示TitleBar  
	//ModifyStyle(0,  WS_CAPTION,  SWP_FRAMECHANGED); 
	m_startplay.EnableWindow(TRUE);
	m_stopplay.EnableWindow(FALSE);

	//初始化播放器
	m_hwnd = GetDlgItem(IDC_H264_SHOW)->GetSafeHwnd();
	m_vlc_ins = libvlc_new(0, NULL);
	if(m_vlc_ins != NULL)
	{
		// 创建一个VLC播放器
		m_vlc_player = libvlc_media_player_new(m_vlc_ins);
	}



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CH264videoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}



}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CH264videoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


int CH264videoDlg::showH264()
{
		if(m_vlc_player != NULL && m_vlc_ins !=NULL)
		{
			//libvlc_media_player_set_hwnd (m_vlc_player, hwnd);
			// 通过文件路径创建一个媒体实例,这里是我的测试文件
			//m_vlc_media = libvlc_media_new_path(m_vlc_ins, "d:\\my.h264");
			m_vlc_media = libvlc_media_new_location(m_vlc_ins,m_url.GetBuffer(0));
			if(m_vlc_media != NULL)
			{
				// 解析媒体实例
				libvlc_media_parse(m_vlc_media);
				// 获取媒体文件的播放长度,  返回 ms
				//libvlc_time_t duration = libvlc_media_get_duration(m_vlc_media);

				// 此处是获取媒体包含多个的视频和音频轨以及其他类型的轨道信息
				//libvlc_media_track_info_t *media_tracks = NULL;
				//int trackCount = libvlc_media_get_tracks_info(m_vlc_media, &media_tracks);
				// 这里是释放内存，但我测试的时候会有问题，还没仔细研究是为何
				// free(media_tracks);  // crash?

				// 把打开的媒体文件设置给播放器
				libvlc_media_player_set_media(m_vlc_player,m_vlc_media);

				// 因为是windows系统，所以需要设置一个HWND给播放器作为窗口,这里就直接使用桌面窗口,这里仅是测试
				libvlc_media_player_set_hwnd(m_vlc_player,m_hwnd);
				// 开始播放视频
				libvlc_media_player_play(m_vlc_player);
				
			}

		}
	return 0;
}
void CH264videoDlg::FreeH264()
{
	if(m_vlc_media)
	{
		libvlc_media_player_stop(m_vlc_player);
		libvlc_media_release(m_vlc_media);
		m_vlc_media = NULL;

	}
	if (m_vlc_player)
	{
		libvlc_media_player_release(m_vlc_player);
		m_vlc_player = NULL;
	}
	if (m_vlc_ins)
	{
		libvlc_release(m_vlc_ins);
		m_vlc_ins = NULL;

	}
	
}

void CH264videoDlg::OnBnClickedOk()
{
	// 开始播放
	if(m_vlc_player)
	{
		m_urlEdi.GetWindowText(m_url);
		showH264();
		m_startplay.EnableWindow(FALSE);
		m_stopplay.EnableWindow(TRUE);
		Sleep(1000);
	}


}

void CH264videoDlg::OnBnClickedButton1()
{
	//停止播放
	if (m_vlc_player)
	{
		libvlc_media_player_stop(m_vlc_player);
		//FreeH264();
		m_startplay.EnableWindow(TRUE);
		m_stopplay.EnableWindow(FALSE);
		Sleep(1000);

	}

}


void CH264videoDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	FreeH264();
	CDialog::OnClose();
}



BOOL CH264videoDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类


if (WM_KEYDOWN == pMsg->message)
{
	if (VK_ESCAPE == pMsg->wParam)  
	{  
		MoveWindow(m_x,m_y,m_wigth,m_higth);
		m_PicVideo.MoveWindow(4,60,m_wigth - 15,m_higth - 130, TRUE);
		m_urlEdi.MoveWindow(60,30,m_wigth - 120,28);
		m_staticUrl.MoveWindow(10,30,50,28);
		m_staticUrl.SetWindowText("URL:");
		m_startplay.MoveWindow(80,m_higth - 60,50,30);
		m_Fullscreen.MoveWindow(m_wigth/2 - 25,m_higth - 60,50,30);
		m_stopplay.MoveWindow(m_wigth - 80 -50,m_higth - 60,50,30);
		//  显示TitleBar  
		ModifyStyle(0,  WS_CAPTION,  SWP_FRAMECHANGED); 

		m_stopplay.ShowWindow(SW_SHOW);
		m_startplay.ShowWindow(SW_SHOW);
		m_Fullscreen.ShowWindow(SW_SHOW);
		m_urlEdi.ShowWindow(SW_SHOW);
		m_staticUrl.ShowWindow(SW_SHOW);
		return TRUE;
	}
}


	return CDialog::PreTranslateMessage(pMsg);
}

void CH264videoDlg::OnBnClickedButton2()
{
	// 实现全屏
	//获取屏幕分辨率
	ModifyStyle(WS_CAPTION,  0,  SWP_FRAMECHANGED);
	int nFullWidth  = GetSystemMetrics(SM_CXSCREEN);  
	int nFullHeight = GetSystemMetrics(SM_CYSCREEN);  
	MoveWindow(0,0,nFullWidth,nFullHeight);
	m_PicVideo.MoveWindow(0, 0, nFullWidth, nFullHeight, TRUE);


	ShowWindow(SW_HIDE);
	ShowWindow(SW_SHOW);
	m_stopplay.ShowWindow(SW_HIDE);
	m_startplay.ShowWindow(SW_HIDE);
	m_Fullscreen.ShowWindow(SW_HIDE);
	m_urlEdi.ShowWindow(SW_HIDE);
	m_staticUrl.ShowWindow(SW_HIDE);
	Invalidate();
	
}
