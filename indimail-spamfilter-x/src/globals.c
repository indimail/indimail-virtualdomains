/*****************************************************************************

NAME:
   globals.c -- define global variables

AUTHOR:
   Matthias Andree <matthias.andree@gmx.de>

******************************************************************************/

#include "system.h"
#include "globals.h"
#include "score.h"
#include "xstrdup.h"

/* exports */

bool 	fBogofilter = false;
bool 	fBogotune   = false;
bool 	fBogoutil   = false;

/* command line options */

bulk_t	bulk_mode = B_NORMAL;		/* '-b, -B' */
bool	suppress_config_file;		/* '-C' */
bool	nonspam_exits_zero;		/* '-e' */
FILE	*fpin = NULL;			/* '-I' */
bool	logflag;			/* '-l' */
bool	mbox_mode;			/* '-M' */
bool	replace_nonascii_characters;	/* '-n' */
bool	passthrough;			/* '-p' */
bool	pairs = false;			/* '-P' */
bool	quiet;				/* '-q' */
int	query;				/* '-Q' */
bool	Rtable;				/* '-R' */
bool	terse;				/* '-t' */
int	bogotest;			/* '-X', env("BOGOTEST") */
int	verbose;			/* '-v' */

/* config file options */
double	min_dev;
double	ham_cutoff = HAM_CUTOFF;
double	spam_cutoff;
double	thresh_update;

uint	min_token_len       = MIN_TOKEN_LEN;
uint	max_token_len       = MAX_TOKEN_LEN;
uint	max_multi_token_len = 0;
uint	multi_token_count   = MUL_TOKEN_CNT;

uint	token_count_fix = 0;
uint	token_count_min = 0;
uint	token_count_max = 0;

const char	*update_dir;
/*@observer@*/
const char	*stats_prefix;

char *spam_header_name;
char *spam_header_place;

char *charset_default;
char *charset_unicode;

/* for lexer_v3.l */
bool	header_line_markup = true;	/* -H */

/* for  transactions */
#ifndef	ENABLE_TRANSACTIONS
e_txn	eTransaction = T_DEFAULT_OFF;
#else
e_txn	eTransaction = T_DEFAULT_ON;
#endif

/* for  encodings */
e_enc	encoding = E_UNKNOWN;

/* for  bogoconfig.c, prob.c, rstats.c and score.c */
double	robx = 0.0;
double	robs = 0.0;
double	sp_esf = SP_ESF;
double	ns_esf = NS_ESF;

/* other */
FILE	*fpo;
uint	db_cachesize = DB_CACHESIZE;	/* in MB */
bool	msg_count_file = false;
char	*progtype = NULL;
bool	unsure_stats = false;		/* true if print stats for unsures */

run_t run_type = RUN_UNKNOWN;

uint	wordlist_version;

char *header_format;
char *terse_format;
char *log_header_format;
char *log_update_format;

static int globals_initialized;

#define zxfree(a) do { xfree((a)); (a) = 0; } while(0)

static void free_globals(void) {
    if (globals_initialized) {
	zxfree(spam_header_place);
	zxfree(spam_header_name);
	zxfree(charset_default);
	zxfree(charset_unicode);
	globals_initialized = -1;
    }
}

void init_globals(void) {
    if (globals_initialized == 1) return;
    spam_header_name = xstrdup(SPAM_HEADER_NAME);
    spam_header_place = xstrdup("");
    charset_default = xstrdup(DEFAULT_CHARSET);
    charset_unicode = xstrdup("UTF-8");
    header_format = xstrdup("%h: %c, tests=bogofilter, spamicity=%p, version=%v");
    terse_format = xstrdup("%1.1c %f");
    log_header_format = xstrdup("%h: %c, spamicity=%p, version=%v");
    log_update_format = xstrdup("register-%r, %w words, %m messages");

    if (!globals_initialized)
	atexit(free_globals);
    globals_initialized = 1;
}
