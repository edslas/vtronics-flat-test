#include <math.h>
#include <float.h>
#include <time.h>

int BestLinearFit(
					double			*Data,
					unsigned short	Points,
					double			*Slope,
					double			*Offset,
					double			*MSE)
{
// Data is a 2-D array of data
int ierror = 0;
unsigned short i;
double m, b;
double SumX, SumXX, SumY, SumYY, SumXY;
double SSE, SST, LLR, DEM;

	if (Points)
	{
		SumX  = 0.0;
		SumXX = 0.0;
		SumY  = 0.0;
		SumYY = 0.0;
		SumXY = 0.0;
		for (i=0;i<Points;i++)
		{
			//SumX  += Data[i*2+0];
			//SumXX += (Data[i*2+0] * Data[i*2+0]);
			//SumY  += Data[i*2+1];
			//SumYY += (Data[i*2+1] * Data[i*2+1]);
			//SumXY += (Data[i*2+0] * Data[i*2+1]);
			
			SumX  += i;
			SumXX += i * i;
			SumY  += Data[i];
			SumYY += Data[i] * Data[i];
			SumXY += Data[i] * i;
		}

		DEM = ((double)(Points) * SumXX) - (SumX * SumX);

		if (fabs(DEM) > DBL_EPSILON)
		{
			m =    (((double)(Points) * SumXY) - (SumX * SumY)) / DEM;

			b = (SumY - (m * SumX)) / (double) Points;

			SSE = SumYY - (b * SumY) - (m * SumXY);

			SST = SumYY - ((SumY * SumY) / (double) Points);

			if(fabs(SST) > DBL_EPSILON)
			{
				LLR = 1.0 - (SSE / SST);
				if (Slope)
					*Slope = m;
				if (Offset)
					*Offset = b;
				if (MSE)
					*MSE = LLR;
			}
			else
			{
				ierror = -1;
			}
		}
		else
		{
			ierror = -2;
		}
	}
	else
	{
		ierror = -3;
	}
	return(ierror);
}//BestLinearFit()

void StartTimer(long *Timer, double TimeOut)
{
	*Timer = clock() + long((TimeOut * CLOCKS_PER_SEC));
		
}
bool TimerExpired(long Timer)
{
	if(clock() > Timer)
	{
		return true;
	}
	else
	{
		return false;
	}
}