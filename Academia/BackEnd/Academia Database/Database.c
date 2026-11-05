/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/

// Main Method
// We included the libraries stdio.h, sqlite.h with Database.h to read and display the database
#include <stdio.h>
#include <sqlite3.h>
#include "Database.h"

int main(void)
{
/* WE INITIALIZE THE DATABASE*/
// We asked sqlite3 library to open the database and return with empty output and used double loop with if else condition
// Then we use use sqlite libraries with database to NULL which stores memories and connects the database
// We use sqlite3_open to call and open the database then opens our Database file name "Academia.sql"
// With use of "if(result != SQLITE_OK)" we check if the database is connected or not connected and if not connected
    //then show error message of Database Connection Failed and use sqlite3_close to close the database and return NULL

sqlite3* open_database(void) {
    sqlite3 *database = NULL;
    int result = sqlite3_open("academia.db", &database);

    if(result != SQLITE_OK) {
        printf("Database Connection Failed.\n");
        sqlite3_close(database);
        return NULL; // Return NULL only if it fails
    }

    return database; // Return the active database connection if successful
}

/* READ THE DATABASE */
// If the database is successfully connected, the code moves here and reads our key and activate the database with compeltion of this displays the message of "Database Connected Successfully."
// Enable foreign keys.
    sqlite3_exec( database,                         // We are using sqlite3_exec command whihc is used to execute the SQL command to enable foreign keys in the database
        "PRAGMA foreign_keys = ON;",                // After the command is successfully executed, it reads the PRAGMA from Database and then we will have an active database connection
    );
    printf("Database Connected Successfully.\n");   // Once all this process above is successfully completed then we display a message of "Database Connected Successfully."
    return database;
}

/* CLOSE THE DATABASE*/
// If there is a problem after the database is connected if there is a problem with the database then we execute the command of closing the Database but this
    //can also be cloased after the session is completed by the user or there is nothing in the databse then we display the message of "Database connection closed."
// Close the database connection.
void close_database(sqlite3 *database)              // We use the function with "void close_database(sqlite3* database)" to close the database connection
{
    if (database != NULL)                           // We use the if condition to check if there is any data and if none then we close the connection and display the message of "Database connection closed."
    {
        sqlite3_close(database);
        printf("Database connection closed.\n");
    }
}

// Create the tables from your SQL file.
int create_tables(sqlite3 *database)
{
    const char *sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS user ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "first_name TEXT,"
        "last_name TEXT,"
        "role TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS parent ("
        "id INTEGER PRIMARY KEY,"
        "phone TEXT,"
        "FOREIGN KEY(id) REFERENCES user(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS student ("
        "id INTEGER PRIMARY KEY,"
        "parent_id INTEGER,"
        "grade_level INTEGER,"
        "gpa REAL,"
        "FOREIGN KEY(id) REFERENCES user(id),"
        "FOREIGN KEY(parent_id) REFERENCES parent(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS staff ("
        "id INTEGER PRIMARY KEY,"
        "phone TEXT,"
        "FOREIGN KEY(id) REFERENCES user(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS course ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "staff_id INTEGER,"
        "course_name TEXT NOT NULL,"
        "FOREIGN KEY(staff_id) REFERENCES staff(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS assignment ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "course_id INTEGER,"
        "assignment_name TEXT NOT NULL,"
        "due_date DATE NOT NULL,"
        "FOREIGN KEY(course_id) REFERENCES course(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS attendance ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "course_id INTEGER,"
        "student_id INTEGER,"
        "present BOOLEAN,"
        "FOREIGN KEY(course_id) REFERENCES course(id),"
        "FOREIGN KEY(student_id) REFERENCES student(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS enrollment ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER,"
        "course_id INTEGER,"
        "status BOOLEAN,"
        "FOREIGN KEY(student_id) REFERENCES student(id),"
        "FOREIGN KEY(course_id) REFERENCES course(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS announcement ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "course_id INTEGER,"
        "title TEXT,"
        "message TEXT,"
        "announcement_date DATE,"
        "FOREIGN KEY(course_id) REFERENCES course(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS grade ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER,"
        "course_id INTEGER,"
        "score REAL,"
        "FOREIGN KEY(student_id) REFERENCES student(id),"
        "FOREIGN KEY(course_id) REFERENCES course(id)"
        ");";

    char *error = NULL;
    int result = sqlite3_exec(
        database,
        sql,
        NULL,
        NULL,
        &error
    );
    if (result != SQLITE_OK)
    {
        printf("Table creation error: %s\n", error);
        sqlite3_free(error);
        return 0;
    }
    printf("Database tables are ready.\n");
    return 0;
}
}
