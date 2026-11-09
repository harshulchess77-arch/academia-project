/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
Database Header Interface
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <winsqlite/winsqlite3.h>

/* Opens the database and creates tables if needed */
sqlite3 *open_database(void);

/* Closes the database connection */
void close_database(sqlite3 *database);

/* Creates all required tables and default sample data */
int create_tables(sqlite3 *database);

#endif

