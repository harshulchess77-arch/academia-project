/*
Dmitri Buiamov
Cs50 c project
sql database
10.22.26
*/
#include <stdio.h>
#include "Academia_Database.h"

int initializeDatabase(void)
{
    sqlite3 *db;

    if (sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Failed to connect database\n");
        return 1;
    }

    printf("Database connected!\n");

    sqlite3_close(db);

    return 0;
}
