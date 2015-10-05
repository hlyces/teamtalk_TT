#pragma once


// CVideoWnd �Ի���

class CVideoWnd : public CDialog
{
	DECLARE_DYNAMIC(CVideoWnd)

public:
	CVideoWnd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CVideoWnd();

// �Ի�������
	enum { IDD = IDD_VIDEO };
	CString m_strID;
	BITMAPINFO* m_pInfo;
	BYTE*       m_pData;
	BOOL        m_bShowVideoImage;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();

	void SetImage(LPBITMAPINFO pInfo,LPBYTE pData,BOOL bBK/*=FALSE*/);

};
