
// H264ShowPlayDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "H264ShowPlay.h"
#include "H264ShowPlayDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CH264ShowPlayDlg 对话框


WT_H264HANDLE g_handle1;//视频显示返回的句柄
WT_H264HANDLE g_handle2;//视频显示返回的句柄
BOOL nWindowType=false;
//默认保存BMP文件到E:/temp/中
void SaveBMP(unsigned char *pFrameRGB, int width, int height, int index, int bpp)  
{  
    char buf[5] = {0};  
    BITMAPFILEHEADER bmpheader;  
    BITMAPINFOHEADER bmpinfo;  
    FILE *fp;  
	int nWidth;

	nWidth = (width +3) /4 *4;
  
	char filename[255] = {0}; 
    char *chTemp = new char[nWidth*bpp/8];  
  
    //文件存放路径，根据自己的修改  
    sprintf_s(filename, 255, "%s%d.bmp", "./", index);  
    if( (fp = fopen(filename,"wb+")) == NULL ) {  
        printf ("open file failed!\n");  
		delete chTemp;
		chTemp = NULL;
        return;  
    }  
  
    bmpheader.bfType = 0x4d42;  
    bmpheader.bfReserved1 = 0;  
    bmpheader.bfReserved2 = 0;  
    bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
    bmpheader.bfSize = bmpheader.bfOffBits + nWidth*height*bpp/8;  
  
    bmpinfo.biSize = sizeof(BITMAPINFOHEADER);  
    bmpinfo.biWidth = nWidth;  
    bmpinfo.biHeight = height;  
    bmpinfo.biPlanes = 1;  
    bmpinfo.biBitCount = bpp;  
    bmpinfo.biCompression = BI_RGB;  
    bmpinfo.biSizeImage = (nWidth*bpp+31)/32*4*height;  
    bmpinfo.biXPelsPerMeter = 100;  
    bmpinfo.biYPelsPerMeter = 100;  
    bmpinfo.biClrUsed = 0;  
    bmpinfo.biClrImportant = 0;  
 
    fwrite(&bmpheader, sizeof(bmpheader), 1, fp);  
    fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp);
	/*将图像上下颠倒一下，否则显示图像不正常*/
	for(int i = height-1; i >= 0; i--)
	{
		memset(chTemp, 0, nWidth*bpp/8);
		memcpy(chTemp, pFrameRGB+(i*width*bpp/8), width*bpp/8);
		fwrite(chTemp, nWidth*bpp/8, 1, fp);
	}
    //fwrite(pFrameRGB, width*height*bpp/8, 1, fp);  
  
    fclose(fp);  
	delete chTemp;
	chTemp = NULL;
} 

// CEP_H264_TESTDlg 消息处理程序
int index;




void SaveYUV(WT_H264Decode_t *h264_decode,int index)
{
	char filename[255] = {0}; 
	FILE *fp;  
	int i = 0;
	//文件存放路径，根据自己的修改  
	sprintf_s(filename, 255, "%s%d.yuv", "./", index);  
	if( (fp = fopen(filename,"wb+")) == NULL ) {  
		printf ("open file failed!\n");  
		return;  
	} 
	fwrite(h264_decode->imageInfo.pBuffer[0],h264_decode->imageInfo.uPitch[0]*h264_decode->imageInfo.uHeight, 1, fp);
	fwrite(h264_decode->imageInfo.pBuffer[1],h264_decode->imageInfo.uPitch[1]*h264_decode->imageInfo.uHeight, 1, fp);
	fwrite(h264_decode->imageInfo.pBuffer[2],h264_decode->imageInfo.uPitch[2]*h264_decode->imageInfo.uHeight, 1, fp);
	fclose(fp);  

}








//回调函数
void  h264Decode(WT_H264Decode_t *h264_decode)
{
	//AfxMessageBox(_T("hello"));
/*	if(h264_decode->imageInfo.uPixFmt == WT_PIX_FMT_BGR24)
	SaveBMP(h264_decode->imageInfo.pBuffer[0], h264_decode->imageInfo.uWidth, h264_decode->imageInfo.uHeight, index++, 24);
	else if(h264_decode->imageInfo.uPixFmt == WT_PIX_FMT_YUV420P)
	SaveYUV(h264_decode,index++);*/
}


