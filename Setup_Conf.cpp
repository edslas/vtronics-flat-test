
#include <UTILITY.h>
#include <ANSI_C.h>
#include <cvirte.h>		/* Needed if linking in external compiler; harmless otherwise */
#include <userint.h>
#include "main.h"
#include "globals.h"
#include "DataAquisition.h"
#include "StepperMotor.h"
#include "TroubleShoot.h"

extern int GetVDatFromFile(VNumberStruct *pVEntry,char *pPath);
extern int SetEditVData(VNumberStruct *pData);  

// Global Declarations:
int pMainScreen;
int pEditV;
int pConformityTest; 
int pTroubleShoot;

VNumberStruct CurrentVNumber;

int BuildVNumberList(void);
int SetVData(VNumberStruct *pData);

int main (int argc, char *argv[])
{
int ierror;
double Reading; 


	if (InitCVIRTE (0, argv, 0) == 0)	/* Needed if linking in external compiler; harmless otherwise */
		return -1;	/* out of memory */
	if ((pMainScreen = LoadPanel (0, "main.uir", SETUP)) < 0)
		return -1;
	if ((pEditV = LoadPanel (0, "main.uir", EDITV)) < 0)
		return -1;
	if ((pConformityTest = LoadPanel (0, "main.uir", TEST)) < 0)
		return -1;
	if ((pTroubleShoot = LoadPanel (0, "main.uir", TBLSHOOT)) < 0)
		return -1;
	
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
	
	// Create Directory just to be sure
	ierror = MakeDir (V_FILE_PATH);
	
	if(IMS_Open(1) != 0)
	{
		MessagePopup ("Fatal Error", "Could not open COM1"); 
		return -1;		
	}
	
	IMS_SendCommand(1, "MUNIT=51200/360", NULL);
	GetDefaults();
	
	BuildVNumberList();
	DisplayPanel (pMainScreen);
	RunUserInterface ();
	return 0;
}

int CVICALLBACK EditV (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			if(strlen(CurrentVNumber.pVNumber))
			{
				SetEditVData(&CurrentVNumber);		 
			}
			HidePanel (pMainScreen);
			DisplayPanel (pEditV);
			
			break;
		}
	return 0;
}

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
			HidePanel (pMainScreen);
			DisplayPanel (pTroubleShoot);
			break;
		}
	return 0;
}
int CVICALLBACK StartTesting (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int ierror;
	switch (event)
		{
		case EVENT_COMMIT:
			// Fill in current V Number Data
			
			ierror = SetCtrlVal (pConformityTest, TEST_VNUMBER, CurrentVNumber.pVNumber);	
			HidePanel (pMainScreen);
			DisplayPanel (pConformityTest);
			break;
		}
	return 0;
}
int BuildVNumberList(void)
{
	char pPath[MAX_FILENAME_LEN];
	char pFileName[MAX_FILENAME_LEN];
	char pDirectory[MAX_FILENAME_LEN];
	int result;
	int i = 0;
	VNumberStruct *pVEntry;	
	int ierror = 0;
	int Count;
	unsigned int uiData;
	
	//TODO
	// Kill old list
	//
	i = 0;
	ierror = GetNumListItems (pMainScreen, SETUP_VNUMBER, &Count);
	while(ierror == 0 && i < Count)
	{
		ierror = GetValueFromIndex (pMainScreen, SETUP_VNUMBER, i, &uiData);
		if(ierror == 0)
		{
			free ((void *)(uiData));
		}
		i++;
	}
	ierror = ClearListCtrl (pMainScreen, SETUP_VNUMBER);
	
	strcpy(pDirectory, V_FILE_PATH);
	
	
	
	strcat(pDirectory, "\\*.*");
	result = GetFirstFile (pDirectory, 1, 0, 0, 0, 0, 0, pFileName);
	i = 0;
	while(result == 0)
	{
		// Load file into control:
		// Malloc memory for entry
		pVEntry = (VNumberStruct *)malloc(sizeof(VNumberStruct));
		// Decode entry from file
		// build full path
		strcpy(pPath, V_FILE_PATH);
		strcat(pPath, pFileName); 
		if(GetVDatFromFile(pVEntry,pPath) == 0)
		{
			// add to control
			ierror = InsertListItem (pMainScreen, SETUP_VNUMBER, i, pVEntry->pVNumber, (unsigned int)pVEntry);		
			if(ierror == 0)
			{
				if(i == 0)
				{
					CurrentVNumber = *pVEntry; 
					SetVData(pVEntry);
					ierror = SetCtrlIndex (pMainScreen, SETUP_VNUMBER, 0);
					
				}
				i++;
				result = GetNextFile (pFileName); 
			}
			else
			{
				result = -1;	
			}
		}
		else
		{
		     result = GetNextFile (pFileName);
		}
	}
	
	return ierror;
}


int CVICALLBACK VSelection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int i, ierror;
unsigned int uiData;
VNumberStruct VData;

	switch (event)
		{
		case EVENT_VAL_CHANGED:
			ierror = GetCtrlIndex (pMainScreen, SETUP_VNUMBER, &i);
			if(ierror == 0)
			{
				ierror = GetValueFromIndex (pMainScreen, SETUP_VNUMBER, i, &uiData);
				
				if(ierror == 0)
				{
					CurrentVNumber = *(VNumberStruct *)uiData;
					SetVData((VNumberStruct *)uiData);
				}
			}
			break;
		}
	return 0;
}

int SetVData(VNumberStruct *pData)
{
int ierror;

	ierror = SetCtrlVal (pMainScreen, SETUP_DIAMETER, pData->dDiameter);
	ierror += SetCtrlVal (pMainScreen, SETUP_ANGLE, pData->iAngle);
	ierror += SetCtrlVal (pMainScreen, SETUP_CONFORMITY, pData->dConformity);
	ierror += SetCtrlVal (pMainScreen, SETUP_RESISTANCE, pData->iResistance);
	ierror += SetCtrlVal (pMainScreen, SETUP_TOLERENCE, pData->dTolerence);
	ierror += SetCtrlVal (pMainScreen, SETUP_WINDING, pData->iWinding);
	ierror += SetCtrlVal (pMainScreen, SETUP_STACKS, pData->iStacks);
	ierror = ResetTextBox (pMainScreen, SETUP_COMMENT, pData->pComment);
	
	return ierror;
}
