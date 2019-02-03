#include <afxdb.h>
#include "Globals.h"
#include "DB_IO.h"
#include <windows.h>

//#include <UTILITY.h>
#include <ANSI_C.h>
#include <userint.h>
#include "main.h"
#include "Utility.h"
#include "steppermotor.h"
#include "DataAquisition.h"
#include "Troubleshoot.h"
#include "Flat_Test.h"

extern int pMainScreen;
extern int pConformityTest;
extern ProgramDataStruct *gpCurrentProgram;
extern long    glPassCnt;
extern long    glFailCnt;
extern ResultDataStruct gCurrentResults;



#define AVE_WINDOW	1

int PlotData(double *xData, double *yData, double *lyData, double *DeviationData, int iNumberOfPoints, int iOffset);
int CalculateDeviation(double *xData, double *yData, double *lyData, double *DeviationData, double *dMax, double *dMaxIndex, int iNumberOfPoints, int iOffset);
int WriteResultsToDB(double dMax);
void UpdateResultList(char *pBuffer);
void IncrementSerialNumber(char *pSerialNumber);
int FindFlats(double *yData, int *iOffset, int *iLength);
int FindDeadBand(double *yData, int *iOffset, int *iLength);
int CheckUserData(void);
int SmoothData(double *Data, int Length, int Window);
int CheckReady(void);
int CheckOutputRange(double *Data, int iNumberOfPoints);

int CVICALLBACK StartTest2 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
char pBuffer[128];
int ierror;
double xData[MAX_NUMBER_POINTS]; 
double yData[MAX_NUMBER_POINTS];
double lyData[MAX_NUMBER_POINTS];
double DeviationData[MAX_NUMBER_POINTS];
double dMax;
double dMaxIndex;
int iNumberOfPoints;
int iOffset, iLength;
double dtemp;

	

	memset(yData, 0, sizeof(yData));
	dtemp = gpCurrentProgram->dTestLength *  (ENCODER_PULSES_PER_REV / IN_PER_REV);
	
	//
	iNumberOfPoints = (int)(dtemp); 
	if(iNumberOfPoints > MAX_NUMBER_POINTS) iNumberOfPoints = MAX_NUMBER_POINTS;

	ierror = ReadFromTrigger(yData , iNumberOfPoints);

	if(ierror)
	{
		SetStatusLights(STATUS_IN_FAIL);
		sprintf(pBuffer, "%d Data Error", ierror);
		UpdateResultList(pBuffer);
		return 0;
	}
	
	if(CheckOutputRange(yData, iNumberOfPoints) != 0) 
	{
		SetStatusLights(STATUS_IN_FAIL);
		UpdateResultList("Output Range Fail");
		return 0;
	}

	iLength = iNumberOfPoints;

	FindFlats(yData, &iOffset, &iLength);
	
	iNumberOfPoints = iLength;
	if(iNumberOfPoints == 0)
	{
		SetStatusLights(STATUS_IN_FAIL);
		sprintf(pBuffer, "Not enough data");
		UpdateResultList(pBuffer);
		return 0;
	}

	ierror = CalculateDeviation(
				xData, 
				yData, 
				lyData, 
				DeviationData, 
				&dMax, 
				&dMaxIndex, 
				iNumberOfPoints, 
				iOffset);	
	if(ierror)
	{
		SetStatusLights(STATUS_IN_FAIL);
		sprintf(pBuffer, "%d Bad Data Error", ierror);
		UpdateResultList(pBuffer);
		return 0;
	}
	
	PlotData(xData, yData, lyData, DeviationData, iNumberOfPoints, iOffset);
		
	sprintf(pBuffer, "%d\033p050l%1.2lf",glPassCnt+glFailCnt+1, dMax);
	UpdateResultList(pBuffer);

	// Write to DataBase
	CDB_IO DB;
	gCurrentResults.dConformity = dMax;
	DB.WriteFlatData(&gCurrentResults);
	if(gCurrentResults.dConformity <= gpCurrentProgram->dMaxConformity)
	{
		SetStatusLights(STATUS_IN_PASS);
	}
	else
	{
		SetStatusLights(STATUS_IN_FAIL);
	}

	return 0;
}

