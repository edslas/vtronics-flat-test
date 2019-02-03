/*************************************************************************/
/*                                                                       */
/* File Name:           Main.CPP                                     */
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
#include "Flat_Test.h"
#include "Utility.h"
#include "windows.h"


#define MAIN_UIR "C:\\Voltronics\\Flat Test\\Program\\main.uir"
#define GRIPPER_TIME 0.5
#define OPEN 1
#define CLOSE 0

//extern int GetVDatFromFile(VNumberStruct *pVEntry,char *pPath);
int CVICALLBACK EditV (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);
DWORD BuildProgramList();


// Global Declarations:
int pMainScreen;
int pTroubleShoot;
double gdHomePosition = 0.0; 

enum Errors{	
	eNoError,
	eCommError
	};

enum States{
	eNeedsHome,			//0	
	eReady,				//1
	eError,				//2
	eHoming,			//3
	eGoingToClear,		//4
	eGoingToLoad,		//5
	eClosingGripper,	//6
	eGointToTest,		//7
	eTesting,			//8
	eGoingToPass,		//9
	eOpenGrippers,		//10
	eScraping,			//11
	eUnScraping			//11
	}gState;


ProgramDataStruct *gpCurrentProgram;

ResultDataStruct gCurrentResults;

int giDefaultProgramIndex = 0;

DWORD LoadProgram(char *FileName, ProgramDataStruct *Data);

int		giAutomatic = 0;
bool	gbtemp = true;
long	glGripperTimer;
long    glPassCnt;
long    glFailCnt;
int StartHomeRobot();
int MotionStopped();
void UpdateState(States NewState);
int SetGrippers(int Open);

int CVICALLBACK Exit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			IMS_STOP(2);
			IMS_Close(-1);
			QuitUserInterface(1);
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

int CVICALLBACK StartTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_COMMIT) return 0;
	if(gState == eReady)
	{
		// execute Test
		// Initialize Part Data
		gCurrentResults.dConformity = -1.0;
		gCurrentResults.dResistance = -1.0;

		
		SetStatusLights(STATUS_IN_PROGRESS);

		GetCtrlVal (pMainScreen, SETUP_OPERATOR, gCurrentResults.strOperator); 
		GetCtrlVal (pMainScreen, SETUP_PO, gCurrentResults.strBatch); 
		strcpy(gCurrentResults.strType, gpCurrentProgram->strProgramName);

		char Buffer[64];
		sprintf(Buffer, "vi=%0.3lf", gpCurrentProgram->dInitialSpeed);
		IMS_SendCommand(2, Buffer, NULL); 
		sprintf(Buffer, "vm=%0.3lf", gpCurrentProgram->dFinalSpeed);
		IMS_SendCommand(2, Buffer, NULL); 
		sprintf(Buffer, "accl=%0.3lf", gpCurrentProgram->dAcceleration);
		IMS_SendCommand(2, Buffer, NULL); 

		IMS_AbsoulteMove(2,gpCurrentProgram->dLoadPosition);
		UpdateState(eGoingToLoad);
	}
	else
	{
		AfxMessageBox("Not Ready - Could not Start");
	}
	return 0;
}
int CVICALLBACK Initialize (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_COMMIT) return 0;
	IMS_STOP(2);
	StartHomeRobot();
	return 0;
}
int CVICALLBACK StopMotion (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_COMMIT) return 0;
	IMS_STOP(2);
	UpdateState(eNeedsHome);
	return 0;
}
int CVICALLBACK UpdateAutomatic (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_COMMIT) return 0;
	GetCtrlVal (pMainScreen, SETUP_AUTOMATIC, &giAutomatic); 
	return 0;
}

int CVICALLBACK ResetCounters (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_COMMIT) return 0;
	glPassCnt = 0;
	glFailCnt = 0;

	SetCtrlVal (pMainScreen, SETUP_PASSEDCNT, (long)0);
	SetCtrlVal (pMainScreen, SETUP_FAILEDCNT, (long)0);
	return 0;
}
int CVICALLBACK ChangeProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int Index = 0;
long Temp;
int ierror;
	if(event != EVENT_COMMIT) return 0;

	ierror = GetCtrlIndex( pMainScreen, SETUP_PROGRAM, &Index);
	ierror = GetValueFromIndex (pMainScreen, SETUP_PROGRAM, Index, &Temp);	
	if(!ierror)
	{
		gpCurrentProgram = ((ProgramDataStruct *)Temp);
	}
	glFailCnt = 0;
	glPassCnt = 0;

	return 0;
}

