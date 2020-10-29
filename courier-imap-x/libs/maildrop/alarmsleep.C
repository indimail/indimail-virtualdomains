#include	"config.h"
#include	<signal.h>
#include	"alarmsleep.h"


AlarmSleep::AlarmSleep(unsigned nseconds) : flag(0)
{
	sigset_t new_mask;
	sigemptyset(&new_mask);
	Set(nseconds);
	do
	{
		sigsuspend(&new_mask);
	} while (!flag);
	Cancel();
}

AlarmSleep::~AlarmSleep()
{
}

void AlarmSleep::handler()
{
	flag=1;
	Set(5);	// A possibility of a race condition - try again.
}
