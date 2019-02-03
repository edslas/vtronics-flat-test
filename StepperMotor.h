extern int IMS_Open(int iCommPort);
extern int IMS_Close(int iCommPort); 
extern int IMS_SendCommand(int iCommPort, char *pCommand, char *pResponse); 
extern int IMS_GetVelocity(int iCommPort, double *dpVelocity);
extern int IMS_GetPosition(int iCommPort, double *dpVelocity);
extern int IMS_RelativeMove(int iCommPort, double dDegrees);
extern int IMS_AbsoulteMove(int iCommPort, double dUnitsToMove);
extern int IMS_STOP(int iCommPort);


