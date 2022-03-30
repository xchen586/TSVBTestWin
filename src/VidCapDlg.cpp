// VidCapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VidCap.h"
#include "VidCapDlg.h"

#include "capture.h"
#include "samplegrab.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define USE_MULTIMEDIA_TIMER 0


// CVidCapDlg dialog


CVidCapDlg::CVidCapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVidCapDlg::IDD, pParent)
	, m_nTimer(0)
#if USE_MULTIMEDIA_TIMER 
	, m_nTimerInterval(30)
#else
	, m_nTimerInterval(10)
#endif
	, m_idEvent(0), m_uResolution(30)
	, m_TakeSnapshot(false)
	, pBmpEncoder(GUID_NULL)
	, m_bEnableVB(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVidCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRV_STATIC, m_PrvStatic);
	DDX_Control(pDX, IDC_ADAPTORS_COMBO, m_AdapterCombo);
	DDX_Text(pDX, IDC_SAMPLEINTERVAL_EDIT, m_nTimerInterval);
	DDV_MinMaxUInt(pDX, m_nTimerInterval, 10, 10000);
	DDX_Control(pDX, IDC_RUN_BUTTON, m_RunButton);
	DDX_Control(pDX, IDC_CAPIMG_STATIC, m_CapImgStatic);
	DDX_Control(pDX, IDC_VIDINFO_STATIC, m_VideoFormat);
	DDX_Control(pDX, IDC_CHECK_VB, m_checkEnableVB);
	DDX_Check(pDX, IDC_CHECK_VB, m_bEnableVB);
}

BEGIN_MESSAGE_MAP(CVidCapDlg, CDialog)
	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphMessage)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ENUMADAPTORS_BUTTON, &CVidCapDlg::OnBnClickedEnumadaptorsButton)
	ON_BN_CLICKED(IDC_RUN_BUTTON, &CVidCapDlg::OnBnClickedRunButton)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	ON_STN_DBLCLK(IDC_CAPIMG_STATIC, &CVidCapDlg::OnStnDblclickCapimgStatic)
	ON_BN_CLICKED(IDC_CHECK_VB, &CVidCapDlg::OnClickedCheckVb)
END_MESSAGE_MAP()


// CVidCapDlg message handlers
LRESULT CVidCapDlg::OnGraphMessage(WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = vcHandleGraphEvent();
	TRACE(L" WM_GRAPH 0x%X\n", hr);
	return 0;
}

BOOL CVidCapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	if (!m_handler.isInitialized()) {
		MessageBox(L"Fail to load VB SDK!", L"VB error");
		PostMessage(WM_CLOSE);
		return FALSE;
	}

	sgSetVBHandler(&m_handler);
	// Initialize COM
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		MessageBox(L"CoInitialize Failed!", L"COM error");
		m_RunButton.EnableWindow(FALSE);
		return TRUE;
	}

	if (GetEncoderClsid(L"image/jpeg", &pBmpEncoder) < 0) {
		MessageBox(L"Failed to get image/jpeg encoder", L"warning");
	}

	vcGetCaptureDevices(m_AdapterCombo);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVidCapDlg::OnOK()
{
	return;
}

void CVidCapDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(m_nTimer);
	vcStopCaptureVideo();
	CoUninitialize();

	m_handler.releaseResource();
	CDialog::OnClose();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVidCapDlg::OnPaint()
{
	if (IsIconic()) {
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
	else {
		CDialog::OnPaint();
	}
}


int CVidCapDlg::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j) {
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVidCapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVidCapDlg::OnBnClickedEnumadaptorsButton()
{
	//Enum adaptors
	vcGetCaptureDevices(m_AdapterCombo);
}

void CVidCapDlg::OnBnClickedRunButton()
{
	UpdateData();
	BOOL enable = m_checkEnableVB.IsWindowEnabled();
	m_checkEnableVB.EnableWindow(!enable);

#if USE_MULTIMEDIA_TIMER
	OnDealWithMultiMediaTimer();
#else
	OnDealWithTimer();
#endif
}

void CVidCapDlg::OnDealWithTimer()
{
	HRESULT hr;
	if (m_nTimer == 0) {
		//Run capture
		hr = vcCaptureVideo(m_hWnd, m_PrvStatic.m_hWnd, m_AdapterCombo.GetCurSel() + 1);
		if (hr != S_OK) {
			vcStopCaptureVideo();
			return;
		}

		CString str;
		str.Format(L"Video output: %dx%d %dbpp", sgGetDataWidth(), sgGetDataHeight(), 8 * sgGetDataChannels());
		m_VideoFormat.SetWindowTextW(str);

		//Setup Timer
		m_nTimer = SetTimer(1, m_nTimerInterval, 0);

		m_RunButton.SetWindowTextW(L"Stop");
	}
	else {
		//Close Timer
		KillTimer(m_nTimer);
		m_nTimer = 0;
		m_RunButton.SetWindowTextW(L"Run");

		m_VideoFormat.SetWindowTextW(L"Video output");
		//Close Capture
		vcStopCaptureVideo();
	}
}

void CVidCapDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	DoProcessing();
	CDialog::OnTimer(nIDEvent);
}

