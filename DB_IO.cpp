// DB_IO.cpp: implementation of the CDB_IO class.
//
//////////////////////////////////////////////////////////////////////

#include <afxdb.h>
#include "Globals.h"


#include "DB_IO.h"
#include "FlatTestDBData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDB_IO::CDB_IO()
{
 

	m_bSuccess = TRUE;
	try 
	{
		m_bSuccess = m_DB.OpenEx("ODBC;DBQ=C:\\Voltronics\\Flat Test\\Data\\FlatData.mdb;DefaultDir=C:\\Voltronics\\Flat Test\\Data;Driver={Microsoft Access Driver (*.mdb)};DriverId=281;FIL=MS Access;FILEDSN=C:\\Voltronics\\Flat Test\\Data\\FlatData.dsn;MaxBufferSize=2048;MaxScanRows=8;PageTimeout=5;SafeTransactions=0;Threads=3;UID=admin;UserCommitSync=Yes;");
	}
	catch( CDBException eBD)
	{
		m_bSuccess = FALSE;
	}
	catch( CMemoryException eM )
	{
		m_bSuccess = FALSE;
	}
	catch(...)
	{
		m_bSuccess = FALSE;
	}
	if(!(m_DB.IsOpen()))
	{
		m_bSuccess = FALSE;
	}

}

CDB_IO::~CDB_IO()
{
	if(m_bSuccess) 
	{
		m_DB.Close();
	}
	m_bSuccess = false;

}

int CDB_IO::WriteFlatData(ResultDataStruct *RD)
{
CTime Time(time(NULL));

	if(m_bSuccess)
	{
		CFlatTestDBData *Data = new CFlatTestDBData(&m_DB);
		
		if(Data)
		{
			if(Data->Open(CRecordset::dynaset))
			{
				Data->AddNew();
					Data->m_Batch = RD->strBatch;

					Data->m_Conformity = RD->dConformity;

					//Data->m_Date = *(new CTime(time(NULL)));
					Data->m_Date = Time;

					Data->m_Operator = RD->strOperator;

					Data->m_Resistance = RD->dResistance;

					Data->m_Type = RD->strType;

				Data->Update();

			
			}
			else
			{
				AfxMessageBox("Can't open Part Data Data Base",  MB_ICONEXCLAMATION | MB_OK);
			}
			delete Data;
		}
	}
return (0);
}

