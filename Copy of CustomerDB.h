#if !defined(AFX_CUSTOMERDB_H__AE12146E_3E8C_4ADB_97BC_086DD0749C10__INCLUDED_)
#define AFX_CUSTOMERDB_H__AE12146E_3E8C_4ADB_97BC_086DD0749C10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomerDB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomerDB recordset

class CCustomerDB : public CRecordset
{
public:
	CCustomerDB(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CCustomerDB)

// Field/Param Data
	//{{AFX_FIELD(CCustomerDB, CRecordset)
	long	m_ID;
	CString	m_Customer_Name;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomerDB)
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

#endif // !defined(AFX_CUSTOMERDB_H__AE12146E_3E8C_4ADB_97BC_086DD0749C10__INCLUDED_)
