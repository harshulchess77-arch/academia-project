/*
Dmitri Buiamov
Cs50 c project
sql database
10.22.26
*/
#include <stdio.h>
#include "SQLite3/sqlite3.h"

void connectDatabase(void)
{
    sqlite3 *db;

    if (sqlite3_open("academia.db", &db) == SQLITE_OK)
    {
        printf("Database Connected\n");
    }
    else
    {
        printf("Connection Failed\n");
    }

    sqlite3_close(db);
}
