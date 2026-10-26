/*
Lawrence Mao
Suengho Kim
Harshul Gupta
Dmitrii Buianov

Cs50 c project
common functions
10.22.26
*/

#include <stdio.h>
#include <cs50.h>
#include <sqlite3.h>

#include "Academia_Parents.c"
#include "Academia_Student.c"
#include "Academia_school_Staff.c"
#include "Academia_Database.h"
#include "SQLite3/sqlite3.h"

int main()
{
    connectDatabase();
    sqlite3 *db;

    if(sqlite3_open("academia.db", &db) == SQLITE_OK)
    {
        printf("Database opened!\n");
    }

    sqlite3_close(db);

    return 0;
}
