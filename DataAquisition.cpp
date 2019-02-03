#include <userint.h>
#include "main.h"
#include <dataacq.h>
#include "Globals.h"
#include <stdlib.h>
#include <string.h>
//--------------------------------------------------------------------------
//InitializeDAQ
//--------------------------------------------------------------------------
// Initializes the data aquisition board for this application
int SetUpExternalTrigger(void);

int InitializeDAQ(void)
{
int ierror = 0; 
short iBoardType;
int ireturnval = 0;

#if DAQ_SUPORT

	ierror = Init_DA_Brds (1, &iBoardType);
	if(ierror == 0)
	{
		if(iBoardType != 36)
		{
			ierror = 1;
		}
	}
	else
	{
		ireturnval = ierror;
	}
	
	// Configure channels for input type, range, etc.
		 
		ierror = AI_Configure (
							1 					// Board #
							,SUPP_VOLTAGE_CH	// Channel
							,0					// 0=differ, 1=RSE, 2 = NRSE 
							,5					// Input Range 5,10,20
							,1					// 0 = bipolar, 1 = Unipolar
							,0					// Drive AISense
							);
		
		// Configure channels for 
		ierror = AI_Configure (
							1 					// Board #
							,FLAT_CH			// Channel
							,0					// 0=differ, 1=RSE, 2 = NRSE 
							,5					// Input Range 5,10,20
							,1					// 0 = bipolar, 1 = Unipolar
							,0					// Drive AISense
							);
		

#endif	
	return ierror;
}
//--------------------------------------------------------------------------
//ReadChannel
//--------------------------------------------------------------------------
int ReadChannel(int Channel, double *Reading)
{
int ierror = 0;
	// Set cnvert back to auto see SetUpExternalTrigger()
#if DAQ_SUPORT
	ierror = Select_Signal (1, ND_IN_CONVERT, ND_INTERNAL_TIMER, ND_LOW_TO_HIGH);
	if(ierror == 0)
	{
		ierror = AI_VRead (1, Channel, 1, Reading);
	}
#else
	*Reading = 9.99;
#endif
	return ierror;
																	 
																	  
}
//--------------------------------------------------------------------------
//ReadChannel
//--------------------------------------------------------------------------
// Tells Data Aq board to samle analog channel from PFI1, the encoder
int SetUpExternalTrigger(void)
{
int ierror = 0;
#if DAQ_SUPORT
	ierror = Select_Signal (1, ND_IN_CONVERT, ND_PFI_1, ND_HIGH_TO_LOW);
#endif
	return ierror;
}
//--------------------------------------------------------------------------
//ReadChannel
//--------------------------------------------------------------------------
// Tells Data Aq board to samle analog channel from PFI0, the encoder
int ReadFromTrigger(double *dBuffer, int iNumPoints)
{
int ierror = 0;
short sBuffer[MAX_NUMBER_POINTS];
	if(iNumPoints > MAX_NUMBER_POINTS)
		return -1;
	memset(sBuffer, 0, MAX_NUMBER_POINTS*sizeof(short));
#if DAQ_SUPORT
	ierror = SetUpExternalTrigger();
	if(ierror == 0)
	{
		ierror = Timeout_Config (
				1,	//short Board, 
				10000	//long Timeout 55ms per count;
				);

		ierror = DAQ_Op (1, 3, 2, sBuffer, iNumPoints, 10000.0);
	}
	if(ierror == -10800) ierror = 0;
	
	if(ierror == 0)
	{
		for(int i = 0; i < iNumPoints; i++)
		{
			dBuffer[i] = ((double)sBuffer[i])*100.0 / 4096.0;
		}
	}
	else
	{
		InitializeDAQ();
	}
#endif
	return ierror;
}