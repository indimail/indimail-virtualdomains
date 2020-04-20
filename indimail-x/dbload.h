/*
 * $Log: dbload.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dbload.h
 *
 */
#ifndef DBLOAD_H
#define DBLOAD_H
#include "indimail.h"
#include <mysql.h>

int             connect_db(DBINFO **, MYSQL **);
int             OpenDatabases();
void            close_db();

#endif
