#include "alarmtimer.h"
#include "alarmsleep.h"
#include <unistd.h>
#include <iostream>
#include "config.h"

extern "C" {

	void rfc2045_error(const char *p)
	{
		fprintf(stderr, "%s\n", p);
		fflush(stderr);
		exit(1);
	}
}

#if SYSLOG_LOGGING
int do_syslog = 1;
#endif

int main()
{
	alarm(30);

	{
		AlarmTimer timer;

		timer.Set(20);

		AlarmSleep(1);

		if (timer.Expired())
		{
			std::cerr << "Timer shouldn't expire\n";
			exit(1);
		}
	}

	AlarmTimer timer;

	timer.Set(1);

	while (!timer.Expired())
	{
		AlarmSleep(1);
	}

	return 0;
}
