
// MFCApplication1Dlg.cpp : 实现文件
//MFC+海康

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/bgsegm.hpp>
#include<iostream>
#include<string>
#include<fstream>
#include "SendMail.h"//发邮件
#include<Kinect.h>//
#include<stdio.h>//海康SDK包含的头文件
#include"Windows.h"
#include"HCNetSDK.h"
#include"plaympeg4.h"
#include<time.h>
#include <cstdlib>
#include <cstring>
#include"videoprocessor.h"//opencv处理包含的文件
#include"BGFGSegmentor.h"
#include<list>//存放码流的帧
#include<queue>
#include<stack>


using namespace std;
using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/*定义全局变量*/
cv::VideoCapture  capture;//定义一个摄像头类型的对象
cv::Mat cframe = imread("homecare.png");//定义一个Mat类对象，用来存放摄像头的图像流
IplImage *iframe = &IplImage(cframe);//转换图像用于显示在MFC上
Mat output1;
cv::Mat foreground;//图像处理
cv::Mat background;
cv::Ptr<cv::BackgroundSubtractor> ptrMOG = cv::bgsegm::createBackgroundSubtractorMOG();
std::string lxr[20];//定义字符串数组存放联系人
int i = 0;//联系人数量
LONG nPort = -1;//海康
volatile int gbHandling = 3;//解决了延时3
LONG lRealPlayHandle;//全局启停预览
LONG lUserID;//注册设备
HWND hPlayWnd = NULL;//官方demo中用的
BOOL bRet1;//官方demo中用的
CRITICAL_SECTION g_cs_frameList;
std::list<Mat> g_frameList;//存放码流的帧，CSDN
queue<Mat> framequeue;
stack<Mat> g_framestack;
BOOL STOP(FALSE);
Mat output;
int h = 0;
int diedaojishu = 0;
Mat output2;//用usb摄像头
int jishi = 0;//数据框累 计监护时间
int cFlag = 0;//组合框
float lvbo = 0;//可调图像处理
float fushi = 0;
float pengzhang = 0;
float v = 0;//跌倒检测
float shengao = 0;
float jiao = 0;
float vjiao = 0;
int jiange = 0;


/*函数声明*/
void process1(cv::Mat &frame, cv::Mat &output, int threshold, double learningRate);


// IP to String
CString IPToStr(DWORD dwIP)
{
	CString strIP = _T("");
	WORD add1, add2, add3, add4;

	add1 = (WORD)(dwIP & 255);
	add2 = (WORD)((dwIP >> 8) & 255);
	add3 = (WORD)((dwIP >> 16) & 255);
	add4 = (WORD)((dwIP >> 24) & 255);
	strIP.Format(_T("%d.%d.%d.%d"), add4, add3, add2, add1);
	return strIP;
}

//用于将视频显示到MFC界面上  
void CMFCApplication1Dlg::DrawPicToHDC(IplImage *img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img); // 复制图片
	cimg.DrawToHDC(hDC, &rect); // 将图片绘制到显示控件的指定区域内
	ReleaseDC(pDC);
}//<<<<<<<<


/************海康************/
//解码回调 视频为YUV数据(YV12)，音频为PCM数据,视频流转码为可供opencv处理的BGR类型的图片数据
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	if (gbHandling)
	{
		gbHandling--;
		return;
	}

	long lFrameType = pFrameInfo->nType;
	if (lFrameType == T_YV12)
	{

		Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
		Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
		cvtColor(src, pImg, CV_YUV2BGR_YV12);
		Mat frametemp=pImg.clone();
		g_framestack.push(frametemp); 
		imshow("IPCamera", pImg);
		waitKey(2);

	}

	gbHandling = 3;

}

//实时视频码流数据获取  回调函数
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;//官方demo

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //系统头
		if (nPort >= 0)
		{
			break;//同一路码流不需要多次调用开流接口
		}

		if (!PlayM4_GetPort(&nPort))
		{
			break;
		}
		if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}

		//设置解码回调函数 解码且显示
		if (!PlayM4_SetDecCallBackEx(nPort, DecCBFun, NULL, NULL))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}


		//打开视频解码
		if (!PlayM4_Play(nPort, hPlayWnd))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}
	case NET_DVR_STREAMDATA:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			//OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	default:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			//OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	}

}


