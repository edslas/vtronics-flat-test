// FlatTestDBData.cpp : implementation file
//

//#include "stdafx.h"
#include "FlatTestDBData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlatTestDBData

IMPLEMENT_DYNAMIC(CFlatTestDBData, CRecordset)

CFlatTestDBData::CFlatTestDBData(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CFlatTestDBData)
	m_ID = 0;
	m_Conformity = 0.0;
	m_Resistance = 0.0;
	m_Batch = _T("");
	m_Type = _T("");
	m_Operator = _T("");
	m_nFields = 7;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CFlatTestDBData::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CFlatTestDBData::GetDefaultSQL()
{
	return _T("[Flat Test Data]");
}

void CFlatTestDBData::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CFlatTestDBData)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ID]"), m_ID);
	RFX_Double(pFX, _T("[Conformity]"), m_Conformity);
	RFX_Double(pFX, _T("[Resistance]"), m_Resistance);
	RFX_Date(pFX, _T("[Date]"), m_Date);
	RFX_Text(pFX, _T("[Batch]"), m_Batch);
	RFX_Text(pFX, _T("[Type]"), m_Type);
	RFX_Text(pFX, _T("[Operator]"), m_Operator);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFlatTestDBData diagnostics

#ifdef _DEBUG
void CFlatTestDBData::AssertValid() const
{
	CRecordset::AssertValid();
}

void CFlatTestDBData::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
