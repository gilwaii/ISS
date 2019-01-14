
// ISSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISS.h"
#include "ISSDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int flags[5];

//so luong goi tin toi da
#define MAX_CONNECT 10

// kich thuoc goi tin
#define SIZE_PACKAGE 4096

//so luong cua so toi da
#define NUMBER_WIN 4

//kich thuoc cua queue
#define SIZE_QUEUE 15

// cau truc thong diep nhan
typedef struct message
{
	int id;
	int size;
	int stt;
	char data[SIZE_PACKAGE];
}MESSAGE;



DWORD __stdcall startMethodInThread(LPVOID arg);
// cau truc thong diep gui
//typedef struct command
//{
//	int fps;
//	int quality;
//	int size;
//}COMMAND;
void command(SOCKADDR_IN, char*);

// xay dung Queue
typedef struct queue {
	MESSAGE data[SIZE_QUEUE];
	int front;
	int back;
}QUEUE;

//destroy
void destroy(SOCKADDR_IN client, char *windowName);

//buffer nhan du lieu
char buff[5200];

//reset info[] client
void resetInfo(int i);

//
void convert(char *arr, double number); // ham chuyen doi so thuc sang so xau ki tu

// dinh danh luong
long cookie[NUMBER_WIN];

// mang socketclient tcp
SOCKET tcpClients[NUMBER_WIN];

// so cua so hien tai dang mo
int numOfThread = 0;

//ham thong bao loi
void messageError();

//khai bao socket trao doi du lieu
SOCKET udpSocket;

// Tên cửa sổ

Info info[4];
char window_name[4][32];

// luong lang nghe
DWORD WINAPI server(LPVOID);

// 
DWORD WINAPI processConnect(LPVOID);

// luong xu li theo doi tung client
DWORD WINAPI processThread(LPVOID);

// ham xu li hien thi anh
void processPicture(char *buffWin, int size, char windowName[]);

// cac thao tac voi queue
int emptyQueue(QUEUE* q); // kiem tra queue rong
int fullQueue(QUEUE* q); // kiem tra queue day
int enQueue(QUEUE* q, MESSAGE item); // them phan tu vao queue
int deQueue(QUEUE* q, MESSAGE *out); // xoa phan tu khoi queue
int sizeQueue(QUEUE *q); // tra ve so phan tu cua queue


// hang doi xu li cac goi tin
QUEUE messageQueue[NUMBER_WIN];



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CISSDlg dialog



CISSDlg::CISSDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CISSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EDT_FPS, edt_fps_);
	//DDX_Control(pDX, IDC_EDT_SIZE, edt_size_);
	DDX_Control(pDX, IDC_EDT_QUAL, edt_qual_);
}

BEGIN_MESSAGE_MAP(CISSDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CAM1, &CISSDlg::OnBnClickedBtnCam1)
	ON_BN_CLICKED(IDC_BTN_CAM2, &CISSDlg::OnBnClickedBtnCam2)
	ON_BN_CLICKED(IDC_BTN_CAM3, &CISSDlg::OnBnClickedBtnCam3)
	ON_BN_CLICKED(IDC_BTN_CAM4, &CISSDlg::OnBnClickedBtnCam4)
	ON_BN_CLICKED(IDC_BTN_APPLY, &CISSDlg::OnBnClickedBtnApply)
	ON_BN_CLICKED(IDC_BTN_XCAM1, &CISSDlg::OnBnClickedBtnXcam1)
	ON_BN_CLICKED(IDC_BTN_XCAM2, &CISSDlg::OnBnClickedBtnXcam2)
	ON_BN_CLICKED(IDC_BTN_XCAM3, &CISSDlg::OnBnClickedBtnXcam3)
	ON_BN_CLICKED(IDC_BTN_XCAM4, &CISSDlg::OnBnClickedBtnXcam4)
END_MESSAGE_MAP()


// CISSDlg message handlers

