/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
/*
Dmitri Buiamov
Cs50 c project
sql database
10.22.26
*/

// Main Method

// We have added ifndef which uses to check if the header was there and define command to check if its defined and run the file asked 
#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>

//We ask sqlite3 to connect with database and use void to return with empty output.
sqlite3* open_database(void);

// With the void we also close the database after it return with empty output.
void close_database(sqlite3* database);

// We ask sqlite3 to create table with the help of sqlite3* in the database
int create_table(sqlite3* database);

// We use endif to end the process of the database.h fierl once its completed the task which we have asked it to do.
#endif
