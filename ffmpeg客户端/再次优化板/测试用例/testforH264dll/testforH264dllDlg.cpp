// testforH264dllDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "testforH264dll.h"
#include "testforH264dllDlg.h"
#include "H264Show.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtestforH264dllDlg 对话框


HANDLE  H264handl = NULL;

CtestforH264dllDlg::CtestforH264dllDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CtestforH264dllDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_x = 500;//坐标
	m_y = 250;
	m_wigth = 600;//宽高
	m_higth = 600;


	//Init_H264();
}

void CtestforH264dllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_urlTX);
	DDX_Control(pDX, IDC_STATIC1, m_show);
	DDX_Control(pDX, IDOK, m_start);
	DDX_Control(pDX, IDC_BUTTON1, m_fullscreen);
	DDX_Control(pDX, IDCANCEL, m_closestream);
	DDX_Control(pDX, IDC_STATIC2, m_txt);
}

BEGIN_MESSAGE_MAP(CtestforH264dllDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CtestforH264dllDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CtestforH264dllDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CtestforH264dllDlg::OnBnClickedButton1)
	//ON_BN_CLICKED(IDC_BUTTON2, &CtestforH264dllDlg::OnBnClickedButton2)
	ON_STN_DBLCLK(IDC_STATIC1, &CtestforH264dllDlg::OnStnDblclickStatic1)
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BUTTON5, &CtestforH264dllDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CtestforH264dllDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CtestforH264dllDlg::OnBnClickedButton7)
END_MESSAGE_MAP()


// CtestforH264dllDlg 消息处理程序

BOOL CtestforH264dllDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	m_urlTX.SetWindowText("rtsp://192.168.0.239/stream1");
	MoveWindow(m_x,m_y,m_wigth,m_higth);
	m_show.MoveWindow(4,80,m_wigth - 15,m_higth - 150, TRUE);
	m_urlTX.MoveWindow(60,30,m_wigth - 200,28);
	m_txt.MoveWindow(10,30,50,28);
	m_txt.SetWindowText("URL:");

	m_fullscreen.MoveWindow(m_wigth/2 - 25,m_higth - 60,50,30);
	m_start.MoveWindow(m_wigth-80,10,50,30);
	m_closestream.MoveWindow(m_wigth-80,50,50,30);

	//  显示TitleBar  
	ModifyStyle(0,  WS_CAPTION,  SWP_FRAMECHANGED); 

	m_fullscreen.ShowWindow(SW_SHOW);
	m_urlTX.ShowWindow(SW_SHOW);
	m_txt.ShowWindow(SW_SHOW);
	m_start.ShowWindow(SW_SHOW);
	m_closestream.ShowWindow(SW_SHOW);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CtestforH264dllDlg::OnPaint()
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
HCURSOR CtestforH264dllDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestforH264dllDlg::OnBnClickedOk()
{
	CString s1;
	int ret;
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC1); //获取图片控件的窗口指针 

	m_urlTX.GetWindowText(s1);
	H264handl = H264_Play(* pWnd,s1);

	//OnOK();
}

void CtestforH264dllDlg::OnBnClickedCancel()
{
if(H264handl)	
 H264_Destroy(H264handl);
}

void CtestforH264dllDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//exit_H264();
	OnCancel();
	CDialog::OnClose();
}

void CtestforH264dllDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString s1;
	//exit_H264();
	ModifyStyle(WS_CAPTION,  0,  SWP_FRAMECHANGED);
	int nFullWidth  = GetSystemMetrics(SM_CXSCREEN);  
	int nFullHeight = GetSystemMetrics(SM_CYSCREEN);  
	MoveWindow(0,0,nFullWidth,nFullHeight);
	m_show.MoveWindow(0, 0, nFullWidth, nFullHeight, TRUE);
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC1); //获取图片控件的窗口指针 
	m_urlTX.GetWindowText(s1);

	//Open_stream(pWnd,s1.GetBuffer(0));
	//change_Window(pWnd);

}



void CtestforH264dllDlg::OnStnDblclickStatic1()
{
	// TODO: 在此添加控件通知处理程序代码


	//放大窗口
	MoveWindow(m_x,m_y,m_wigth,m_higth);
	m_show.MoveWindow(4,80,m_wigth - 15,m_higth - 150, TRUE);
	m_urlTX.MoveWindow(60,30,m_wigth - 120,28);
	m_txt.MoveWindow(10,30,50,28);
	m_txt.SetWindowText("URL:");
	m_startplay.MoveWindow(80,m_higth - 60,50,30);
	m_fullscreen.MoveWindow(m_wigth/2 - 25,m_higth - 60,50,30);

	m_closestream.MoveWindow(m_wigth-20,50,50,30);

	//  显示TitleBar  
	ModifyStyle(0,  WS_CAPTION,  SWP_FRAMECHANGED); 

	m_fullscreen.ShowWindow(SW_SHOW);
	m_urlTX.ShowWindow(SW_SHOW);
	m_txt.ShowWindow(SW_SHOW);
	m_start.ShowWindow(SW_SHOW);
	m_closestream.ShowWindow(SW_SHOW);

	CWnd* pWnd = this->GetDlgItem(IDC_STATIC1); //获取图片控件的窗口指针 
	//change_Window(pWnd);

}

void CtestforH264dllDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	MoveWindow(m_x,m_y,m_wigth,m_higth);
	m_show.MoveWindow(4,60,m_wigth - 15,m_higth - 130, TRUE);
	m_urlTX.MoveWindow(60,30,m_wigth - 200,28);
	m_txt.MoveWindow(10,30,50,28);
	m_txt.SetWindowText("URL:");

	m_fullscreen.MoveWindow(m_wigth/2 - 25,m_higth - 60,50,30);

	m_start.MoveWindow(m_wigth-80,10,50,30);
	m_closestream.MoveWindow(m_wigth-80,40,50,30);

	//  显示TitleBar  
	ModifyStyle(0,  WS_CAPTION,  SWP_FRAMECHANGED); 



	m_fullscreen.ShowWindow(SW_SHOW);
	m_urlTX.ShowWindow(SW_SHOW);
	m_txt.ShowWindow(SW_SHOW);
	m_start.ShowWindow(SW_SHOW);
	m_closestream.ShowWindow(SW_SHOW);

	CWnd* pWnd = this->GetDlgItem(IDC_STATIC1); //获取图片控件的窗口指针 
	//change_Window(pWnd);

	CDialog::OnLButtonDblClk(nFlags, point);

}

void CtestforH264dllDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	if(H264handl)	
		H264_Control(H264handl,1);
}

void CtestforH264dllDlg::OnBnClickedButton6()
{
	if(H264handl)	
		H264_Control(H264handl,0);
}

void CtestforH264dllDlg::OnBnClickedButton7()
{

}
