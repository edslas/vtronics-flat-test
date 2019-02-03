#include <userint.h>
#include "main.h"
#include <RS232.h>
#include <ANSI_C.h>
#define MAX_PORT_NUMBER 4
#define MAX_RESPONSE_SIZE 128
#define TERMINATION_CHAR 13


extern int pTroubleShoot; // this panel handle

int PortsOpen[MAX_PORT_NUMBER] = {0,0,0,0};
int IMS_SendCommand(int iCommPort, char *pCommand, char *pResponse);
int IMS_Open(int iCommPort)
{
int ierror;

	if(iCommPort < 1 || iCommPort > MAX_PORT_NUMBER)
	{
		return -1;
	}
	
 	ierror = OpenComConfig (iCommPort, "", 9600, 0, 8, 1, 512, 512);
 	if(ierror == 0)
 	{
 		PortsOpen[iCommPort - 1] = 1;
 		
 	}
 	return ierror;
}
int IMS_Close(int iCommPort)
{
int ierror, i;
	ierror = 0;
	if(iCommPort == -1)
	{
		// Close all open
		for(i = 1; i < MAX_PORT_NUMBER; i++)
		{
			if(PortsOpen[i-1])
			{
				ierror += CloseCom (i);
			}
		}
	}
	else
	{
		// Close specific
		if(iCommPort < 1 || iCommPort > MAX_PORT_NUMBER) return -1;
		// Verify port is open 
		if(PortsOpen[iCommPort - 1] == 0) return -2;
		ierror = CloseCom (iCommPort);
	}
	return ierror;
}
int IMS_RelativeMove(int iCommPort, double dDegrees)
{
int ierror;
char pCommand[64];
//char  pResponse[MAX_RESPONSE_SIZE];
	sprintf(pCommand, "movr %0.3lf", dDegrees);
	ierror =  IMS_SendCommand(iCommPort, pCommand, NULL);

return ierror;
}
int IMS_AbsoulteMove(int iCommPort, double dUnitsToMove)
{
int ierror;
char pCommand[64];
	sprintf(pCommand, "mova %0.3lf", dUnitsToMove);
	ierror =  IMS_SendCommand(iCommPort, pCommand, NULL);

return ierror;
}

int IMS_GetVelocity(int iCommPort, double *dpVelocity)
{
int ierror;
char  pResponse[MAX_RESPONSE_SIZE];
	ierror =  IMS_SendCommand(iCommPort, "print vel", pResponse);
	if(ierror == 0)
	{
		sscanf(pResponse, "%lf", dpVelocity);
	}
	
return ierror;
}
int IMS_GetPosition(int iCommPort, double *dpVelocity)
{
int ierror;
char  pResponse[MAX_RESPONSE_SIZE];
	ierror =  IMS_SendCommand(iCommPort, "print pos", pResponse);
	if(ierror == 0)
	{
		sscanf(pResponse, "%lf", dpVelocity);
	}
	
return ierror;
}

int IMS_SendCommand(int iCommPort, char *pCommand, char *pResponse)
{
int ierror;

int iCountToSend, iNumberRead, iWritten;
int iResponseSize = MAX_RESPONSE_SIZE;
char *pEOL;
char pEcho[MAX_RESPONSE_SIZE];

	// Verify Valid Port	
	if(iCommPort < 1 || iCommPort > MAX_PORT_NUMBER) return -1;
	// Verify port is open 
	if(PortsOpen[iCommPort - 1] == 0) return -2;

	
	// Flush receive buffer
	ierror = FlushInQ (iCommPort);
	
	iCountToSend = strlen(pCommand);
	iWritten = ComWrt (iCommPort, pCommand, iCountToSend);
	if(iWritten != iCountToSend) return -3;
	// Write terminitor
	iWritten = ComWrt (iCommPort, "\r\n", 2); 
	if(iWritten != 2) return -4;
	// read back echoed command
	iNumberRead = 0;
	iNumberRead = ComRdTerm (iCommPort, pEcho, iResponseSize,'\n');	
	
	if(pResponse)
	{

		//read response
		iNumberRead = 0;
		iNumberRead = ComRdTerm (iCommPort, pResponse, iResponseSize,'\n');
						 
		if(iNumberRead)
		{
			// We got something back
			pResponse[iNumberRead + 1] = 0; // Null terminate it to be safe
			pEOL = strchr(pResponse, '\r');
		   	if(pEOL)
		   	{
		   		*pEOL = 0; // truncates it
	   		
		   	}
	   	
		   	return 0;
	  	
		}
		else
			return -5;
	}
	else
	{
		return 0;
	}
}
extern int IMS_STOP(int iCommPort)
{
	return IMS_SendCommand(2,"SSTP 1",NULL);
}