void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //预览时重连
		printf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}





// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCApplication1Dlg::IDD, pParent)
	, m_newcontact(_T(""))
	, m_csUserName(_T(""))
	, m_csPassword(_T(""))
	, m_nLoginPort(0)
	, m_lvbo(0)
	, m_fushi(0)
	, m_pengzhang(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_contact);
	DDX_Text(pDX, IDC_EDIT1, m_newcontact);
	DDX_Control(pDX, IDC_DATA_LIST2, m_data);
	DDX_Control(pDX, IDC_SHITU_COMBO1, m_shitu);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ctrlDeviceIP);
	DDX_Text(pDX, IDC_EDIT4, m_csUserName);
	DDX_Text(pDX, IDC_EDIT5, m_csPassword);
	DDX_Text(pDX, IDC_EDIT3, m_nLoginPort);
	DDX_Text(pDX, IDC_EDIT2, m_lvbo);
	DDX_Text(pDX, IDC_EDIT6, m_fushi);
	DDX_Text(pDX, IDC_EDIT7, m_pengzhang);
	DDX_Control(pDX, IDC_SLIDER1, m_lvboslider);
	DDX_Control(pDX, IDC_SLIDER2, m_fushislider);
	DDX_Control(pDX, IDC_SLIDER3, m_pengzhangslider);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCApplication1Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_GUANBI, &CMFCApplication1Dlg::OnBnClickedGuanbi)
//	ON_WM_CREATE()
//ON_WM_CREATE()
ON_BN_CLICKED(IDC_TIANJIA, &CMFCApplication1Dlg::OnBnClickedTianjia)
ON_BN_CLICKED(IDC_SHANCHU, &CMFCApplication1Dlg::OnBnClickedShanchu)
ON_BN_CLICKED(IDC_QINGKONG, &CMFCApplication1Dlg::OnBnClickedQingkong)
ON_BN_CLICKED(IDCANCEL, &CMFCApplication1Dlg::OnBnClickedCancel)
ON_CBN_SELCHANGE(IDC_SHITU_COMBO1, &CMFCApplication1Dlg::OnCbnSelchangeShituCombo1)
ON_BN_CLICKED(IDC_LOGIN_BUTTON2, &CMFCApplication1Dlg::OnBnClickedLoginButton2)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMFCApplication1Dlg::OnNMCustomdrawSlider1)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CMFCApplication1Dlg::OnNMCustomdrawSlider2)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, &CMFCApplication1Dlg::OnNMCustomdrawSlider3)
ON_BN_CLICKED(IDC_BAOCUN_BUTTON4, &CMFCApplication1Dlg::OnBnClickedBaocunButton4)
ON_COMMAND(ID_32771, &CMFCApplication1Dlg::On32771)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	UpdateData(TRUE);
	NET_DVR_Init();// 初始化海康SDK

	lRealPlayHandle = -1;
	lUserID = -1;

	/***登录***/
	m_ctrlDeviceIP.SetAddress(192,168,1,104);
	m_csUserName = "admin";
	m_csPassword = "809709qwe!";
	m_nLoginPort = 8000;
	/****数据框初始化****/
	CTime time = CTime::GetCurrentTime();
	CString csTmp = time.Format(_T("%Y-%m-%d  %H:%M:%S  %A"));
	m_data.InsertString(0, csTmp);
	SetTimer(5, 1000, NULL);

	//显示系统监护时间
	CString jishistr;
	jishistr.Format(_T("系统累计监护时间：%d s"), jishi);
	m_data.InsertString(1, jishistr);

	/******组合框初始化******/
	m_shitu.AddString(_T("原图"));
	m_shitu.AddString(_T("跟踪图"));
	m_shitu.SetCurSel(0);//选择第一项

	/***调节条***/
	m_lvboslider.SetRange(0, 50);
	m_lvboslider.SetTicFreq(1);
	m_lvboslider.SetPos(5);
	m_lvboslider.SetPageSize(0.1);//按上下键的移动量

	m_fushislider.SetRange(0, 50);
	m_fushislider.SetTicFreq(1);
	m_fushislider.SetPos(11);
	m_fushislider.SetPageSize(0.1);//按上下键的移动量

	m_pengzhangslider.SetRange(0, 50);
	m_pengzhangslider.SetTicFreq(1);
	m_pengzhangslider.SetPos(4);
	m_pengzhangslider.SetPageSize(0.1);//按上下键的移动量

	m_lvbo = m_lvboslider.GetPos();
	m_fushi = m_fushislider.GetPos();
	m_pengzhang = m_pengzhangslider.GetPos();
	UpdateData(FALSE);//显示到编辑框
	lvbo = m_lvbo;
	fushi = m_fushi;
	pengzhang = m_pengzhang;


	DrawPicToHDC(iframe, IDC_PIC_STATIC);
	UpdateData(FALSE);

	std::ifstream infile;
	infile.open("联系人.txt");
	if (!infile.is_open())
		return TRUE;//避免首次打开程序没有txt文件，MFC界面打不开。
	                //因此这段必须放在最后，以免程序执行不到
	while (!infile.eof())//若未到文件结束一直循环
	{
		std::getline(infile, lxr[i], '\n');
		i++;
	}
	//删空（因为上面程序读取txt文件会多读一个空行）
	for (int j = 0; j < i-1;j++)
		m_contact.AddString(CString(lxr[j].c_str()) );//显示到listbox
	i--;

	infile.close();

	UpdateData(FALSE);//程序中改变的变量值更新到控件中（编辑框、picture control）
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}



