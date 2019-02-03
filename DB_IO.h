// DB_IO.h: interface for the CDB_IO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DB_IO_H__815D1A20_155A_11D5_B177_00E029467F2C__INCLUDED_)
#define AFX_DB_IO_H__815D1A20_155A_11D5_B177_00E029467F2C__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "globals.h"
class CDB_IO  
{
public:
	CDB_IO();
	virtual ~CDB_IO();

	int WriteFlatData(ResultDataStruct *RD);


private:
	CDatabase m_DB;
	BOOL m_bSuccess;

};

#endif // !defined(AFX_DB_IO_H__815D1A20_155A_11D5_B177_00E029467F2C__INCLUDED_)
