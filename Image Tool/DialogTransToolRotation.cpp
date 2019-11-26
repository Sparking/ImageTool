// DialogTransToolRotation.cpp : 实现文件
//

#include "stdafx.h"
#include "Image Tool.h"
#include "DialogTransToolRotation.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "Image ToolDoc.h"
#include "Image ToolView.h"


// DialogTransToolRotation 对话框

IMPLEMENT_DYNAMIC(DialogTransToolRotation, CDialog)

DialogTransToolRotation::DialogTransToolRotation(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOGTRANSTOOLROTATION, pParent)
	, m_rotationAngle(0)
	, m_center{ 0, 0 }
	, m_offset{ 0, 0 }
{
	//m_rotationAngle = 0.5;
}

BOOL DialogTransToolRotation::OnInitDialog()
{
	if (!CDialog::OnInitDialog())
		return FALSE;

	m_interpMethod.AddString(_T("邻近"));
	m_interpMethod.InsertString(1, _T("双线性"));
	m_interpMethod.InsertString(2, _T("双立方"));
	m_interpMethod.InsertString(3, _T("lanzcos"));
	m_interpMethod.SetCurSel(0);
	m_thetaRatio.SetRange(0,3600);
	m_thetaRatio.SetLineSize(1);
	m_thetaRatio.SetPos(1800);
	m_Angle_spin.SetRange32(-180,180);
	m_Angle_spin.SetBase(10);
	return TRUE;
}

void DialogTransToolRotation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_rotationAngle);
	DDV_MinMaxFloat(pDX, m_rotationAngle, -180.0, 180.0);
	DDX_Text(pDX, IDC_EDIT2, m_center.x);
	DDX_Text(pDX, IDC_EDIT3, m_center.y);
	DDX_Text(pDX, IDC_EDIT4, m_offset.x);
	DDX_Text(pDX, IDC_EDIT5, m_offset.y);
	DDX_Control(pDX, IDC_COMBO1, m_interpMethod);
	DDX_Control(pDX, IDC_SLIDER1, m_thetaRatio);
	DDX_Control(pDX, IDC_SPIN1, m_Angle_spin);
}


BEGIN_MESSAGE_MAP(DialogTransToolRotation, CDialog)
	ON_BN_CLICKED(IDOK, &DialogTransToolRotation::OnBnClickedOk)
	ON_BN_CLICKED(IDC_TRANSTOOLROTATION_BUTTON_RESET, &DialogTransToolRotation::OnBnClickedTranstoolrotationButtonReset)
	ON_EN_CHANGE(IDC_EDIT1, &DialogTransToolRotation::OnEnChangeEdit1)
	ON_WM_HSCROLL(IDC_SLIDER1, &DialogTransToolRotation::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar))
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &DialogTransToolRotation::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &DialogTransToolRotation::OnDeltaposSpin2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &DialogTransToolRotation::OnDeltaposSpin3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &DialogTransToolRotation::OnDeltaposSpin4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, &DialogTransToolRotation::OnDeltaposSpin5)
END_MESSAGE_MAP()


// DialogTransToolRotation 消息处理程序


void DialogTransToolRotation::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	UpdateData(TRUE);
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), TRUE);
}


void DialogTransToolRotation::OnBnClickedTranstoolrotationButtonReset()
{
	// TODO: 在此添加控件通知处理程序代码
	m_rotationAngle = 0.0f;
	m_thetaRatio.SetPos(0);
	m_center.x = m_center.y = 0;
	m_offset.x = m_offset.y = 0;
	m_interpMethod.SetCurSel(0);
	UpdateData(FALSE);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}

void DialogTransToolRotation::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_thetaRatio.SetPos((int)m_rotationAngle*10+1800);
	Invalidate();
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}

void DialogTransToolRotation::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (*pScrollBar == m_thetaRatio) {
		m_rotationAngle = 0.1f*(m_thetaRatio.GetPos()-1800);
		UpdateData(FALSE);
	}
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}



void DialogTransToolRotation::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString fmt(_T(""));
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData(TRUE);
	if (pNMUpDown->iDelta == 1)
	{
		m_rotationAngle = m_rotationAngle + 0.1;
	}
	else if (pNMUpDown->iDelta == -1)
	{
		m_rotationAngle = m_rotationAngle - 0.1;
	}
	fmt.Format(_T("%.2f"), m_rotationAngle);
	SetDlgItemText(IDC_EDIT1, fmt);
	m_thetaRatio.SetPos((int)m_rotationAngle*10+1800);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}


void DialogTransToolRotation::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData(TRUE);
	if (pNMUpDown->iDelta == 1)
	{
		m_center.x += 1;
	}
	else if (pNMUpDown->iDelta == -1)
	{
		m_center.x -= 1;
	}
	UpdateData(FALSE);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}


void DialogTransToolRotation::OnDeltaposSpin3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData(TRUE);
	if (pNMUpDown->iDelta == 1)
	{
		m_center.y += 1;
	}
	else if (pNMUpDown->iDelta == -1)
	{
		m_center.y -= 1;
	}
	UpdateData(FALSE);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}


void DialogTransToolRotation::OnDeltaposSpin4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData(TRUE);
	if (pNMUpDown->iDelta == 1)
	{
		m_offset.x += 1;
	}
	else if (pNMUpDown->iDelta == -1)
	{
		m_offset.x -= 1;
	}
	UpdateData(FALSE);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}


void DialogTransToolRotation::OnDeltaposSpin5(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData(TRUE);
	if (pNMUpDown->iDelta == 1)
	{
		m_offset.y += 1;
	}
	else if (pNMUpDown->iDelta == -1)
	{
		m_offset.y -= 1;
	}
	UpdateData(FALSE);
	CMainFrame *pParentWnd;
	CImageToolView *pView;
	CImageToolDoc *pDoc;

	pParentWnd = (CMainFrame *)(this->m_pParentWnd);
	pView = (CImageToolView *)(pParentWnd->GetActiveFrame()->GetActiveView());
	pDoc = pView->GetDocument();
	// TODO: 在此添加控件通知处理程序代码
	pDoc->RotateImage(&m_center, &m_offset, m_rotationAngle, m_interpMethod.GetCurSel(), FALSE);
}