//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***登录程序***/
void CMFCApplication1Dlg::OnBnClickedLoginButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	DWORD dwDeviceIP;
	char DeviceIP[16] = { 0 };
	char cUserName[100] = { 0 };
	char cPassword[100] = { 0 };
	CString csTemp;

	m_ctrlDeviceIP.GetAddress(dwDeviceIP);
	csTemp = IPToStr(dwDeviceIP);

	//登录设备，需要设备IP、端口、用户名、密码 Login the device
	NET_DVR_DEVICEINFO_V30 devInfo;
	lUserID = NET_DVR_Login_V30(CT2A(csTemp.GetString()), m_nLoginPort, CT2A(m_csUserName.GetString()), CT2A(m_csPassword.GetString()), &devInfo);

	DWORD dwReturned = 0;



	if (lUserID<0)
		MessageBox(_T("Login failed!"));
	else
		MessageBox(_T("Login successfully!"));

	UpdateData(FALSE);
}


//测试邮件发送程序
void SendMailCS(void)
{
	int num = 100;
	for (int j = 0; j < i; j++)
	{
		string EmailContents = "From: \"跌倒报警系统\"<diedaoyujing@163.com>\r\n"/*From:\"发件人名\"<发件人邮箱>\r\n"<<<<<<<*/
			+ string("To: \"dasiy\"<" + lxr[j] + ">\r\n")
			+ "Subject: 本条为测试邮件，测试是否收到报警信息。\r\n\r\n"/*邮件内容<<<<<<<*/
			+ "test sending variable" + to_string(num) + "\n";

		char EmailTo[100];
		int k;
		for (k = 0; k < lxr[j].length(); k++)
			EmailTo[k] = lxr[j][k];
		EmailTo[k] = '\0';
		SendMail(EmailTo, EmailContents.c_str());
	}
}

//报警邮件发送程序
void SendMailsj(void)
{
	int num = 100;
	for (int j = 0; j < i; j++)
	{
		string EmailContents = "From: \"跌倒报警系统\"<diedaoyujing@163.com>\r\n"/*From:\"发件人名\"<发件人邮箱>\r\n"<<<<<<<*/
			+ string("To: \"dasiy\"<" + lxr[j] + ">\r\n")
			+ "Subject: 有人跌倒，请及时救援。\r\n\r\n"/*邮件内容<<<<<<<*/
			+ "test sending variable" + to_string(num) + "\n";

		char EmailTo[100];
		int k;
		for (k = 0; k < lxr[j].length(); k++)
			EmailTo[k] = lxr[j][k];
		EmailTo[k] = '\0';
		SendMail(EmailTo, EmailContents.c_str());
	}
}



