// H264videoDlg.h : 头文件
//

#pragma once

#include <tchar.h>
#include <time.h>
#include <vlc/vlc.h>
#include "afxwin.h"

// CH264videoDlg 对话框
class CH264videoDlg : public CDialog
{
// 构造
public:
	CH264videoDlg(CWnd* pParent = NULL);	// 标准构造函数
	//int showH264();
// 对话框数据
	enum { IDD = IDD_H264VIDEO_DIALOG };

	libvlc_instance_t *     m_vlc_ins;
	libvlc_media_player_t * m_vlc_player;
	libvlc_media_t *        m_vlc_media;
	void * m_hwnd;
	CString m_url;
	int m_x;
	int m_y;
	int m_wigth;
	int m_higth;

	void FreeH264();
	int showH264();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	// 开始播放
	CButton m_startplay;
	// 停止播放
	CButton m_stopplay;
	// 地址输入框
	CEdit m_urlEdi;
	afx_msg void OnClose();

	// 显示视频
	CStatic m_PicVideo;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButton2();
	// 全屏
	CButton m_Fullscreen;
	CStatic m_staticUrl;
};