BOOL CISSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int ii;
	for (int i = 0; i < 4; i++) {
		resetInfo(i);
	}
	flags[4] = 0;
	CreateThread(NULL, NULL, server, (LPVOID)0, NULL, NULL);
	CreateThread(NULL, NULL, startMethodInThread, this, NULL, NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CISSDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CISSDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CISSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

char* CStringtoChar(CString cstring)
{
	long nSize = cstring.GetLength();
	char* str = new char[nSize];
	memset(str, 0, nSize);
	wcstombs(str, cstring, nSize + 1);
	return str;
}

void flag(int);
void CISSDlg::OnBnClickedBtnCam1()
{
	if (info[0].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		// TODO: Add your control notification handler code here
		flag(0);
	}
}


void CISSDlg::OnBnClickedBtnCam2()
{
	if (info[1].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		flag(1);
	}
	// TODO: Add your control notification handler code here
}


void CISSDlg::OnBnClickedBtnCam3()
{
	if (info[2].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		flag(2);
		// TODO: Add your control notification handler code here
	}
}


void CISSDlg::OnBnClickedBtnCam4()
{
	if (info[3].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		flag(3);
	}
	// TODO: Add your control notification handler code here
}

void CISSDlg::OnBnClickedBtnApply()
{
	int i;
	// TODO: Add your control notification handler code here
	for (i = 0; i < 4; i++) {
		if (flags[i] == 1) break;
	}
	if (flags[i] == 0)
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		CString str_qual = _T("");
		CString str_fps = _T("");
		CString str_size = _T("");

		edt_qual_.GetWindowText(str_qual);
		//edt_fps_.GetWindowText(str_fps);
		//edt_size_.GetWindowText(str_size);

		char* qual = CStringtoChar(str_qual);
		//char* fps = CStringtoChar(str_fps);
		//char* size = CStringtoChar(str_size);

		Info oldInfo;
		oldInfo = info[i];

		if (qual == "")
			info[i].QUAL = oldInfo.QUAL;
		else 
			info[i].QUAL = qual;
		//if (size == "")
		//	info[i].SIZE = oldInfo.SIZE;
		//else 
		//	info[i].SIZE = size;
		//if (fps == "")
		//	info[i].FPS = oldInfo.FPS;
		//else
		//	info[i].FPS = fps;
		info[i].ADDR = oldInfo.ADDR;
		changeInfo(i, info[i]);
		command(info[i].client, qual);
	}
}

void CISSDlg::changeButtonName(int i, char*name) {
	if (strlen(name) < 0) return;
	if (i == 0)
	{
		wchar_t *wmsg5 = new wchar_t[strlen(name) + 1];
		mbstowcs(wmsg5, name, strlen(name) + 1);
		SetDlgItemText(IDC_BTN_CAM1, wmsg5);
		delete[]wmsg5;
	}
	else if (i == 1)
	{
		wchar_t *wmsg6 = new wchar_t[strlen(name) + 1];
		mbstowcs(wmsg6, name, strlen(name) + 1);
		SetDlgItemText(IDC_BTN_CAM2, wmsg6);
		delete[] wmsg6;
	}
	else if (i == 2)
	{
		wchar_t *wmsg7 = new wchar_t[strlen(name) + 1];
		mbstowcs(wmsg7, name, strlen(name) + 1);
		SetDlgItemText(IDC_BTN_CAM3, wmsg7);
		delete[] wmsg7;
	}
	else if (i == 3)
	{
		wchar_t *wmsg8 = new wchar_t[strlen(name) + 8];
		mbstowcs(wmsg8, name, strlen(name) + 1);
		SetDlgItemText(IDC_BTN_CAM4, wmsg8);

		delete[]wmsg8;
	}
}