CH264ShowPlayDlg::CH264ShowPlayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CH264ShowPlayDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CH264ShowPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CH264ShowPlayDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CH264ShowPlayDlg::OnStart)
	ON_BN_CLICKED(IDC_BUTTON2, &CH264ShowPlayDlg::OnH264End)
	ON_BN_CLICKED(IDC_BUTTON5, &CH264ShowPlayDlg::OnChangeH264ShowWindows)
END_MESSAGE_MAP()


// CH264ShowPlayDlg 消息处理程序

BOOL CH264ShowPlayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	WT_H264Init();//启动H264接收初始化

	//WT_RegH264DecodeEvent(h264Decode);//注册回调函数
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CH264ShowPlayDlg::OnPaint()
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
HCURSOR CH264ShowPlayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CH264ShowPlayDlg::OnDestroy()
{
	WT_H264Destory();
	CDialog::OnDestroy();
}








void CH264ShowPlayDlg::OnStart()
{
	int i = 0;
	UpdateData(TRUE);
	char			chDevIP1[16];
	char			chDevIP2[16];
	this->GetDlgItemText(IDC_DEV1_IP, chDevIP1, 16);
	this->GetDlgItemText(IDC_DEV2_IP, chDevIP2, 16);
	g_handle1 = WT_H264Start(chDevIP1, GetDlgItem(IDC_PICTURE1)->GetSafeHwnd(), WT_PIX_FMT_BGR24, 1);//视频的开启
	g_handle2 = WT_H264Start(chDevIP2, GetDlgItem(IDC_PICTURE2)->GetSafeHwnd(), WT_PIX_FMT_BGR24, 1);//视频的开启
}




void CH264ShowPlayDlg::OnH264End()  
{
	//if(g_handle1!=0)
	WT_H264End(g_handle1);//视频关闭
	//if(g_handle2!=0)
	WT_H264End(g_handle2);//视频关闭
	g_handle1=0;
	g_handle2=0;
	this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
	this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
}


void CH264ShowPlayDlg::OnChangeH264ShowWindows()
{
	this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
	this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
	if(nWindowType)
	{
		if(g_handle1!=0)
		{
			WT_ChangeH264ShowWindows(g_handle1, GetDlgItem(IDC_PICTURE1)->GetSafeHwnd());
			
			nWindowType=false;
		}
	}
	else
	{
		WT_ChangeH264ShowWindows(g_handle1, GetDlgItem(IDC_PICTURE2)->GetSafeHwnd());
		nWindowType=true;
	}
	this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
	this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
 /*
	if(nWindowType)
	{		
		WT_ChangeH264ShowWindows(g_handle2, GetDlgItem(IDC_PICTURE2)->GetSafeHwnd());		
		this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
		this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);	
		WT_ChangeH264ShowWindows(g_handle1, GetDlgItem(IDC_PICTURE1)->GetSafeHwnd());
		WT_ChangeH264ShowWindows(g_handle2, GetDlgItem(IDC_PICTURE2)->GetSafeHwnd());
		this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
		this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
		nWindowType=false;
	}
	else
	{
		WT_ChangeH264ShowWindows(g_handle2, GetDlgItem(IDC_PICTURE1)->GetSafeHwnd());
		this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
		this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);	
		WT_ChangeH264ShowWindows(g_handle1, GetDlgItem(IDC_PICTURE2)->GetSafeHwnd());
		WT_ChangeH264ShowWindows(g_handle2, GetDlgItem(IDC_PICTURE1)->GetSafeHwnd());
		this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
		this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
		nWindowType=true;
	}
	this->GetDlgItem(IDC_PICTURE1)->ShowWindow(TRUE);
	this->GetDlgItem(IDC_PICTURE2)->ShowWindow(TRUE);
*/
	}

