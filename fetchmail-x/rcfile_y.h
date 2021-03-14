/* A Bison parser, made by GNU Bison 3.6.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_RCFILE_Y_H_INCLUDED
# define YY_YY_RCFILE_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    DEFAULTS = 258,                /* DEFAULTS  */
    POLL = 259,                    /* POLL  */
    SKIP = 260,                    /* SKIP  */
    VIA = 261,                     /* VIA  */
    AKA = 262,                     /* AKA  */
    LOCALDOMAINS = 263,            /* LOCALDOMAINS  */
    PROTOCOL = 264,                /* PROTOCOL  */
    AUTHENTICATE = 265,            /* AUTHENTICATE  */
    TIMEOUT = 266,                 /* TIMEOUT  */
    KPOP = 267,                    /* KPOP  */
    SDPS = 268,                    /* SDPS  */
    ENVELOPE = 269,                /* ENVELOPE  */
    QVIRTUAL = 270,                /* QVIRTUAL  */
    USERNAME = 271,                /* USERNAME  */
    PASSWORD = 272,                /* PASSWORD  */
    FOLDER = 273,                  /* FOLDER  */
    SMTPHOST = 274,                /* SMTPHOST  */
    FETCHDOMAINS = 275,            /* FETCHDOMAINS  */
    MDA = 276,                     /* MDA  */
    BSMTP = 277,                   /* BSMTP  */
    LMTP = 278,                    /* LMTP  */
    SMTPADDRESS = 279,             /* SMTPADDRESS  */
    SMTPNAME = 280,                /* SMTPNAME  */
    SPAMRESPONSE = 281,            /* SPAMRESPONSE  */
    PRECONNECT = 282,              /* PRECONNECT  */
    POSTCONNECT = 283,             /* POSTCONNECT  */
    LIMIT = 284,                   /* LIMIT  */
    WARNINGS = 285,                /* WARNINGS  */
    INTERFACE = 286,               /* INTERFACE  */
    MONITOR = 287,                 /* MONITOR  */
    PLUGIN = 288,                  /* PLUGIN  */
    PLUGOUT = 289,                 /* PLUGOUT  */
    IS = 290,                      /* IS  */
    HERE = 291,                    /* HERE  */
    THERE = 292,                   /* THERE  */
    TO = 293,                      /* TO  */
    MAP = 294,                     /* MAP  */
    BATCHLIMIT = 295,              /* BATCHLIMIT  */
    FETCHLIMIT = 296,              /* FETCHLIMIT  */
    FETCHSIZELIMIT = 297,          /* FETCHSIZELIMIT  */
    FASTUIDL = 298,                /* FASTUIDL  */
    EXPUNGE = 299,                 /* EXPUNGE  */
    PROPERTIES = 300,              /* PROPERTIES  */
    SET = 301,                     /* SET  */
    LOGFILE = 302,                 /* LOGFILE  */
    DAEMON = 303,                  /* DAEMON  */
    SYSLOG = 304,                  /* SYSLOG  */
    IDFILE = 305,                  /* IDFILE  */
    PIDFILE = 306,                 /* PIDFILE  */
    INVISIBLE = 307,               /* INVISIBLE  */
    POSTMASTER = 308,              /* POSTMASTER  */
    BOUNCEMAIL = 309,              /* BOUNCEMAIL  */
    SPAMBOUNCE = 310,              /* SPAMBOUNCE  */
    SOFTBOUNCE = 311,              /* SOFTBOUNCE  */
    SHOWDOTS = 312,                /* SHOWDOTS  */
    BADHEADER = 313,               /* BADHEADER  */
    ACCEPT = 314,                  /* ACCEPT  */
    REJECT_ = 315,                 /* REJECT_  */
    PROTO = 316,                   /* PROTO  */
    AUTHTYPE = 317,                /* AUTHTYPE  */
    STRING = 318,                  /* STRING  */
    NUMBER = 319,                  /* NUMBER  */
    NO = 320,                      /* NO  */
    KEEP = 321,                    /* KEEP  */
    FLUSH = 322,                   /* FLUSH  */
    LIMITFLUSH = 323,              /* LIMITFLUSH  */
    FETCHALL = 324,                /* FETCHALL  */
    REWRITE = 325,                 /* REWRITE  */
    FORCECR = 326,                 /* FORCECR  */
    STRIPCR = 327,                 /* STRIPCR  */
    PASS8BITS = 328,               /* PASS8BITS  */
    DROPSTATUS = 329,              /* DROPSTATUS  */
    DROPDELIVERED = 330,           /* DROPDELIVERED  */
    DNS = 331,                     /* DNS  */
    SERVICE = 332,                 /* SERVICE  */
    PORT = 333,                    /* PORT  */
    UIDL = 334,                    /* UIDL  */
    INTERVAL = 335,                /* INTERVAL  */
    MIMEDECODE = 336,              /* MIMEDECODE  */
    IDLE = 337,                    /* IDLE  */
    CHECKALIAS = 338,              /* CHECKALIAS  */
    SSL = 339,                     /* SSL  */
    SSLKEY = 340,                  /* SSLKEY  */
    SSLCERT = 341,                 /* SSLCERT  */
    SSLPROTO = 342,                /* SSLPROTO  */
    SSLCERTCK = 343,               /* SSLCERTCK  */
    SSLCERTFILE = 344,             /* SSLCERTFILE  */
    SSLCERTPATH = 345,             /* SSLCERTPATH  */
    SSLCOMMONNAME = 346,           /* SSLCOMMONNAME  */
    SSLFINGERPRINT = 347,          /* SSLFINGERPRINT  */
    PRINCIPAL = 348,               /* PRINCIPAL  */
    ESMTPNAME = 349,               /* ESMTPNAME  */
    ESMTPPASSWORD = 350,           /* ESMTPPASSWORD  */
    AUTHMETH = 351,                /* AUTHMETH  */
    TRACEPOLLS = 352               /* TRACEPOLLS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define DEFAULTS 258
#define POLL 259
#define SKIP 260
#define VIA 261
#define AKA 262
#define LOCALDOMAINS 263
#define PROTOCOL 264
#define AUTHENTICATE 265
#define TIMEOUT 266
#define KPOP 267
#define SDPS 268
#define ENVELOPE 269
#define QVIRTUAL 270
#define USERNAME 271
#define PASSWORD 272
#define FOLDER 273
#define SMTPHOST 274
#define FETCHDOMAINS 275
#define MDA 276
#define BSMTP 277
#define LMTP 278
#define SMTPADDRESS 279
#define SMTPNAME 280
#define SPAMRESPONSE 281
#define PRECONNECT 282
#define POSTCONNECT 283
#define LIMIT 284
#define WARNINGS 285
#define INTERFACE 286
#define MONITOR 287
#define PLUGIN 288
#define PLUGOUT 289
#define IS 290
#define HERE 291
#define THERE 292
#define TO 293
#define MAP 294
#define BATCHLIMIT 295
#define FETCHLIMIT 296
#define FETCHSIZELIMIT 297
#define FASTUIDL 298
#define EXPUNGE 299
#define PROPERTIES 300
#define SET 301
#define LOGFILE 302
#define DAEMON 303
#define SYSLOG 304
#define IDFILE 305
#define PIDFILE 306
#define INVISIBLE 307
#define POSTMASTER 308
#define BOUNCEMAIL 309
#define SPAMBOUNCE 310
#define SOFTBOUNCE 311
#define SHOWDOTS 312
#define BADHEADER 313
#define ACCEPT 314
#define REJECT_ 315
#define PROTO 316
#define AUTHTYPE 317
#define STRING 318
#define NUMBER 319
#define NO 320
#define KEEP 321
#define FLUSH 322
#define LIMITFLUSH 323
#define FETCHALL 324
#define REWRITE 325
#define FORCECR 326
#define STRIPCR 327
#define PASS8BITS 328
#define DROPSTATUS 329
#define DROPDELIVERED 330
#define DNS 331
#define SERVICE 332
#define PORT 333
#define UIDL 334
#define INTERVAL 335
#define MIMEDECODE 336
#define IDLE 337
#define CHECKALIAS 338
#define SSL 339
#define SSLKEY 340
#define SSLCERT 341
#define SSLPROTO 342
#define SSLCERTCK 343
#define SSLCERTFILE 344
#define SSLCERTPATH 345
#define SSLCOMMONNAME 346
#define SSLFINGERPRINT 347
#define PRINCIPAL 348
#define ESMTPNAME 349
#define ESMTPPASSWORD 350
#define AUTHMETH 351
#define TRACEPOLLS 352

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 57 "rcfile_y.y"

  int proto;
  int number;
  char *sval;

#line 266 "rcfile_y.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_RCFILE_Y_H_INCLUDED  */
