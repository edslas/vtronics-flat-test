/*************************************************************************/
/*                                                                       */
/* File Name:           Conformity.CPP                                     */
/* Name:				Voltronics, Potentiometer Tester                 */
/* Project Number:		1000                                             */
/*                                                                       */
/* Date:                March 8, 2001                                  */
/*                                                                       */
/* Operating System:    WIN2000                                           */
/* Compiler:            Visual C++ 6.0                                   */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Main module for the HMI                                             */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/* REVISION HISTORY                                                      */
/*                                                                       */
/* mm/dd/yyyy                                                            */
/*************************************************************************/

#include <afxdb.h>
#include "globals.h"
#include "DB_IO.h"

#include <UTILITY.h>
#include <ANSI_C.h>
//#include <memory.h>
#include <cvirte.h>		/* Needed if linking in external compiler; harmless otherwise */
#include <userint.h>
#include "main.h"
#include "globals.h"
#include "DataAquisition.h"
#include "StepperMotor.h"
#include "TroubleShoot.h"
#include "Conformity_Test.h"

#define MAIN_UIR "C:\\Voltronics\\Flat Test\\Program\\main.uir"


// Global Declarations:
int pMainScreen;
int pTroubleShoot;





int CVICALLBACK Exit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			IMS_Close(-1);
			exit(0);
			break;
		}
	return 0;
}

int CVICALLBACK TroubleShoot (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			//HidePanel (pMainScreen);
			DisplayPanel (pTroubleShoot);
			break;
		}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//		Main Function
//
int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
int ierror;
double Reading; 
char command[128];


	if (InitCVIRTE(hInstance,0,0)==0)	/* Needed if linking in external compiler; harmless otherwise */
		return -1;	/* out of memory */
	if ((pMainScreen = LoadPanel (0, MAIN_UIR, SETUP)) < 0)
		return -1;

	// pMainScreen Callbacks
	//InstallCtrlCallback(pMainScreen,SETUP_EXIT,Exit,NULL);
	
	InstallCtrlCallback(pMainScreen,SETUP_TROUBLESHOOT,TroubleShoot,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_START,StartTest,NULL);

	// TroubleShoot Callbacks
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_TIMER,TroubleShootTimer,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_START_MOVE,StartTroubleShootMove,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_SAVE_DEFAULT,SaveDefaultSpeeds,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_CANCEL,ExitTroubleShoot,NULL);
	
	


	if(InitializeDAQ() != 0)
	{
		MessagePopup ("Fatal Error", "Could not initialize DAQ"); 
		return -1;
	}
	if(SetUpExternalTrigger() != 0)
	{
		MessagePopup ("Fatal Error", "Could not set trigger"); 
		return -1;
	}
	
	if(ReadChannel(SUPP_VOLTAGE_CH, &Reading) != 0)
	{
		MessagePopup ("Fatal Error", "Could not Read Analog Input from DAQ"); 
		return -1;	
	}

	

	
	if(IMS_Open(2) != 0)
	{
		MessagePopup ("Fatal Error", "Could not open COM2. Restart Computer"); 
		return -1;		
	}

	sprintf(command, "MUNIT=%d", MUNIT_VALUE);
	IMS_SendCommand(2, command);

	GetDefaults();
	
	DisplayPanel (pMainScreen);
	RunUserInterface ();
	return 0;
}