void CISSDlg::changeInfo(int i, Info info) {
	// Change address
	wchar_t *wmsg1 = new wchar_t[20 + 1];
	mbstowcs(wmsg1, info.ADDR, 20 + 1);
	SetDlgItemText(IDC_STT_ADDR, wmsg1);
	delete[]wmsg1;

	// Change size
	wchar_t *wmsg2 = new wchar_t[20];
	mbstowcs(wmsg2, info.SIZE, 20);
	SetDlgItemText(IDC_STT_SIZE, wmsg2);
	delete[]wmsg2;

	// Change quanlity
	wchar_t *wmsg3 = new wchar_t[20];
	mbstowcs(wmsg3, info.QUAL, 20);
	SetDlgItemText(IDC_STT_QUAL, wmsg3);
	delete[]wmsg3;

	// Change FPS
	wchar_t *wmsg4 = new wchar_t[20];
	mbstowcs(wmsg4, info.FPS, 20);
		SetDlgItemText(IDC_STT_FPS, wmsg4);
	delete[]wmsg4;
}

void flag(int i) {
	for (int j = 0; j < 4; j++) {
		if (j == i) flags[j] = 1;
		else flags[j] = 0;
	}
}


void CISSDlg::OnBnClickedBtnXcam1()
{
	if (info[0].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		// TODO: Add your control notification handler code here
		destroy(info[0].client, info[0].ADDR);
		resetInfo(0);
		changeInfo(0, info[0]);
	}
}


void CISSDlg::OnBnClickedBtnXcam2()
{
	if (info[1].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		// TODO: Add your control notification handler code here
		destroy(info[1].client, info[1].ADDR);
		resetInfo(1);
		changeInfo(1, info[1]);
	}
}


void CISSDlg::OnBnClickedBtnXcam3()
{
	if (info[2].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		// TODO: Add your control notification handler code here
		destroy(info[2].client, info[2].ADDR);
		resetInfo(2);
		changeInfo(2, info[2]);
	}
}


void CISSDlg::OnBnClickedBtnXcam4()
{
	if (info[3].ADDR == "NULL")
	{
		MessageBox(_T("No camera connect!!!!"), _T("Error"), MB_OK | MB_ICONERROR);
	}
	else {
		// TODO: Add your control notification handler code here
		destroy(info[3].client, info[3].ADDR);
		resetInfo(3);
		changeInfo(3, info[3]);
	}
}

////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI server(LPVOID para)
{
	//init winsock
	WORD version = MAKEWORD(2, 2);
	WSADATA infoWin;
	WSAStartup(version, &infoWin);


	//configure server
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(8888);

	// creat listen socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// bind listen socket with interface network

	int re = 0;
	re = bind(udpSocket, (const sockaddr*)&server, sizeof(server));

	if (re == SOCKET_ERROR)
	{
		messageError();
		return -1;
	}


	//goi luong xu li ket noi
	CreateThread(0, 0, processConnect, 0, 0, 0);

	// use select model
	fd_set fdread;
	while (true)
	{
		// delete set fdread
		FD_ZERO(&fdread);
		// add updSocket into fdread
		FD_SET(udpSocket, &fdread);
		// wating event
		re = select(0, &fdread, NULL, NULL, NULL);
		if (re == SOCKET_ERROR)
		{
			messageError();
			return -1;
		}

		//info client
		SOCKADDR_IN client;
		int size = sizeof(client);

		re = recvfrom(udpSocket, buff, sizeof(buff), 0, (sockaddr*)&client, &size);
		// xu li goi tin nhan duoc
		// neu loi khong nhan duoc du lieu
		if (re <= 0)
			continue;
		// neu nhan duoc goi tin
		// neu khong phai la du lieu hinh anh

		if (re == sizeof(MESSAGE))
		{
			// day la du lieu hinh anh
			// xep no vao hang doi cua client tuong ung
			for (int i = 0; i < NUMBER_WIN; i++)
			{
				if (cookie[i] == client.sin_addr.s_addr)
				{
					enQueue(&messageQueue[i], *(MESSAGE *)buff);
					break;
				}
			}
		}
		else
			continue;

	}

	WSACleanup();
}

void messageError()
{
	printf("\nerror %d", WSAGetLastError());
	system("pause");
}

DWORD __stdcall startMethodInThread(LPVOID arg)
{
	if (!arg)
		return 0;
	CISSDlg *yc_ptr = (CISSDlg*)arg;
	yc_ptr->print();
	return 1;
}