int CVICALLBACK StateMachine(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event != EVENT_TIMER_TICK) return 0;

	switch (gState)
	{
	default:
		break;
	case eHoming:
		// wait fo motion to stop
		if(MotionStopped() == 1)
		{
			// set 0 position
			
			char Buffer[64];
			sprintf(Buffer, "pos=%0.3lf", gdHomePosition);
			IMS_SendCommand(2, Buffer, NULL); 

			// Change to high speed
			//Set up speeds
		
			sprintf(Buffer, "vi=%0.3lf", gpCurrentProgram->dInitialSpeed);
			IMS_SendCommand(2, Buffer, NULL); 
			sprintf(Buffer, "vm=%0.3lf", gpCurrentProgram->dFinalSpeed);
			IMS_SendCommand(2, Buffer, NULL); 
			sprintf(Buffer, "accl=%0.3lf", gpCurrentProgram->dAcceleration);
			IMS_SendCommand(2, Buffer, NULL); 
			// start motion to clear
			IMS_AbsoulteMove(2, gpCurrentProgram->dClearPosition);
			UpdateState(eGoingToClear);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}

			
		break;
	case eGoingToClear:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			UpdateState(eReady);

		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;
	case eGoingToLoad:
		// wait fo motion to stop
		if(MotionStopped() == 1)
		{
			// Close Gripper
			SetGrippers(CLOSE);
			// Set Timer
			StartTimer(&glGripperTimer, GRIPPER_TIME);
			UpdateState(eClosingGripper);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;
	case eClosingGripper:
		// wait for gripper to close
		if(TimerExpired(glGripperTimer))
		{
			//Move to test
			IMS_AbsoulteMove(2, gpCurrentProgram->dTestPosition);
			UpdateState(eGointToTest);
		}	
		break;
	case eScraping:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			// move to Fail
			IMS_RelativeMove(2, gpCurrentProgram->dTestLength);
			UpdateState(eUnScraping);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;
	case eUnScraping:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			// move to Fail
			IMS_AbsoulteMove(2, gpCurrentProgram->dTestPosition);
			UpdateState(eGointToTest);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;

	case eGointToTest:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			Sleep(100);
			// move to Fail
			IMS_AbsoulteMove(2, gpCurrentProgram->dFailPosition);

			gCurrentResults.dConformity = 99.99;
			// Arm Data Aquisition
			StartTest2(0,0,0,NULL,0,0);

			UpdateState(eTesting);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;
	case eTesting:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			// Determine Pass Fail
		
			if(gCurrentResults.dConformity <= gpCurrentProgram->dMaxConformity ) 
			{
				gbtemp = false;
				// pass
				glPassCnt++;
				SetCtrlVal (pMainScreen, SETUP_PASSEDCNT, glPassCnt);

				IMS_AbsoulteMove(2, gpCurrentProgram->dPassPosition);
				UpdateState(eGoingToPass);
			}
			else
			{
				gbtemp = true;
				// fail
				glFailCnt++;
				SetCtrlVal (pMainScreen, SETUP_FAILEDCNT, glFailCnt);

				// Open Grippers
				SetGrippers(OPEN);
				StartTimer(&glGripperTimer, GRIPPER_TIME);
				UpdateState(eOpenGrippers);
			}
			
			
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;
	case eGoingToPass:
		// wait for motion to stop
		if(MotionStopped() == 1)
		{
			// open grippers
			SetGrippers(OPEN);
			StartTimer(&glGripperTimer, GRIPPER_TIME);
			UpdateState(eOpenGrippers);
		}
		else if(MotionStopped() < 0)
		{
			UpdateState(eError);
		}	
		break;

	case eOpenGrippers:
		// wait for grippers open
		if(TimerExpired(glGripperTimer))
		{
			if(giAutomatic)
			{
				UpdateState(eReady);
				StartTest (0, 0, EVENT_COMMIT, NULL, 0,0);
			}
			else
			{
				UpdateState(eReady);
			}
		}
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

double Reading;
	if (InitCVIRTE(hInstance,0,0)==0)	/* Needed if linking in external compiler; harmless otherwise */
		return -1;	/* out of memory */
	if ((pMainScreen = LoadPanel (0, MAIN_UIR, SETUP)) < 0)
		return -1;
	if ((pTroubleShoot = LoadPanel (0, MAIN_UIR, TBLSHOOT)) < 0)
		return -1;
	// pMainScreen Callbacks
	InstallCtrlCallback(pMainScreen,SETUP_EXIT,Exit,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_TROUBLESHOOT,TroubleShoot,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_START,StartTest,NULL);

	InstallCtrlCallback(pMainScreen,SETUP_TIMER,StateMachine,NULL);

	InstallCtrlCallback(pMainScreen,SETUP_AUTOMATIC,UpdateAutomatic,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_INITIALIZE,Initialize,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_STOP,StopMotion,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_RESETCNT,ResetCounters,NULL);
	InstallCtrlCallback(pMainScreen,SETUP_PROGRAM,ChangeProgram,NULL);
	

	// TroubleShoot Callbacks
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_TIMER,TroubleShootTimer,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_START_MOVE,StartTroubleShootMove,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_SAVE_DEFAULT,SaveDefaultSpeeds,NULL);
	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_CANCEL,ExitTroubleShoot,NULL);

	InstallCtrlCallback(pTroubleShoot,TBLSHOOT_TOGGLE_GRIP,ToggleGripper,NULL);
	
	

	ResetCounters (0, 0, EVENT_COMMIT,NULL,0,0);

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
	
	if(BuildProgramList() <= 0)
	{
		MessagePopup ("Fatal Error", "Cound not load any ini programs."); 
		return -1;	
	}

	// initialize default program
	SetCtrlIndex( pMainScreen, SETUP_PROGRAM, giDefaultProgramIndex);
	ChangeProgram( 0,0,EVENT_COMMIT, NULL, 0, 0);
	memset(&gCurrentResults, 0, sizeof(ResultDataStruct));
	GetDefaults(); 
	DisplayPanel (pMainScreen);
	RunUserInterface ();

	return 0;
}

