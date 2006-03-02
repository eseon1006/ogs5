
//------------------------------------------------------------------------------
// 06/2004 CC Implementation Edit control for editing the subItem of list control
//-------------------------------------------------------------------------------

#if !defined(AFX_EDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_)
#define AFX_EDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditCell.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyEditCell window
class CMyEditCell : public CEdit
{
public:
			CMyEditCell (CMyListCtrl* pCtrl, int iItem, int iSubItem, CString sInitText);
    virtual ~CMyEditCell();
    void    SetListText();

    //{{AFX_VIRTUAL(CMyEditCell)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
    CMyListCtrl* pListCtrl;
    int			Item;
    int			SubItem;
    CString		InitText;
    BOOL		bEscape;
    
    //{{AFX_MSG(CMyEditCell)
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_)