void CMFCApplication1Dlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	SendMailCS();

}



void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	/*********海康**********/
	//---------------------------------------
	UpdateData(TRUE);
	//---------------------------------------
	if (lRealPlayHandle < 0)
	{
		UpdateData(TRUE);
		if (lUserID < 0)
		{
			MessageBox(_T("请先登录设备"), _T("系统"), MB_ICONINFORMATION);
			return;
		}

		//---------------------------------------
		//设置异常消息回调函数
		NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

		//---------------------------------------
		//启动预览并设置回调数据流

		NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
		//	struPlayInfo.hPlayWnd = h;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
		struPlayInfo.hPlayWnd = NULL;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
		struPlayInfo.lChannel = 1;           //预览通道号
		struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
		struPlayInfo.dwLinkMode = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP


		lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, fRealDataCallBack, NULL);

		if (lRealPlayHandle < 0)
		{
			MessageBox(_T("NET_DVR_RealPlay_V40 error\n"), _T("系统"), MB_ICONINFORMATION);
			printf("NET_DVR_RealPlay_V40 error\n");
			printf("%d\n", NET_DVR_GetLastError());
			NET_DVR_Logout(lUserID);
			NET_DVR_Cleanup();
			return;
		}
	}

	SetTimer(4, 1000, NULL);//统计系统监护时间

	while (!STOP)//等待
	{
		 if(g_framestack.size()>0)//栈的元素增加
		 {
			 (g_framestack.top()).copyTo(cframe);
			g_framestack.pop();
			process1(cframe, output, 22, 0.01);
			if (jiange > 0)
				jiange--;
			else
			{
				if (shengao < 75)
				{
					if (v > 25)
					{
						if (vjiao < 5 && v < 40)
						{		
							    SendMailsj();//邮件在前否则要确认窗口才往下进行
								CTime time = CTime::GetCurrentTime();
								CString csTmp = time.Format(_T("%Y-%m-%d  %H:%M:%S"));
								m_data.InsertString(-1, _T("有人跌倒")+csTmp);
								MessageBox(_T("有人跌倒"), _T("系统"), MB_ICONINFORMATION);
								jiange = 20;//空一段时间，避免多次报
						}
					}
				}
			}

			if (cFlag)
				iframe = &IplImage(output1);
			else
				iframe = &IplImage(cframe);
			DrawPicToHDC(iframe, IDC_REAL_STATIC); 
			iframe = &IplImage(output);
			DrawPicToHDC(iframe, IDC_PIC_STATIC);
			waitKey(1);//不加延时不显示
		}
	}
	

	UpdateData(FALSE);
}



void CMFCApplication1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 4:
	{jishi++;
	CString jishistr;
	jishistr.Format(_T("系统累计监护时间：%d s"), jishi);
	m_data.DeleteString(1);
	m_data.InsertString(1, jishistr);
	}
		break;
	case 5:
	{	CTime time = CTime::GetCurrentTime();
	CString csTmp = time.Format(_T("%Y-%m-%d  %H:%M:%S  %A"));
	m_data.DeleteString(0);
	m_data.InsertString(0, csTmp); }
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CMFCApplication1Dlg::OnBnClickedGuanbi()
{	
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	/*******海康*******/
	//---------------------------------------
	KillTimer(4);//关闭累计监护时间计时
	//停止预览
	if (NET_DVR_StopRealPlay(lRealPlayHandle))
	{
		bRet1 = NET_DVR_GetLastError();
	};
	lRealPlayHandle = -1;

	//停止解码
	if (nPort>-1)
	{
		if (!PlayM4_StopSound())
		{
			bRet1 = PlayM4_GetLastError(nPort);
		}
		if (!PlayM4_Stop(nPort))
		{
			bRet1 = PlayM4_GetLastError(nPort);
		}
		if (!PlayM4_CloseStream(nPort))
		{
			bRet1 = PlayM4_GetLastError(nPort);
		}
		PlayM4_FreePort(nPort);
		nPort = -1;
	}
	//停止图像处理
	STOP = TRUE;

	IplImage *image = NULL; //原始图像
	if (image) cvReleaseImage(&image);
	image = cvLoadImage("homecare.jpg", 1); //显示屏保图片
	DrawPicToHDC(image, IDC_PIC_STATIC);

	UpdateData(FALSE);


}

