int BestLinearFit(
					double			*Data,
					unsigned short	Points,
					double			*Slope,
					double			*Offset,
					double			*MSE);
void StartTimer(long *Timer, double TimeOut);
bool TimerExpired(long Timer);
