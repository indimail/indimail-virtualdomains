/*****************************************************************************

NAME:
   bogotune.h -- definitions and prototypes for bogotune.c

******************************************************************************/

#ifndef BOGOTUNE_H
#define BOGOTUNE_H

/* typedefs */

typedef struct result_s {
    uint idx;
    uint rsi;
    uint mdi;
    uint rxi;
    uint spi;
    uint nsi;

    double rs;
    double md;
    double rx;
    double co;
    double sp_exp;
    double ns_exp;

    uint fp;
    uint fn;
} result_t;

#endif
