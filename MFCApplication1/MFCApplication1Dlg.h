#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "CvvImage.h"<<<<<<<<<
#include "afxwin.h"
// MFCApplication1Dlg.h : 头文件
//

#pragma once

#include"HCNetSDK.h"
#include"PlayM4.h"//好像不是这个问题5.8
#include "afxcmn.h"
#include<string>//用不用？
#include <cstring>
#include"Queue.h"
// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	void DrawPicToHDC(IplImage *img, UINT ID);//<<<<<
	afx_msg void OnBnClickedButton1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedGuanbi();
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedTianjia();
	CListBox m_contact;
	afx_msg void OnBnClickedShanchu();
	afx_msg void OnBnClickedQingkong();
	afx_msg void OnBnClickedCancel();
	CString m_newcontact;
	CListBox m_data;
	CComboBox m_shitu;
	afx_msg void OnCbnSelchangeShituCombo1();
	afx_msg void OnBnClickedLoginButton2();
	CIPAddressCtrl m_ctrlDeviceIP;
	CString m_csUserName;
	CString m_csPassword;
	short m_nLoginPort;
	float m_lvbo;
	float m_fushi;
	float m_pengzhang;
	CSliderCtrl m_lvboslider;
	CSliderCtrl m_fushislider;
	CSliderCtrl m_pengzhangslider;
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBaocunButton4();
	afx_msg void On32771();
};
