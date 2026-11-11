/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
October 27, 2026
*/

// Database Header

// We check for the header file if its defined or not and then we use define to
// use it and show the rest of the code in the file
#ifndef DATABASE_H // We use the "ifndef" to call whether the file is there
                    // or not there
#define DATABASE_H // We use the "define" to call the file to read the code
                    // inside after the "ifndef" is successful

// We add required libraries to make sure the code runs properly and for that we
// have used "sqlite3.h"
#include <sqlite3.h>

// Open the SQLite database connection.
sqlite3 *open_database(void);

// Close the SQLite database connection.
void close_database(sqlite3 *database);

// Create all required database tables and initial seed records.
int create_tables(sqlite3 *database);

#endif