void CMFCApplication1Dlg::OnBnClickedTianjia()//添加联系人
{
	// TODO:  在此添加控件通知处理程序代码
	if (i < 20)//防止联系人数组溢出
	{
		UpdateData(TRUE);
		if (m_newcontact != "")//防止添加空行
		{ 
			m_contact.AddString(m_newcontact);
			lxr[i] = CW2A(m_newcontact.GetString());//string数组
			i++;///将联系人保存到数组实验
			m_newcontact = "";//清空输入
			UpdateData(FALSE);
		}
	}
	else
		MessageBox(_T("最多只可添加20位联系人"), _T("系统"), MB_ICONINFORMATION);
}


void CMFCApplication1Dlg::OnBnClickedShanchu()
{
	// TODO:  在此添加控件通知处理程序代码
	int suoyin, shanchu;
	suoyin = m_contact.GetCurSel();//获取单选的索引
	shanchu = m_contact.DeleteString(suoyin);//删除所选
	for (; suoyin<i; suoyin++)
	{
		lxr[suoyin] = lxr[suoyin + 1];
	}
	i--;//总数已减掉，后面添加要从新的开始
}


void CMFCApplication1Dlg::OnBnClickedQingkong()
{
	// TODO:  在此添加控件通知处理程序代码
	m_contact.ResetContent();//清空联系人列表
	i = 0;
}


void CMFCApplication1Dlg::OnBnClickedCancel()
{	
	// TODO:  在此添加控件通知处理程序代码
	/*******海康*******/
	//---------------------------------------
	if (lRealPlayHandle >= 0)
	{
		MessageBox(_T("请先关闭预览！"), _T("系统"), MB_ICONINFORMATION);
		return;
	}
	//注销用户
	NET_DVR_Logout_V30(lUserID);
	//释放SDK资源
	NET_DVR_Cleanup();
	KillTimer(5);
	

	CDialogEx::OnCancel();
}