void CVidCapDlg::DoProcessing() 
{
	if (m_bEnableVB) {
		DoVBFrame();
	}
	else {
		DoCaptureFrame();
	}
}

void CVidCapDlg::DoCaptureFrame()
{
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);
	TRACE(L" %d:%d:%d\n", SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);

	unsigned char* pData = sgGrabRGB32Data();
	if (pData != 0) {
		DrawData(sgGetCaptureBitmap());
	}
	else {
		TRACE(L"Fail to capture frame data");
		return;
	}
		
}

void CVidCapDlg::DoVBFrame()
{
	Gdiplus::Bitmap * ret = sgGetVBBitmap();
	if (ret) {
		DrawData(ret);
	}

}

void CVidCapDlg::OnDealWithMultiMediaTimer()
{

	HRESULT hr;
	if (!m_idEvent) {
		//Run capture
		hr = vcCaptureVideo(m_hWnd, m_PrvStatic.m_hWnd, m_AdapterCombo.GetCurSel() + 1);
		if (hr != S_OK) {
			vcStopCaptureVideo();
			return;
		}

		CString str;
		str.Format(L"Video output: %dx%d %dbpp", sgGetDataWidth(), sgGetDataHeight(), 8 * sgGetDataChannels());
		m_VideoFormat.SetWindowTextW(str);

		// Set resolution to the minimum supported by the system
		TIMECAPS tc;
		timeGetDevCaps(&tc, sizeof(TIMECAPS));
		m_uResolution = min(max(tc.wPeriodMin, m_nTimerInterval), tc.wPeriodMax);
		timeBeginPeriod(m_uResolution);

		// create the timer
		m_idEvent = timeSetEvent(
			30,
			m_uResolution,
			TimerFunction,
			(DWORD_PTR)this,
			TIME_PERIODIC);

		m_RunButton.SetWindowTextW(L"Stop");
	}
	else {
		// destroy the timer
		timeKillEvent(m_idEvent);
		m_idEvent = 0;
		// reset the timer
		timeEndPeriod(m_uResolution);

		m_RunButton.SetWindowTextW(L"Run");

		m_VideoFormat.SetWindowTextW(L"Video output");
		//Close Capture
		vcStopCaptureVideo();
	}


}


#ifdef _WIN64
void __stdcall TimerFunction(UINT wTimerID, UINT msg,
	DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
#else
void CALLBACK TimerFunction(UINT wTimerID, UINT msg,
	DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
#endif
{
	// This is used only to call MMTimerHandler

	// Typically, this function is static member of CTimersDlg

	CVidCapDlg* obj = (CVidCapDlg*)dwUser;
	obj->MMTimerHandler(wTimerID);
}

void CVidCapDlg::MMTimerHandler(UINT nIDEvent) // called every elTime milliseconds
{
	// do what you want to do, but quickly
	DoProcessing();
}


void CVidCapDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanged(lpwndpos);

	// TODO: Add your message handler code here
	vcChangePreviewState(!IsIconic());
}

void CVidCapDlg::DrawData(Gdiplus::Bitmap *pBitmap)
{
	if (pBitmap == 0)
		return;

	if (m_TakeSnapshot == true) {
		m_TakeSnapshot = false;
		sndPlaySound(L"snap.wav", SND_ASYNC);
		if (pBmpEncoder != GUID_NULL) {
			wchar_t FileName[_MAX_PATH] = L"";
			for (unsigned int i = 1; i < 0xFFFFFFFF; i++) {
				swprintf_s(FileName, _MAX_PATH, L"Snapshot %04d.jpg", i);
				FILE* fp = _wfopen(FileName, L"rb");
				if (fp == 0) {
					pBitmap->Save(FileName, &pBmpEncoder);
					break;
				}
				else
					fclose(fp);

			}
		}
	}

	RECT r;
	m_CapImgStatic.GetClientRect(&r);
	Gdiplus::Graphics g(m_CapImgStatic.GetDC()->m_hDC);

	g.DrawImage(pBitmap, Gdiplus::Rect(0, 0, r.right, r.bottom));

	//UpdateData(FALSE);
}

void CVidCapDlg::OnStnDblclickCapimgStatic()
{
	m_TakeSnapshot = true;
}



void CVidCapDlg::OnClickedCheckVb()
{
	// TODO: Add your control notification handler code here
	UpdateData();
}
