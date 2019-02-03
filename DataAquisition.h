
extern int InitializeDAQ(void);
extern int ReadChannel(int Channel, double *Reading);
extern int SetUpExternalTrigger(void);
extern int ReadFromTrigger(double *dBuffer, int iNumPoints);
