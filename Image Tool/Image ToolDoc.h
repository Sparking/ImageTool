
// Image ToolDoc.h : CImageToolDoc 类的接口
//


#pragma once

#include "image.h"
#include <stack>

class CImageToolDoc : public CDocument
{
protected: // 仅从序列化创建
	CImageToolDoc();
	DECLARE_DYNCREATE(CImageToolDoc)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CImageToolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
private:
	image *m_srcimg, *m_img;
	CBitmap m_bitmap;	/* m_img对应的bitmap数据, 每次更新m_img后都要重新设置该成员的内容 */

	/* 撤销栈、重做栈*/
	std::stack<image *> m_stack_undo, m_stack_redo;

	void ClearRedoStack();
	void ClearUndoStack();
	void UpdateBitmap();
public:
	CBitmap *GetBitmap();
	image *GetSrcImage() const;
	image *GetImage() const;
	CSize GetDocSize() const;
	void MarkQRPM();
	void MarkEdges();
	void EdgesTest();

	/* 操作接口 */
	void RotateImage(const point *rotation_center, const point *rotation_offset,
		const float theta, const int interp, const BOOL save);	/* 旋转 */
	afx_msg void OnEditUndo();	/* 撤销 */
	afx_msg void OnEditRedo();  /* 重做 */
};