DWORD WINAPI CISSDlg::print() {
	int i = 0;
	long currentStart = clock();
	while (true)
	{	
		if (clock() - currentStart > 50)
		{
			changeButtonName(0, info[0].ADDR);
			changeButtonName(1, info[1].ADDR);
			changeButtonName(2, info[2].ADDR);
			changeButtonName(3, info[3].ADDR);
			if(flags[0] == 1) changeInfo(0, info[0]);
			else if(flags[1]) changeInfo(1, info[1]);
			else if (flags[2]) changeInfo(2, info[2]);
			else if (flags[3]) changeInfo(3, info[3]);
			currentStart = clock();
		}
	}
	return (DWORD)0;
}
DWORD WINAPI processThread(LPVOID para)
{
	// tao cua truc chua dia chi client
	SOCKADDR_IN client;
	client = *(SOCKADDR_IN *)para;

	//tim dinh danh cua luong tuong ung voi luong nao
	int i = 0;
	for (i = 0; i < NUMBER_WIN; i++)
	{
		if (cookie[i] == client.sin_addr.s_addr)
		{
			break;
		}
	}

	int currentID = -1; // ID cua anh hien tai nhan
	int oldID = 0; // ID cua anh da bi huy truoc do
	int tmpID = -1; // ID cua frame ke tiep
	int count = 0; // dem so goi tin den cua frame hien tai
	int countTmp = 0; // dem so goi tin den cua frame ke tiep frame hien tai
	MESSAGE tmp; // goi tin bi lay ra tu hang doi de xu li
	char *buffWin = NULL; // bo dem phuc vu cho rap goi tin cua frame hien tai
	char *buffWinTmp = NULL; // bo dem phuc vu cho viec rap goi tin ke tiep cua goi tin hien tai trong truong hop ma no den som
	char fps[10]; // tinh fps
	clock_t currentStart, currentEnd; // bo dem thoi gian cpu cho frame xu li hien tai
	double cpu_time;
	int loss = 0;

	// tao cua so trinh chieu cho client hien tai
	char window_name[32];
	strcpy(window_name, inet_ntoa(client.sin_addr));
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	info[i].ADDR = window_name;
	if (strlen(info[i].ADDR) == 0) resetInfo(i);
	info[i].client = client;
	info[i].SIZE = "640 x 480";

	while (1)
	{
		// lay ra goi tin de xu li
		if (deQueue(&messageQueue[i], &tmp))
		{
			loss = 0;
			// neu day la goi tin dau tien ma server nhan duoc tu mot frame moi
			if (currentID == -1)
			{
				currentStart = clock();

				buffWin = (char *)malloc(sizeof(char) * SIZE_PACKAGE * tmp.size);
				currentID = tmp.id;
				memcpy(buffWin + SIZE_PACKAGE * tmp.stt, tmp.data, SIZE_PACKAGE);
				count++;
			}
			else if (currentID == tmp.id) // neu day la goi tin cua frame dang duoc xu li
			{
				memcpy(buffWin + SIZE_PACKAGE * tmp.stt, tmp.data, SIZE_PACKAGE);
				count++;
				if (count == tmp.size) // neu nhan du mot frame
				{
					// xu li frame
					processPicture(buffWin, count, window_name);
					// ket thuc xu li frame hien tai

					currentEnd = clock();
					cpu_time = (double)(currentEnd - currentStart) / CLOCKS_PER_SEC;
					if (cpu_time > 0) {
						char fps[10];
						convert(fps, 1 / cpu_time);
						info[i].FPS = fps;
					}
						currentStart = currentEnd;
					// giai phong vung nho cua frame hien tai
					free(buffWin);
					// nhan frame ke tiep
					// kiem tra bo dem frame ke tiep xem co du lieu khong
					if (buffWinTmp != NULL)
					{
						currentID = tmpID;
						count = countTmp;
						buffWin = buffWinTmp;

						tmpID = -1;
						buffWinTmp = NULL;
						countTmp = 0;
					}
					else
					{
						currentID = -1;
						count = 0;
						buffWin = NULL;
					}

				}
			}
			else // truong hop goi tin den khong phai la cua frame hien tai
			{
				if (oldID != tmp.id) // kiem tra xem goi tin nay co phai cua frame da bi huy hay khong
				{
					if (tmpID == -1) // day la goi tin cua frame moi, kiem tra xem neu vung dem trong
					{
						buffWinTmp = (char *)(malloc(sizeof(char) * SIZE_PACKAGE * tmp.size));
						memcpy(buffWinTmp + SIZE_PACKAGE * tmp.stt, tmp.data, SIZE_PACKAGE);
						countTmp++;
						tmpID = tmp.id;

					}
					else if (tmpID == tmp.id)
					{
						memcpy(buffWinTmp + SIZE_PACKAGE * tmp.stt, tmp.data, SIZE_PACKAGE);
						countTmp++;
						if (countTmp == tmp.size) // dieu kien mat goi tin
						{
							// frame hien tai bi hong do den qua tre
							// huy frame hien tai
							info[i].FPS = "loss";
							free(buffWin);
							count = 0;
							buffWin = NULL;
							oldID = currentID;
							currentID = -1;

							// xu li frame moi nhan duoc
							processPicture(buffWinTmp, countTmp, window_name);

							currentEnd = clock();
							cpu_time = (double)(currentEnd - currentStart) / CLOCKS_PER_SEC;
							if (cpu_time > 0) {
								char fps[10];
								convert(fps, 1 / cpu_time);
								info[i].FPS = fps;
							}
								currentStart = currentEnd;
							//giai phong tai nguyen ban dau de bat dau chu ki moi
							countTmp = 0;
							free(buffWinTmp);
							buffWinTmp = NULL;
							tmpID = -1;
						}
					}
					else // goi tin cua frame + 3 da den => frame hien tai khong co kha nang hoan thanh
					{
						oldID = currentID;
						free(buffWin);
						currentID = tmpID;
						count = countTmp;
						buffWin = buffWinTmp;

						buffWinTmp = (char *)(malloc(sizeof(char) * SIZE_PACKAGE * tmp.size));
						memcpy(buffWinTmp + SIZE_PACKAGE * tmp.stt, tmp.data, SIZE_PACKAGE);
						countTmp = 1;
						tmpID = tmp.id;
					}
				}
				else
				{
					printf("\ncurrent %d", currentID);
					printf("\nframe old %d", tmp.id);
				}
			}
		}
		else
		{
		loss++;
		// kiem tra xem phia client co con hoat dong hay khong
		if (loss > 100000000)
		{
			cv::destroyWindow(window_name);
			resetInfo(i);
			return 0;
		}
		}
		// Kiểm tra tín hiệu kết thúc luồng
		/*if(cookie[i] < 0)
		{
			resetInfo(i);
			return 0;
		}*/
	}
}

