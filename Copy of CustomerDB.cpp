// CustomerDB.cpp : implementation file
//

//#include "stdafx.h"
//#include "hmi.h"
#include <afxdb.h>
#include "CustomerDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomerDB

IMPLEMENT_DYNAMIC(CCustomerDB, CRecordset)

CCustomerDB::CCustomerDB(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CCustomerDB)
	m_ID = 0;
	m_Customer_Name = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CCustomerDB::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CCustomerDB::GetDefaultSQL()
{
	return _T("[Customers]");
}

void CCustomerDB::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CCustomerDB)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ID]"), m_ID);
	RFX_Text(pFX, _T("[Customer Name]"), m_Customer_Name);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CCustomerDB diagnostics

#ifdef _DEBUG
void CCustomerDB::AssertValid() const
{
	CRecordset::AssertValid();
}

void CCustomerDB::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
