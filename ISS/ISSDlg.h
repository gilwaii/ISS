
// ISSDlg.h : header file
//

#pragma once
struct Info
{
	char * ADDR;
	char * FPS;
	char * SIZE;
	char * QUAL;
	SOCKADDR_IN client;
};

// CISSDlg dialog
class CISSDlg : public CDialogEx
{
// Construction
public:
	CISSDlg(CWnd* pParent = nullptr);	// standard constructor

	

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CEdit edt_fps_;
	CEdit edt_size_;
	CEdit edt_qual_;
public:
	afx_msg void OnBnClickedBtnCam1();
	afx_msg void OnBnClickedBtnCam2();
	afx_msg void OnBnClickedBtnCam3();
	afx_msg void OnBnClickedBtnCam4();
	void changeButtonName(int, char*);
	void changeInfo(int, Info);
	afx_msg void OnBnClickedBtnApply();
	afx_msg void OnBnClickedBtnCam5();
	afx_msg void OnBnClickedBtnXcam1();
	afx_msg void OnBnClickedBtnXcam2();
	afx_msg void OnBnClickedBtnXcam3();
	afx_msg void OnBnClickedBtnXcam4();
	DWORD WINAPI processThread(LPVOID para);
	DWORD WINAPI print();
};

#pragma warning(disable: 4996)
#pragma comment(lib, "WS2_32.lib")