void processPicture(char *buffWin, int size, char windowName[])
{
	cv::Mat rawData = cv::Mat(1, (SIZE_PACKAGE)* size, CV_8UC1, buffWin);
	cv::Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
	if (frame.size().width == 0) {
		return;
	}
	imshow(windowName, frame);
	cv::waitKey(1);
}


int emptyQueue(QUEUE* q)
{
	return q->front == q->back;
}

int fullQueue(QUEUE* q)
{
	//return (SIZE_QUEUE - q->front + q->back) % SIZE_QUEUE == SIZE_QUEUE - 1;

	if (q->back > q->front)
	{
		return (q->back - q->front) == SIZE_QUEUE - 1;
	}
	else if ((q->back < q->front))
	{
		return (q->back - q->front) == -1;
	}
	return 0;

}


int enQueue(QUEUE* q, MESSAGE item)
{
	if (fullQueue(q))
	{
		return 0;
	}
	else
	{
		q->data[q->back] = item;
		q->back = (q->back + 1) % SIZE_QUEUE;
	}
}


int deQueue(QUEUE* q, MESSAGE *out)
{
	if (emptyQueue(q))
	{
		return 0;
	}
	else
	{
		*out = q->data[q->front];
		q->front = (q->front + 1) % SIZE_QUEUE;
		return 1;
	}
}