void process1(cv::Mat &frame, cv::Mat &output, int threshold, double learningRate) {

	blur(frame, frame, cv::Size(lvbo, lvbo));//滤波 消除噪声点

	Mat gray, backImage;
	// convert to gray-level image
	cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

	// initialize background to 1st frame
	if (background.empty())
		gray.convertTo(background, CV_32F);

	// convert background to 8U
	background.convertTo(backImage, CV_8U);

	// compute difference between current image and background
	cv::absdiff(backImage, gray, foreground);

	// apply threshold to foreground image
	cv::threshold(foreground, output, threshold, 255, cv::THRESH_BINARY_INV);

	// accumulate background
	cv::accumulateWeighted(gray, background,
		// alpha*gray + (1-alpha)*background
		learningRate,  // alpha 
		output);       // mask

	//形态学运算
	//颜色反向处理
	bitwise_not(output, output);//颜色反转
	//开启——去噪
	cv::Mat element(fushi, fushi, CV_8U, cv::Scalar(1));//腐蚀图像的结构元素//变大后有效去噪
	erode(output, output, element);//腐蚀cv::Mat()->element
	dilate(output, output, cv::Mat());//膨胀
	//关闭——连接碎片
	cv::Mat element1(pengzhang, pengzhang, CV_8U, cv::Scalar(1));
	dilate(output, output, element1);//cv::Mat()->element1
	erode(output, output, cv::Mat());

	/****指针扫描，比OPENCV的at方式快****/
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int nc = output.cols;//列数
	int nl = output.rows;//行数
	int x1min = nc - 1, x2max = 0, y1min = nl - 1, y2max = 0;

	for (int i = 0, q = 0; i < nl; i++)
	{//取得行i的地址
		uchar* data = output.ptr<uchar>(i);
		for (int j = 0; j < nc; j++)
		{
			if (data[j] == 255)
			{
				x1 = j;
				q = 1;
				break;
			}
		}
		if (q)
		{
			y1 = i;
			break;
		}
	}
	cv::line(output, cv::Point(0, y1), cv::Point(nc, y1), cv::Scalar(255, 0, 0), 1);

	for (int i = nl-1, q = 0; i>=y1; i--)
	{//取得行i的地址
		uchar* data = output.ptr<uchar>(i);
		for (int j = 0; j < nc; j++)
		{
			if (data[j] == 255)
			{
				x1 = j;
				q = 1;
				break;
	
			}
		}
		if (q)
		{
			y2 = i;
			break;
		}
	}
	cv::line(output, cv::Point(0, y2), cv::Point(nc, y2), cv::Scalar(255, 0, 0), 1);

	shengao = y2 - y1;
	shengao = shengao * 80 / (y2 - nl / 2);
	
	float dh;
	dh = shengao - h;
	h = shengao; 
	v = -dh * 10/100;//乘以帧速率，转换为m为单位,加负号取向下为正
	vjiao = (y2 - jiao) * 10 / 100;
	jiao = y2;
	//           照片 / 添加的文字 / 左下角坐标 / 字体 / 字体大小 / 颜色 / 字体粗细
	cv::putText(output, std::to_string(shengao)+ "   v" + std::to_string(v) + "   dh" + std::to_string(dh)+"   jiao"+to_string(vjiao), cv::Point(200, y1), cv::FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 1);
	h = y1;
	if (cFlag)//显示跟踪图
	{
		output1 = frame.clone();
		cv::line(output1, cv::Point(0, y1), cv::Point(nc, y1), cv::Scalar(255, 0, 0), 1);
		cv::line(output1, cv::Point(0, y2), cv::Point(nc, y2), cv::Scalar(255, 0, 0), 1);
		cv::putText(output1, std::to_string(shengao) + "   v" + std::to_string(v) + "   dh" + std::to_string(dh) + "   jiao" + to_string(vjiao), cv::Point(200, y1), cv::FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 1);
	}
}


/*******m_shitu的CBN_SELCHANGE消息处理函数*******/
void CMFCApplication1Dlg::OnCbnSelchangeShituCombo1()
{
	// TODO:  在此添加控件通知处理程序代码
	int nSel;
	nSel = m_shitu.GetCurSel();//获取列表框中选中的索引
	switch (nSel)
	{
	case 0:cFlag = 0;//选择显示类型
		break;
	default:cFlag = 1;//选择显示类型
		break;
	}

}



void CMFCApplication1Dlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_lvbo = m_lvboslider.GetPos();
	UpdateData(FALSE);//显示到编辑框
	lvbo = m_lvbo;
	*pResult = 0;
}


void CMFCApplication1Dlg::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_fushi = m_fushislider.GetPos();
	UpdateData(FALSE);//显示到编辑框
	fushi = m_fushi;
	*pResult = 0;
}


void CMFCApplication1Dlg::OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_pengzhang = m_pengzhangslider.GetPos();
	UpdateData(FALSE);//显示到编辑框
	pengzhang = m_pengzhang;
	*pResult = 0;
}


void CMFCApplication1Dlg::OnBnClickedBaocunButton4()
{
	// TODO:  在此添加控件通知处理程序代码
	//保存联系人数据<<<<<
	std::ofstream file("联系人.txt", std::ios::trunc);
	if (!file.is_open())
		MessageBox(_T("联系人文件打开失败"), _T("系统"), MB_ICONINFORMATION);
	else
	{
		for (int j = 0; j < i; j++)
		{
			file << lxr[j] + "\n";
		}
		file.close();
	}

}


void CMFCApplication1Dlg::On32771()
{
	// TODO:  在此添加命令处理程序代码
	ShellExecute(NULL,_T( "open"), _T(".\\help.chm"), NULL, NULL, SW_SHOWMAXIMIZED);

}
