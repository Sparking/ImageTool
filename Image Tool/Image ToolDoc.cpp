
// Image ToolDoc.cpp : CImageToolDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Image Tool.h"
#endif

#include "MainFrm.h"
#include "Image ToolDoc.h"
#include "qr_position.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int(*image_interp_func[4])(const struct image *, unsigned char *, const float, const float) = {
	image_nearest_interp,
	image_bilinear_interp,
	image_bicubic_interp,
	image_bicubic_interp
};
// CImageToolDoc

IMPLEMENT_DYNCREATE(CImageToolDoc, CDocument)

BEGIN_MESSAGE_MAP(CImageToolDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, &CImageToolDoc::OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CImageToolDoc::OnEditRedo)
END_MESSAGE_MAP()


// CImageToolDoc 构造/析构

CImageToolDoc::CImageToolDoc() : m_srcimg(nullptr), m_img(nullptr)
{
	// TODO: 在此添加一次性构造代码

}

CImageToolDoc::~CImageToolDoc()
{
	ClearUndoStack();
	ClearRedoStack();
}

BOOL CImageToolDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	m_bitmap.CreateBitmap(0, 0, 1, 32, nullptr);

	return TRUE;
}




// CImageToolDoc 序列化

void CImageToolDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CImageToolDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CImageToolDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CImageToolDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CImageToolDoc 诊断

#ifdef _DEBUG
void CImageToolDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CImageToolDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CImageToolDoc 命令
BOOL CImageToolDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	image *srcimg;

	srcimg = image_open(CW2A(lpszPathName));
	if (srcimg == nullptr) {
		MessageBox(NULL, _T("打开图像文件失败"), 0, 0);
		return FALSE;
	}

	m_srcimg = image_convert_format(srcimg, IMAGE_FORMAT_BGRA);
	image_release(srcimg);
	if (m_srcimg == nullptr)
		return FALSE;

	if (m_bitmap.CreateBitmap(m_srcimg->width, m_srcimg->height, 1, 32, m_srcimg->data) != TRUE) {
		image_release(m_srcimg);
		m_srcimg = nullptr;
		return FALSE;
	}

	m_stack_undo.push(m_srcimg);
	m_img = m_srcimg;

	return TRUE;
}

BOOL CImageToolDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (m_srcimg == nullptr)
		return FALSE;

	if (image_save(CW2A(lpszPathName), m_srcimg, IMAGE_FILE_BITMAP) != 0)
		return FALSE;

	return TRUE;
}

void CImageToolDoc::OnCloseDocument()
{
	CDocument::OnCloseDocument();
}

image *CImageToolDoc::GetSrcImage() const
{
	return m_srcimg;
}

image *CImageToolDoc::GetImage() const
{
	return m_img;
}

CSize CImageToolDoc::GetDocSize() const
{
	if (m_img == nullptr)
		return CSize(0, 0);

	return CSize(m_img->width, m_img->height);
}

CBitmap *CImageToolDoc::GetBitmap()
{
	return &m_bitmap;
}

void CImageToolDoc::UpdateBitmap()
{
	m_bitmap.SetBitmapBits(m_img->size, m_img->data);
	UpdateAllViews(NULL);
}

void CImageToolDoc::MarkQRPM()
{
	unsigned int n, i;
	struct image *gray, *dump;
	struct qr_position_makrings_info pm[20];
	const unsigned char xxc[4] = { 0x00, 0x7E, 0xFF, 0xFF};

	dump = image_dump(m_img);
	if (dump == nullptr)
		return;

	gray = image_convert_gray(m_srcimg);
	if (gray == nullptr) {
		image_release(dump);
		return;
	}

	m_img = dump;
	n = qr_position_makrings_find(gray, pm, 20);
	image_release(gray);
	for (i = 0; i < n; ++i) {
		img_print_point(m_img, pm[i].center.x, pm[i].center.y, xxc, 3);
	}
	UpdateBitmap();
	m_stack_undo.push(m_img);
	ClearRedoStack();
}

void CImageToolDoc::MarkEdges()
{
	struct point start, xoff;
	unsigned int cnt, i, j;
	struct image *gray, *dump;
	const unsigned char xxc[2][4] = {
		{ 0xFE, 0x97, 0x00, 0xFF },
		{ 0x81, 0x10, 0xFF, 0xFF } };
	struct image_raise_fall_edge rfe[500];

	dump = image_dump(m_img);
	if (dump == nullptr)
		return;

	gray = image_convert_gray(m_srcimg);
	if (gray == nullptr) {
		image_release(dump);
		return;
	}

	m_img = dump;
	xoff.x = 1;
	xoff.y = 0;
	start.x = 0;
	for (j = 0; j < m_img->height; ++j) {
		start.y = j;
		cnt = image_find_raise_fall_edges_by_offset(gray, &start, &xoff, gray->width, rfe, 500);
		for (i = 0; i < cnt; ++i) {
			if (rfe[i].type == IMAGE_RFEDGE_TYPE_RAISE) {
				memcpy(m_img->data + j * m_img->row_size + rfe[i].dpos * m_img->pixel_size, xxc[0], m_img->pixel_size);
			} else {
				memcpy(m_img->data + j * m_img->row_size + rfe[i].dpos * m_img->pixel_size, xxc[1], m_img->pixel_size);
			}
		}
	}
	image_release(gray);
	UpdateBitmap();
	m_stack_undo.push(m_img);
	ClearRedoStack();
}

void CImageToolDoc::RotateImage(const point *rotation_center, const point *rotation_offset,
		const float theta, const int interp, const BOOL save)
{
	image *new_img;
	const float theta_rad = DEGREE2RAD(theta);

	if (m_srcimg == nullptr || interp < 0 || interp > 3)
		return;

	new_img = image_rotation(m_srcimg, rotation_center, rotation_offset, theta_rad, image_interp_func[interp]);
	if (new_img == nullptr) {
		MessageBox(NULL, _T("内存不足"), 0, 0);
		OnCloseDocument();
		return;
	}

	m_img = new_img;
	UpdateBitmap();
	if (save) {
		m_stack_undo.push(m_img);
		ClearRedoStack();
	} else {
		image_release(m_img);
		m_img = nullptr;
	}
}

static void CImageToolDocClearStack(std::stack<image *> &s)
{
	while (!s.empty()) {
		image_release(s.top());
		s.pop();
	}
}

void CImageToolDoc::ClearRedoStack()
{
	CImageToolDocClearStack(m_stack_redo);
}

void CImageToolDoc::ClearUndoStack()
{
	CImageToolDocClearStack(m_stack_undo);
}

void CImageToolDoc::OnEditUndo()
{
	image *tmp;

	if (m_stack_undo.size() <= 1)
		return;

	tmp = m_stack_undo.top();
	m_stack_redo.push(tmp);
	m_stack_undo.pop();

	m_img = m_stack_undo.top();
	UpdateBitmap();
}

void CImageToolDoc::OnEditRedo()
{
	if (m_stack_redo.empty())
		return;

	m_stack_undo.push(m_stack_redo.top());
	m_stack_redo.pop();
	m_img = m_stack_undo.top();
	UpdateBitmap();
}
