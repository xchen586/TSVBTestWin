// VidCapDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "CVBHandler.h"

// CVidCapDlg dialog
class CVidCapDlg : public CDialog
{
// Construction
public:
        CVidCapDlg(CWnd* pParent = NULL);	// standard constructor
		void MMTimerHandler(UINT nIDEvent);

// Dialog Data
        enum { IDD = IDD_VIDCAP_DIALOG };

protected:
        virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
        HICON m_hIcon;

        // Generated message map functions
        virtual BOOL OnInitDialog();
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        DECLARE_MESSAGE_MAP()

private:
		CString m_strBackgroundPath;

		UINT m_uResolution;
		MMRESULT m_idEvent;

        UINT_PTR m_nTimer;
        CStatic m_PrvStatic;
        CStatic m_CapImgStatic;
        CComboBox m_AdapterCombo;
        UINT m_nTimerInterval;
        CButton m_RunButton;
        CStatic m_VideoFormat;                
        bool m_TakeSnapshot;
        
		CVBHandler m_handler;

        CLSID pBmpEncoder;
        int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
        
        void DrawData(Gdiplus::Bitmap* pBitmap);        

		void OnDealWithTimer();
		void OnDealWithMultiMediaTimer();
		CString GetDefaultBackgroundImagePath();
		void DoCaptureFrame();
		void DoVBFrame();
		void DoProcessing();

		BOOL VBEnabled();

        afx_msg LRESULT OnGraphMessage(WPARAM wParam, LPARAM lParam);
        afx_msg void OnBnClickedEnumadaptorsButton();
        afx_msg void OnBnClickedRunButton();
        afx_msg void OnTimer(UINT_PTR nIDEvent);
		afx_msg void OnOK();
        afx_msg void OnClose();
		afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
        afx_msg void OnBnClickedFdetectCheck();       
        afx_msg void OnStnDblclickCapimgStatic();
		afx_msg void OnClickedCheckBlur();

public:
	CButton m_checkEnableBlur;
	
	BOOL m_bEnableBlur;
	afx_msg void OnClickedCheckReplace();
	CButton m_checkEnableReplace;
	BOOL m_bEnableReplace;
};

#ifdef _WIN64
void __stdcall TimerFunction(UINT wTimerID, UINT msg,
	DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
#else
void CALLBACK TimerFunction(UINT wTimerID, UINT msg,
	DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
#endif
