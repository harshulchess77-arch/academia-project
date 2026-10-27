/*
Dmitri Buiamov
Cs50 c project
sql database login check with STU, PAR, TEA codes
10.22.26
*/

#include <stdio.h>
#include <string.h>
#include "SQLite3/sqlite3.h"

void checkUserLogin(const char *username, const char *password)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char sql[256];

    // Open connection to SQLite database
    if (sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Connection Failed\n");
        return;
    }

    // Auto-detect and declare account type based on STU, PAR, or TEA prefixes
    if (strncmp(username, "STU", 3) == 0)
    {
        printf("Code Identified: STU (Student Account)\n");
    }
    else if (strncmp(username, "PAR", 3) == 0)
    {
        printf("Code Identified: PAR (Parent Account)\n");
    }
    else if (strncmp(username, "TEA", 3) == 0)
    {
        printf("Code Identified: TEA (Teacher/Staff Account)\n");
    }
    else
    {
        printf("Code Identified: Unknown Format\n");
    }

    // Query the users table to verify credentials
    sprintf(sql, "SELECT role FROM users WHERE username = '%s' AND password = '%s';", username, password);

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