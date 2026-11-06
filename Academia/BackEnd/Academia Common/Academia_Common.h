/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
/*
Lawrence Mao
Seungho Kim
Harshul Gupta
Dmitrii Buianov

CS50 C Project
Common Backend Function Declarations
10.22.26
*/

#ifndef ACADEMIA_COMMON_H
#define ACADEMIA_COMMON_H

#include <stdio.h>

#include "../Academia Database/SQLite3 Source Code/sqlite3.h"


/*
Opens the Academia database.

Returns a valid SQLite connection when successful.
Returns NULL when the database cannot be opened.
*/
sqlite3 *open_academia_database(void);


/*
Finalizes a prepared SQLite statement and resets it to NULL.
*/
void finalize_statement(sqlite3_stmt **stmt);


/*
Escapes SQLite text before it is written inside a
double-quoted JavaScript string.
*/
void write_js_text(
    FILE *out,
    const unsigned char *text
);


#endif