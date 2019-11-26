#pragma once
#include "afxwin.h"

#include "maths.h"
#include "afxcmn.h"

// DialogTransToolRotation 对话框

class DialogTransToolRotation : public CDialog
{
	DECLARE_DYNAMIC(DialogTransToolRotation)

public:
	DialogTransToolRotation(CWnd* pParent = NULL);   // 标准构造函数
	virtual BOOL OnInitDialog();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGTRANSTOOLROTATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 旋转角度（单位角度制）
	point m_center;
	point m_offset;
	float m_rotationAngle;
	CComboBox m_interpMethod;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedTranstoolrotationButtonReset();
	CSliderCtrl m_thetaRatio;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CSpinButtonCtrl m_Angle_spin;
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin5(NMHDR *pNMHDR, LRESULT *pResult);
};