int sizeQueue(QUEUE *q)
{
	if (q->back > q->front)
	{
		return q->back - q->front;
	}
	else if (q->back < q->front)
	{
		return SIZE_QUEUE - q->front + q->back;
	}
	else
		return 0;
}

void resetInfo(int i) {
	info[i].ADDR = "NULL";
	info[i].FPS = "NULL";
	info[i].QUAL = "NULL";
	info[i].SIZE = "NULL";
	flags[i] = 0;
}

void convert(char *arr, double number)
{
	int c = (int)1000 * number;
	itoa(c, arr, 10);
	int i = strlen(arr);
	arr[i + 1] = '\0';
	arr[i] = arr[i - 1];
	arr[i - 1] = arr[i - 2];
	arr[i - 2] = arr[i - 3];
	arr[i - 3] = '.';
}

/*
	goi loi chao ket thuc viec truyen nhan goi tin voi client
*/
void destroy(SOCKADDR_IN client, char *windowName)
{
	for (int i = 0; i < NUMBER_WIN; i++)
	{
		if (client.sin_addr.s_addr == cookie[i])
		{
			send(tcpClients[i], "bye", strlen("bye"), 0);
			closesocket(tcpClients[i]);
			cookie[i] = -1;
			tcpClients[i] = -1;
			break;
		}
	}
	cv::destroyWindow(windowName);
}

/*
	danh thuc client va yeu cau treaming
*/

void weakup(SOCKADDR_IN client)
{
	sendto(udpSocket, "weakup", strlen("weakup"), 0, (const sockaddr *)&client, sizeof(client));
}


void command(SOCKADDR_IN client, char * q)
{
	char qua[10];
	strcpy(qua, "q");
	strcat(qua, q);
	for (int i = 0; i < NUMBER_WIN; i++)
	{
		if (client.sin_addr.s_addr == cookie[i])
		{
			send(tcpClients[i], qua, strlen(qua), 0);
			break;
		}
	}
}

DWORD WINAPI processConnect(LPVOID para)
{
	//configure server
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(9999);

	// creat listen socket
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// bind listen socket with interface network

	int re = 0;
	re = bind(listenSock, (const sockaddr*)&server, sizeof(server));

	if (re == SOCKET_ERROR)
	{
		messageError();
		return -1;
	}

	// listen connect
	re = listen(listenSock, 10);
	if (re == SOCKET_ERROR)
	{
		messageError();
		return -1;
	}

	//info client
	SOCKADDR_IN client;
	int size = sizeof(client);
	SOCKET tcpClient;

	// khoi tao cookie va khoi tao hang doi thong diep
	for (int i = 0; i < NUMBER_WIN; i++)
	{
		cookie[i] = -1;
		tcpClients[i] = -1;
		messageQueue[i].back = 0;
		messageQueue[i].front = 0;
	}

	while (1)
	{
		if (numOfThread < NUMBER_WIN)
		{
			tcpClient = accept(listenSock, (sockaddr*)&client, &size);
			// chap nhan ket noi, vao tao luong xu li cho client
			for (int i = 0; i < NUMBER_WIN; i++)
			{
				if (cookie[i] == -1)
				{
					// luu thong tin cua client moi
					cookie[i] = client.sin_addr.s_addr;
					//luu lai dia chi socket client
					tcpClients[i] = tcpClient;
					// tao luong xu li cac goi tin den
					CreateThread(NULL, NULL, processThread, (LPVOID)&client, NULL, NULL);
					numOfThread++;
					break;
				}
			}


			/*_sleep(10000);
			send(tcpClient, "q1", 2, 0);*/

			/*_sleep(10000);
			send(tcpClient, "bye", 3, 0);*/


		}
	}

}