/*
 * $Log: getEnvConfig.c,v $
 * Revision 1.1  2019-04-14 18:51:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <env.h>
#include <scan.h>

#ifndef	lint
static char     sccsid[] = "$Id: getEnvConfig.c,v 1.1 2019-04-14 18:51:08+05:30 Cprogrammer Exp mbhangui $";
#endif

/*- getEnvConfigStr */
void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = env_get(envname)))
		*source = defaultValue;
	return;
}

void
getEnvConfigInt(int *source, char *envname, int defaultValue)
{
	char           *value;

	if (!(value = env_get(envname)))
		*source = defaultValue;
	else
		scan_int(value, source);
	return;
}

void
getEnvConfigLong(long *source, char *envname, long defaultValue)
{
	char           *value;

	if (!(value = env_get(envname)))
		*source = defaultValue;
	else
		scan_long(value, source);
	return;
}

void
getEnvConfiguLong(unsigned long *source, char *envname, unsigned long defaultValue)
{
	char           *value;

	if (!(value = env_get(envname)))
		*source = defaultValue;
	else
		scan_ulong(value, source);
	return;
}
