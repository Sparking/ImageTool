
// ImageToolView.cpp : CImageToolView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Image Tool.h"
#endif

#include "Image ToolDoc.h"
#include "Image ToolView.h"
#include "DialogTransToolRotation.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageToolView

IMPLEMENT_DYNCREATE(CImageToolView, CScrollView)

BEGIN_MESSAGE_MAP(CImageToolView, CScrollView)
	ON_COMMAND(ID_TOOL_TRANSFORM_ROTATE, &CImageToolView::RotateImage)
	ON_COMMAND(ID_QR_FIND_PM, &CImageToolView::QRFindPositionMarkings)
	ON_COMMAND(ID_SHOW_RFEDGES, &CImageToolView::ShowEdges)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

// CImageToolView 构造/析构

CImageToolView::CImageToolView()
{
	// TODO: 在此处添加构造代码

}

CImageToolView::~CImageToolView()
{
}

BOOL CImageToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CScrollView::PreCreateWindow(cs);
}

// CImageToolView 绘制

void CImageToolView::OnDraw(CDC* pDC)
{
	CImageToolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	CDC dcMem;
	BITMAP bm;
	CBitmap *pBitmap = pDoc->GetBitmap();

	dcMem.CreateCompatibleDC(pDC);
	pBitmap->GetBitmap(&bm);
	pBitmap = dcMem.SelectObject(pBitmap);
	SetScrollSizes(MM_TEXT, CSize(bm.bmWidth, bm.bmHeight));
	pDC->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pBitmap);
}

void CImageToolView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}

void CImageToolView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CImageToolView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CImageToolView 诊断

#ifdef _DEBUG
void CImageToolView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CImageToolView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CImageToolDoc* CImageToolView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImageToolDoc)));
	return (CImageToolDoc*)m_pDocument;
}
#endif //_DEBUG

void CImageToolView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CImageToolView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CImageToolView::RotateImage(void)
{
	((CMainFrame *)(AfxGetApp()->m_pMainWnd))->m_wndTransToolRotation.ShowWindow(SW_SHOW);
}

void CImageToolView::QRFindPositionMarkings(void)
{
	GetDocument()->MarkQRPM();
}

void CImageToolView::ShowEdges(void)
{
	GetDocument()->MarkEdges();
}