int StartHomeRobot()
{
char command[64];

	// Set up IMS Motor Controller
	sprintf(command, "MUNIT=%d", MUNIT_VALUE); // define units as inches
	IMS_SendCommand(2, command, NULL);

	// Set up I/O
	
	// Set up limits / home
	IMS_SendCommand(2, "IOS 21=14,0,1", NULL); // Limit -
	IMS_SendCommand(2, "IOS 22=12,0,0", NULL); // Home
	IMS_SendCommand(2, "IOS 23=13,0,1", NULL); // Limit +

	// Gripper Outpuy
	IMS_SendCommand(2, "IOS 26=0,1,0", NULL); // SSR

	SetGrippers(OPEN);
// todo up in motion input

	// Motor Current = 100%
	IMS_SendCommand(2, "MRC=100", NULL); // Limit +
	
	//Set up speeds
	IMS_SendCommand(2, "vi=0.5", NULL); 
	IMS_SendCommand(2, "vm=1.5", NULL); 
	IMS_SendCommand(2, "accl=10", NULL); 

	// Start Home process
	IMS_SendCommand(2, "fios", NULL); 
	UpdateState(eHoming);
	return 0;
}


int MotionStopped()
{
double dSpeed;

	if(IMS_GetVelocity(2,&dSpeed) == 0)
	{
		if(fabs(dSpeed) <= DBL_EPSILON)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -eCommError;
	}
}
void UpdateState(States NewState)
{
	gState = NewState;
	SetCtrlVal (pMainScreen, SETUP_STATE, NewState);
}
int SetGrippers(int Open)
{
	if(Open)
	{
		return IMS_SendCommand(2, "IO 26 = 0", NULL);
	}
	else
	{
		return IMS_SendCommand(2, "IO 26 = 1", NULL);
	}
}
DWORD LoadProgram(char *FileName, ProgramDataStruct *Data)
{
DWORD BytesRead;
char Buffer[64];
char FullPath[MAX_PATH];

double ClearPosition = 15;
double LoadPosition = 0;
double TestPosition = 0;
double FailPosition = 0;
double PassPosition = 0;
double Resistance = 0;
double Tolerence = 20;
double InitialSpeed = 1.0;
double FinalSpeed = 6.0;
double Acceleration = 100;
double TestLength = 5.0;
double MaxConformity = 3.0;

	GetCurrentDirectory(MAX_PATH, FullPath);
	if(strlen(FullPath) < 1) return -190;

	if(FullPath[strlen(FullPath) -1] != '\\')
	{
		strcat(FullPath, "\\");
	}
	strcat(FullPath, FileName);

	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "ClearPosition",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &ClearPosition);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "LoadPosition",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &LoadPosition);
	}
	else
	{
		return -1;
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "TestPosition",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &TestPosition);
	}
	else
	{
		return -1;
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "FailPosition",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &FailPosition);
	}
	else
	{
		return -1;
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "PassPosition",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &PassPosition);
	}
	else
	{
		return -1;
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "Resistance",	//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &Resistance);
	}
	else
	{
		return -1;
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "Tolerence",		//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &Tolerence);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "ISpeed",			//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &InitialSpeed);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "FSpeed",			//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &FinalSpeed);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "Acceleration",			//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &Acceleration);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "TestLength",		//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &TestLength);
	}
	//---------------------------------------
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "MaxConformity",		//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead > 0)
	{
		sscanf(Buffer, "%lf", &MaxConformity);
	}


	//---------------------------------------
	//
	// This must be last for name to work!!!
	///
	BytesRead =  GetPrivateProfileString(
			  "Program",		//LPCTSTR lpAppName,        // section name
			  "Name",			//LPCTSTR lpKeyName,        // key name
			  NULL,				//LPCTSTR lpDefault,        // default string
			  Buffer,			//LPTSTR lpReturnedString,  // destination buffer
			  64,				//DWORD nSize,              // size of destination buffer
			  FullPath			//LPCTSTR lpFileName        // initialization file name
			);
	if(BytesRead <= 0)
	{
		return -1;
	}


	Data->dClearPosition = ClearPosition;
	Data->dLoadPosition = LoadPosition;
	Data->dTestPosition = TestPosition;
	Data->dFailPosition = FailPosition;
	Data->dPassPosition = PassPosition;
	Data->dResistance = Resistance;
	Data->dTestLength = TestLength;
	Data->dMaxConformity = MaxConformity;

	Data->dResistanceTolerencePercent = Tolerence;
	Data->dInitialSpeed = InitialSpeed;
	Data->dFinalSpeed = FinalSpeed;
	Data->dAcceleration = Acceleration;

	strcpy(Data->strProgramName, Buffer);

	return 0;
}

DWORD BuildProgramList()
{

WIN32_FIND_DATA FindFileData;
HANDLE hFind;
BOOL FileFound;
ProgramDataStruct *ProgramData;
int ierror,i;

	hFind = FindFirstFile("*.ini", &FindFileData);
	i = 0;
	if(hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			ProgramData = (ProgramDataStruct *)malloc(sizeof(ProgramDataStruct));
			ierror = LoadProgram(FindFileData.cFileName, ProgramData);
			if(ierror == 0)
			{
				// add to control
				ierror = InsertListItem (pMainScreen, SETUP_PROGRAM, i,ProgramData->strProgramName, (unsigned int)ProgramData);		
				if(ierror == 0)
				{
					strupr(ProgramData->strProgramName);
					if(strcmp(ProgramData->strProgramName, "DEFAULT.INI") == 0) // check for default
					{
						giDefaultProgramIndex = i;
					}
					i++;
				}
			}

				FileFound = FindNextFile(hFind,  &FindFileData);
		}while(FileFound); 
	}

  if(hFind != INVALID_HANDLE_VALUE)
  {
	  FindClose(hFind);
  }

  return (i);
}