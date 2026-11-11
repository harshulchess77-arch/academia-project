/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
*/
#include "Database.h"
#include <stdio.h>
#include <stdlib.h>

sqlite3 *open_database(void) {
    sqlite3 *database = NULL;
    int result = sqlite3_open("academia.db", &database);
    if (result != SQLITE_OK) {
        printf("[Database] Connection failed to academia.db\n");
        if (database) sqlite3_close(database);
        return NULL;
    }
    sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    create_tables(database);
    return database;
}

void close_database(sqlite3 *database) {
    if (database) sqlite3_close(database);
}

int create_tables(sqlite3 *database) {
    if (!database) return 0;
    const char *sql_schema = 
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS user (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password TEXT NOT NULL, first_name TEXT NOT NULL, last_name TEXT NOT NULL, role TEXT NOT NULL);"
        "CREATE TABLE IF NOT EXISTS parent (id INTEGER PRIMARY KEY, phone TEXT, FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE);"
        "CREATE TABLE IF NOT EXISTS student (id INTEGER PRIMARY KEY, parent_id INTEGER, grade_level INTEGER DEFAULT 11, gpa REAL DEFAULT 3.85, FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE, FOREIGN KEY(parent_id) REFERENCES parent(id) ON DELETE SET NULL);"
        "CREATE TABLE IF NOT EXISTS staff (id INTEGER PRIMARY KEY, phone TEXT, FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE);"
        "CREATE TABLE IF NOT EXISTS course (id INTEGER PRIMARY KEY AUTOINCREMENT, staff_id INTEGER, course_name TEXT NOT NULL);"
        "CREATE TABLE IF NOT EXISTS assignment (id INTEGER PRIMARY KEY AUTOINCREMENT, course_id INTEGER, assignment_name TEXT NOT NULL, due_date DATE NOT NULL);"
        "CREATE TABLE IF NOT EXISTS attendance (id INTEGER PRIMARY KEY AUTOINCREMENT, course_id INTEGER, student_id INTEGER, present INTEGER DEFAULT 1);"
        "CREATE TABLE IF NOT EXISTS enrollment (id INTEGER PRIMARY KEY AUTOINCREMENT, student_id INTEGER, course_id INTEGER, status INTEGER DEFAULT 1);"
        "CREATE TABLE IF NOT EXISTS announcement (id INTEGER PRIMARY KEY AUTOINCREMENT, course_id INTEGER, title TEXT NOT NULL, message TEXT NOT NULL, announcement_date DATE NOT NULL);"
        "CREATE TABLE IF NOT EXISTS grade (id INTEGER PRIMARY KEY AUTOINCREMENT, student_id INTEGER, course_id INTEGER, score REAL NOT NULL);"
        "INSERT OR IGNORE INTO user (id, username, password, first_name, last_name, role) VALUES "
        "(1, 'student1', 'password123', 'Alex', 'Rivera', 'student'),"
        "(3, 'parent1', 'password123', 'Marcus', 'Rivera', 'parent'),"
        "(4, 'staff1', 'password123', 'Dr. Eleanor', 'Vance', 'staff');"
        "INSERT OR IGNORE INTO parent (id, phone) VALUES (3, '(555) 234-5678');"
        "INSERT OR IGNORE INTO student (id, parent_id, grade_level, gpa) VALUES (1, 3, 11, 3.85);"
        "INSERT OR IGNORE INTO staff (id, phone) VALUES (4, '(555) 987-6543');"
        "INSERT OR IGNORE INTO course (id, staff_id, course_name) VALUES (1, 4, 'Complete HTML Tutorial'), (2, 4, 'Complete CSS Tutorial'), (3, 4, 'Complete JavaScript Tutorial');";

    char *error = NULL;
    sqlite3_exec(database, sql_schema, NULL, NULL, &error);
    if (error) { sqlite3_free(error); return 0; }
    return 1;
}