#if !defined(AFX_DB_GLOBALS_H__815D1A20_155A_11D5_B177_00E029467F2C__INCLUDED_)
#define AFX_DB_GLOBALS_H__815D1A20_155A_11D5_B177_00E029467F2C__INCLUDED_

#define DAQ_SUPORT			1
#define SUPP_VOLTAGE_CH 	0
#define FLAT_CH 			3

#define STATUS_IN_IDLE		0
#define STATUS_IN_PROGRESS	1
#define STATUS_IN_PASS		2
#define STATUS_IN_FAIL		3

#define PULLY_DIAMETER_IN	((double)(35.01/25.4))
#define IN_PER_REV			((double)(3.1415926535897932384626433832795*PULLY_DIAMETER_IN))
#define MUNIT_VALUE			((int)((200*256)/IN_PER_REV))
#define ENCODER_PULSES_PER_REV ((double)(1000.0))
#define ENCODER_PULSES_PER_IN ((double)(ENCODER_PULSES_PER_REV/IN_PER_REV))

//#define MAX_NUMBER_POINTS				((int)(ENCODER_PULSES_PER_IN*6))
#define MAX_NUMBER_POINTS				2048
#define CONFORMITY_OVERSHOOT			0.10 // inches

typedef struct {

	double	dResistance;
	double	dConformity;
	char	strOperator[4];
	char	strBatch[32];
	char	strType[32];
}ResultDataStruct;
typedef struct {

	double	dClearPosition;
	double	dLoadPosition;
	double	dTestPosition;
	double	dFailPosition;
	double	dPassPosition;
	char	strProgramName[64];
	double	dResistance;
	double	dResistanceTolerencePercent;

	double dInitialSpeed;
	double dFinalSpeed;
	double dAcceleration;

	double dTestLength;
	double dMaxConformity;

}ProgramDataStruct;

#endif