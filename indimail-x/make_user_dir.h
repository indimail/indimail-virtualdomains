/*
 * $Log: make_user_dir.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * make_user_dir.h
 *
 */
#ifndef MAKE_USER_DIR
#define MAKE_USER_DIR

char           *make_user_dir(const char *, const char *, uid_t, gid_t, int);

#endif
