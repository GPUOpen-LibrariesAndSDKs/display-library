// DDCBlockAccessDlg.h : header file
//

#pragma once

#define MAX_NUM_DISPLAY_DEVICES             16

// CDDCBlockAccessDlg dialog
class CDDCBlockAccessDlg : public CDialog
{
// Construction
public:
	CDDCBlockAccessDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DDCBlockAccess_DIALOG };

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

public:
  CComboBox cb_VCPCode;
  CComboBox m_indexList;
  CComboBox m_deviceList;
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedButtonGet();
  afx_msg void OnBnClickedButtonSet();
  afx_msg void OnDestroy();
  afx_msg void OnCbnSelchangeComboVcpcode();
  void RefreshDisplayIndices ();
  afx_msg void OnCbnSelchangeComboDisplayindex();
  afx_msg void OnCbnSelchangeComboDriverindex();
};
