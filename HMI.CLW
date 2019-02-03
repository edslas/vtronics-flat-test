; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CFlatTestDBData
LastTemplate=CRecordset
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "hmi.h"
LastPage=0

ClassCount=1

ResourceCount=0
Class1=CFlatTestDBData

[DB:CConformityData]
DB=1
DBType=ODBC
ColumnCount=9
Column1=[ID], 4, 4
Column2=[PO Number], 12, 64
Column3=[Customer], 12, 64
Column4=[V Number], 12, 64
Column5=[Serial Number], 12, 64
Column6=[Test Date], 11, 16
Column7=[Conformity], 8, 8
Column8=[Operator], 12, 6
Column9=[Resistance], 8, 8

[DB:CVNumbersDB]
DB=1
DBType=ODBC
ColumnCount=14
Column1=[ID], 4, 4
Column2=[VNumber], 12, 100
Column3=[Diameter], 8, 8
Column4=[Angle], 4, 4
Column5=[Stack Count], 4, 4
Column6=[Max Conformity], 8, 8
Column7=[Resistance], 4, 4
Column8=[Tolerence], 4, 4
Column9=[Winding], 4, 4
Column10=[Spilt], -7, 1
Column11=[Band 1], 4, 4
Column12=[Dead], 4, 4
Column13=[Band 2], 4, 4
Column14=[Comment], 12, 100

[DB:CCustomerDB]
DB=1
DBType=ODBC
ColumnCount=2
Column1=[ID], 4, 4
Column2=[Customer Name], 12, 100

[CLS:CFlatTestDBData]
Type=0
HeaderFile=FlatTestDBData.h
ImplementationFile=FlatTestDBData.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CFlatTestDBData

[DB:CFlatTestDBData]
DB=1
DBType=ODBC
ColumnCount=7
Column1=[ID], 4, 4
Column2=[Conformity], 8, 8
Column3=[Resistance], 8, 8
Column4=[Date], 11, 16
Column5=[Batch], 12, 64
Column6=[Type], 12, 64
Column7=[Operator], 12, 6

