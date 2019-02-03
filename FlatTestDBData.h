#if !defined(AFX_FLATTESTDBDATA_H__1F1D31C0_902E_11D5_B178_00E029467F2C__INCLUDED_)
#define AFX_FLATTESTDBDATA_H__1F1D31C0_902E_11D5_B178_00E029467F2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlatTestDBData.h : header file
//
#include <afxdb.h>

/////////////////////////////////////////////////////////////////////////////
// CFlatTestDBData recordset

class CFlatTestDBData : public CRecordset
{
public:
	CFlatTestDBData(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CFlatTestDBData)

// Field/Param Data
	//{{AFX_FIELD(CFlatTestDBData, CRecordset)
	long	m_ID;
	double	m_Conformity;
	double	m_Resistance;
	CTime	m_Date;
	CString	m_Batch;
	CString	m_Type;
	CString	m_Operator;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlatTestDBData)
	public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLATTESTDBDATA_H__1F1D31C0_902E_11D5_B178_00E029467F2C__INCLUDED_)
