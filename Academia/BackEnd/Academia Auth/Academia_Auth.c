/*
Dmitri Buiamov
Cs50 c project
sql database login check
10.22.26
*/

/*
This function is used for login
authentication with the specified role
in the school system. It first connect to 
the database and then detect the user type 
format and compare to the records in database
to see if it's valid.
*/

// Main Method
//We have added the basic and required libraries which are required for our project
#include <stdio.h>
#include <string.h>
#include "SQLite3/sqlite3.h"

void checkUserLogin(const char *username, const char *password, const char *role)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char sql[256];

    // Open connection to the database and check if it's connected
    if (sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Connection Failed\n");
        return;
    }


    //sends a query to database using this logic to check if user is there
    //do the username and password check in the sql database
    sprintf(sql, "SELECT role FROM user WHERE username = '%s' AND password = '%s';", username, password);

    // We are using sqlite3_prepare_v2 which will take inputs from database and write into the statements
    //we using if else statement loop to get this condition true
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        const char *dbRole = NULL;

        //run the query and move to the first result
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            dbRole = (const char *)sqlite3_column_text(stmt, 0);
            printf("Database Verified Role: %s\n", dbRole);
        }
        else
        {
            printf("Invalid Username or Password\n");
        }
    }



    // logic to find which account is logging in
    if(strcmp(role, dbRole) == 0)
    {
        if(role == "Student")
        {
            //student account
            printf("Student Account\n");
        }
        else if(role == "Parent")
        {
            //parent account
            printf("Parent Account\n");
        }
        else if(role == "Staff")
        {
            //staff account
            printf("Staff Account\n");
        }
    }
    


    sqlite3_finalize(stmt); //to clean up sql
    sqlite3_close(db); //close the database after running method
}