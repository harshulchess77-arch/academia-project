/*
Dmitri Buiamov
Cs50 c project
sql database login check
10.22.26
*/

// Main Method
//We have added the basic and required libraries which are required for our project
#include <stdio.h>
#include <string.h>
#include "SQLite3/sqlite3.h"

void checkUserLogin(const char *username, const char *password)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char sql[256];

// Open connection to SQLite database matching Database_2.c
    if (sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Connection Failed\n");
        return;
    }

    // 1. Auto-detect account type based on username format prefix
    if (strncmp(username, "STU_", 4) == 0)
    {
        printf("Format Auto-Detected: Student Account\n");
    }
    else if (strncmp(username, "PAR_", 4) == 0)
    {
        printf("Format Auto-Detected: Parent Account\n");
    }
    else if (strncmp(username, "STAFF_", 6) == 0)
    {
        printf("Format Auto-Detected: Staff Account\n");
    }
    else
    {
        printf("Format Auto-Detected: General User\n");
    }

    // 2. Query the users table from Academia_2.sql to verify credentials
    sprintf(sql, "SELECT role FROM user WHERE username = '%s' AND password = '%s';", username, password);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *role = (const char *)sqlite3_column_text(stmt, 0);
            printf("Database Verified Role: %s\n", role);
        }
        else
        {
            printf("Invalid Username or Password\n");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}