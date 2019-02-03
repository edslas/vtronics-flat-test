#include <ANSI_C.h>
#include <userint.h>
#include "main.h"
#include "DataAquisition.h"
#include "Globals.h"
#include "StepperMotor.h"

#define DEFAULTS_FILE_NAME "Defaults.bin"          
extern int SetGrippers(int Open);

extern int pMainScreen;
extern int pTroubleShoot;
extern double gdHomePosition; 

int SetSpeeds(void);
int CVICALLBACK TroubleShootTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int ierror;
double Reading;
	if (GetActivePanel() != panel)
	{
		// don't bother if not on screen.
		return 0;
	}
	switch (event)
		{
		case EVENT_TIMER_TICK:
			// Read Analog Channels and Display 
			ierror = ReadChannel(SUPP_VOLTAGE_CH, &Reading);
			if(ierror == 0)
			{
			   SetCtrlVal (panel, TBLSHOOT_SUPVOLTAGE, Reading);
			}
			else
			{
				SetCtrlVal (panel, TBLSHOOT_SUPVOLTAGE, -99.999);
			}
			ierror = ReadChannel(FLAT_CH, &Reading);
			if(ierror == 0)
			{
			   SetCtrlVal (panel, TBLSHOOT_CONFORMITY, Reading);
			}
			else
			{
				SetCtrlVal (panel, TBLSHOOT_CONFORMITY, -99.999);
			}
			// Read Stepper Motor Speed
			ierror = IMS_GetPosition(2, &Reading); 
			if(ierror == 0)
			{
			   SetCtrlVal (panel, TBLSHOOT_POSITION, Reading);
			}
			else
			{
				SetCtrlVal (panel, TBLSHOOT_POSITION, -99.999);
			}
			break;
		}
	return 0;
}

int CVICALLBACK ExitTroubleShoot (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			HidePanel (pTroubleShoot);
			DisplayPanel (pMainScreen);
			 
			break;
		}
	return 0;
}

int CVICALLBACK StartTroubleShootMove (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
double dDistance;
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, TBLSHOOT_MOTOR_DEG, &dDistance); 
			SetSpeeds();
			IMS_RelativeMove(2, dDistance);
			break;
		}
	return 0;
}
int SetSpeeds(void)
{
int ierror;
char pCommand[64];

   	
   	sprintf(pCommand, "vi=%0.3lf",1.0);
   	ierror = IMS_SendCommand(2,pCommand, NULL);  
   	
   	
   	sprintf(pCommand, "vm=%0.3lf",2.0);
   	ierror += IMS_SendCommand(2,pCommand, NULL);
   	
   
   	sprintf(pCommand, "accl=%0.3lf",50);
   	ierror += IMS_SendCommand(2,pCommand, NULL);
   	
   	return ierror;
}
int GetDefaults(void)
{
int ierror;
	ierror = RecallPanelState (pTroubleShoot, DEFAULTS_FILE_NAME, 0);
	if(!ierror)
	{
		GetCtrlVal (pTroubleShoot, TBLSHOOT_HOME_POS, &gdHomePosition); 
	}
	
	return ierror;
}

int SaveSpeeds(void)
{
int ierror;
	ierror = SavePanelState (pTroubleShoot, DEFAULTS_FILE_NAME, 0);
	return ierror;
}
int CVICALLBACK SaveDefaultSpeeds (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			if(SaveSpeeds() == 0)
			{
				MessagePopup ("Confirmation", "Data was Saved");
			}
			else
			{
				MessagePopup ("Error", "Could not save data");
			}
			break;
		}
	return 0;
}

int CVICALLBACK ToggleGripper (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
static bool bOpen;
	switch (event)
		{
		case EVENT_COMMIT:
			if(bOpen)
			{
				SetGrippers(false);
				bOpen = false;
			}
			else
			{
				SetGrippers(true);
				bOpen = true;
			}
			break;
		}
	return 0;
}