int CVICALLBACK StopTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//int ierror;
	switch (event)
		{
		case EVENT_COMMIT:
			// Clear out results + Serial Number
			//ierror = ClearListCtrl (pMainScreen, SETUP_RESULTS);
			//ierror = SetCtrlVal (pMainScreen, SETUP_SERIALENTRY, "");		

			break;
		}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// PlotData
//
//	Draws both graphs given
///////////////////////////////////////////////////////////////////////////////////////////////
int PlotData(double *xData, double *yData, double *lyData, double *DeviationData, int iNumberOfPoints, int iOffset)
{
int ierror;
double xDataShifted[MAX_NUMBER_POINTS]; 
int i;
		
	// Shift X Data for display Purposes
	for(i = iOffset; i < iNumberOfPoints + iOffset; i++)
	{
		xDataShifted[i] = xData[i] - xData[iOffset];
	}
	// ouput ratio 
	// Clear old graph
	ierror = DeleteGraphPlot (pMainScreen, SETUP_OUTPUTRATIO, -1, VAL_IMMEDIATE_DRAW);
	// Draw real data
	PlotXY (pMainScreen, SETUP_OUTPUTRATIO, &xDataShifted[iOffset], &yData[iOffset], iNumberOfPoints,
						 VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
						 VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
	// draw linear fit
	PlotXY (pMainScreen, SETUP_OUTPUTRATIO, &xDataShifted[iOffset], &lyData[iOffset], iNumberOfPoints,
						 VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
						 VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_BLACK);		
				
	// Deviation 
	// Delete old
	ierror += DeleteGraphPlot (pMainScreen, SETUP_DEVIATION, -1, VAL_IMMEDIATE_DRAW);
	// draw dots
	PlotXY (pMainScreen, SETUP_DEVIATION, &xDataShifted[iOffset], &DeviationData[iOffset],
								  iNumberOfPoints, VAL_DOUBLE,
								  VAL_DOUBLE, VAL_SCATTER, VAL_SOLID_CIRCLE,
								  VAL_SOLID, 1, VAL_BLACK);
	return ierror;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// CalculateDeviation
//
//	Calculates best linear fit
//	Fills in DeviationData = abs(yData - lyData)
//  Calculates Max deviation and the index it happens at.
///////////////////////////////////////////////////////////////////////////////////////////////
int CalculateDeviation(double *xData, double *yData, double *lyData, double *DeviationData, double *dMax, double *dMaxIndex, int iNumberOfPoints, int iOffset)
{
int ierror,i;
double dSlope, dOffset, dMSE;
	//SmoothData(&yData[iOffset], iNumberOfPoints, AVE_WINDOW);
	ierror = BestLinearFit( &yData[iOffset], iNumberOfPoints, &dSlope, &dOffset, &dMSE); 
	if(ierror)
	{
		return ierror;
	}
	
	for (i = iOffset ; i < iNumberOfPoints + iOffset; i++)
	{
		xData[i] = ((double)i) / ENCODER_PULSES_PER_IN; // translate to degrees
		lyData[i] = (dSlope*(i-iOffset)) + dOffset;
		DeviationData[i] =  lyData[i] -  yData[i];
	}
	SmoothData(&DeviationData[iOffset], iNumberOfPoints, AVE_WINDOW);
	
	for (i = iOffset ; i < iNumberOfPoints + iOffset; i++)
	{
		if(i == iOffset)
		{
			*dMax = fabs(DeviationData[i]);
			*dMaxIndex = xData[i] - xData[iOffset];
		}
		else
		{
			if (fabs(DeviationData[i]) > *dMax)
			{
				*dMax = fabs(DeviationData[i]);
				*dMaxIndex = xData[i] - xData[iOffset];
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// WriteResultsToDB
//
//	Uses global data and result to make data base entry
///////////////////////////////////////////////////////////////////////////////////////////////
int WriteResultsToDB(double dMax)
{
int ierror = 0;
	return ierror;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// UpdateResultList
//
//	Builds string to add to result list
///////////////////////////////////////////////////////////////////////////////////////////////
void UpdateResultList(char *pBuffer)
{
int ierror;

int iCount;
	// Make sure list is not too long
	GetNumListItems(pMainScreen, SETUP_RESULTS, &iCount);
	if(iCount > 50)
	{
		DeleteListItem(pMainScreen, SETUP_RESULTS, 0, 10);
	}
	// Add to list				
	ierror = InsertListItem (pMainScreen, SETUP_RESULTS, -1,
			 pBuffer, NULL);
	ierror = GetNumListItems (pMainScreen, SETUP_RESULTS, &iCount);              
	// Set current selection to last added
	ierror = SetCtrlIndex (pMainScreen, SETUP_RESULTS, iCount -1);
}
///////////////////////////////////////////////////////////////////////////////////////////////
// IncrementSerialNumber
//
//	Auto increment serial number if single stack
///////////////////////////////////////////////////////////////////////////////////////////////
void IncrementSerialNumber(char *pSerialNumber)
{

}
int FindFlats(double *yData, int *iOffset, int *iLength)
{
int i;
double dAverageDifference = 0.0;
double dDifference;
int start = 0;
int end = 0;
int Length;
int AngleInPoints;
int	WindowDeficit, WindowSurplus;
bool bCanAdjustStart = true;
bool bCanAdjustEnd = true;
	AngleInPoints = (int)(gpCurrentProgram->dTestLength * ENCODER_PULSES_PER_IN);

	//AngleInPoints = (int)(AngleInPoints * 0.9);
	// calculate average difference between points.
	for(i = 0; i < *iLength - 1; i++)
	{
		dAverageDifference += fabs(yData[i] - yData[i+1]);
	}
	dAverageDifference = dAverageDifference / (double)i;
	
	// See when average Difference starts
	for(i = 1; (i < *iLength - 2) && (start == 0); i++)
	{
		dDifference = fabs(yData[i] - yData[i+1]);
		if(dDifference > (dAverageDifference/1.25))
		{
			if(dDifference < (dAverageDifference*3.0))
			start = i;
		}
	}
	end = *iLength - 1;
	for(i = *iLength - 2; ((i > 0) && (end == *iLength - 1)); i--)
	{
		dDifference = fabs(yData[i] - yData[i-1]);
		if(dDifference > (dAverageDifference/2.0))
		{
			end = i;
		}
	}

	Length = end - start;
	// Open window to spec IF NEEDED.
	WindowDeficit = AngleInPoints - Length;
	while((WindowDeficit < 0) && bCanAdjustStart && bCanAdjustEnd)
	{
		
		if(start > 0)
		{
			start--;
			bCanAdjustStart = true;
		}
		else
		{
			bCanAdjustStart = false;
		}
		Length = end - start;
		WindowDeficit = AngleInPoints - Length;
		if((end < *iLength) && (WindowDeficit < 0))
		{
			end++;
			bCanAdjustEnd = true;
		}
		else
		{
			bCanAdjustEnd = false;
		}
		Length = end - start;
		WindowDeficit = AngleInPoints - Length;
	}
	// CLOSE window to spec IF NEEDED.
	Length = end - start;
	WindowSurplus = Length - AngleInPoints;

	while(WindowSurplus > 0 )
	{
		start+=2;
		
		Length = end - start;
		WindowSurplus = Length - AngleInPoints;
		if(WindowSurplus > 0)
		{
			end--;
			start++;
			Length = end - start;
			WindowSurplus = Length - AngleInPoints;
		}
	}
	*iOffset = start;
	*iLength = end - start;
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// CheckUserData
//
//	Verifies that user input is valid. 
///////////////////////////////////////////////////////////////////////////////////////////////
int CheckUserData(void)
{
char pSerialNumber[32];

	//GetCtrlVal (pMainScreen, SETUP_SERIALENTRY, pSerialNumber);
	if(strlen(pSerialNumber) == 0)
	{
		MessagePopup ("Error", "A Serial Number is Required");
		return -1;
	}
	
return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// CheckUserData
//
//	Verifies that user input is valid. 
///////////////////////////////////////////////////////////////////////////////////////////////
int SetStatusLights(int iStatus)
{
	switch(iStatus)
	{
		case  STATUS_IN_IDLE:
			SetCtrlVal (pMainScreen, SETUP_INPROGRESS, 0);
			SetCtrlVal (pMainScreen, SETUP_PASS, 0);
			SetCtrlVal (pMainScreen, SETUP_FAIL, 0);       
			break;
		case  STATUS_IN_PROGRESS:
			SetCtrlVal (pMainScreen, SETUP_INPROGRESS, 1);
			SetCtrlVal (pMainScreen, SETUP_PASS, 0);
			SetCtrlVal (pMainScreen, SETUP_FAIL, 0);  
			break;
		case  STATUS_IN_PASS:
			SetCtrlVal (pMainScreen, SETUP_INPROGRESS, 0);
			SetCtrlVal (pMainScreen, SETUP_PASS, 1);
			SetCtrlVal (pMainScreen, SETUP_FAIL, 0);  
			break;
		case  STATUS_IN_FAIL:
			SetCtrlVal (pMainScreen, SETUP_INPROGRESS, 0);
			SetCtrlVal (pMainScreen, SETUP_PASS, 0);
			SetCtrlVal (pMainScreen, SETUP_FAIL, 1);  
			for(int i = 0; i < 20; i++)
			{
				::Beep(10000,1); 
				::Beep(5000,1);
			}

			break;
	}
	return 0;
}
int SmoothData(double *Data, int Length, int Window)
{
int i, j, count;
double sum;
double *Temp;
	if(Window <= 0) return -1;
	if(Length <= 0) return -2;
	Temp = (double *)malloc(Length*sizeof(double));
	if(!Temp) return -3;

	for(i = 0; i < 	Length; i++)
	{
		sum = 0.0;
		count = 0;
		for(j = i - Window ; j < i + Window; j++)
		{
			if((j < Length) && (j >= 0))
			{
				sum += Data[j];
				count++;
			}
		}
		if(count > 0) // beware of divide by 0
		{
			Temp[i] = sum / (double)count; // compute average + assign
		}
		else
		{
			Temp[i] = Data[i];
		}
	}
	memcpy(Data, Temp, Length*sizeof(double));
	free(Temp);
	return 0;
}
int CheckReady(void)
{
double SupplyVoltage;

	ReadChannel(SUPP_VOLTAGE_CH, &SupplyVoltage);
	if(SupplyVoltage < 4.5)
	{
		AfxMessageBox("Test Supply Voltage less than 4.5 V."
						"Test not started!\n"
						"Check for short on Red + Black leads\n"
						"Also try shorting Red + Black leads for 1 second to reset power supply.", MB_OK,0);
		return -1;
	}
	else
	{
		return 0;
	}
}
int CheckOutputRange(double *Data, int iNumberOfPoints)
{
int i;
bool bMinFound = false,bMaxFound = false;

	// make sure output goes from at least 20 - 80 %
	for( i = 0; (i < iNumberOfPoints) && !(bMinFound && bMaxFound); i++)
	{
		if(Data[i] < 20) bMinFound = true;
		if(Data[i] > 80) bMaxFound = true;
	}
	if(bMinFound && bMaxFound)
	{
		return 0;
	}
	else
	{
		/*
		AfxMessageBox("Test Failed. Output Ratio deviation too small."
						"Check for short on Red/Black with Green lead\n"
						"Check for mechanical linkage with part\n"
						"Check Stepper motor operation / Supply in Trouble Shooting", MB_OK,0);
		*/
		return -1;
	}

}