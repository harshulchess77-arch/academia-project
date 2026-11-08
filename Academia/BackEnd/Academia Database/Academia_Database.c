/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
Database Implementation
November 1, 2026
*/

#include "Database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opens the SQLite database */
sqlite3 *open_database(void) {
    sqlite3 *database = NULL;
    int result = sqlite3_open("academia.db", &database);

    if (result != SQLITE_OK) {
        printf("[Database] Connection failed to academia.db\n");
        if (database != NULL) {
            sqlite3_close(database);
        }
        return NULL;
    }

    /* Enable foreign keys */
    sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    printf("[Database] Connected successfully to SQLite3 (academia.db).\n");

    /* Ensure tables and initial data are present */
    create_tables(database);

    return database;
}

/* Closes the database connection */
void close_database(sqlite3 *database) {
    if (database != NULL) {
        sqlite3_close(database);
        printf("[Database] Connection closed.\n");
    }
}

/* Creates tables and default seed data */
int create_tables(sqlite3 *database) {
    if (database == NULL) {
        return 0;
    }

    const char *sql_schema = 
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS user ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT UNIQUE NOT NULL,"
        "  password TEXT NOT NULL,"
        "  first_name TEXT NOT NULL,"
        "  last_name TEXT NOT NULL,"
        "  role TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS parent ("
        "  id INTEGER PRIMARY KEY,"
        "  phone TEXT,"
        "  FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS student ("
        "  id INTEGER PRIMARY KEY,"
        "  parent_id INTEGER,"
        "  grade_level INTEGER DEFAULT 11,"
        "  gpa REAL DEFAULT 3.85,"
        "  FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE,"
        "  FOREIGN KEY(parent_id) REFERENCES parent(id) ON DELETE SET NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS staff ("
        "  id INTEGER PRIMARY KEY,"
        "  phone TEXT,"
        "  FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS course ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  staff_id INTEGER,"
        "  course_name TEXT NOT NULL,"
        "  FOREIGN KEY(staff_id) REFERENCES staff(id) ON DELETE SET NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS assignment ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  course_id INTEGER,"
        "  assignment_name TEXT NOT NULL,"
        "  due_date DATE NOT NULL,"
        "  FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS attendance ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  course_id INTEGER,"
        "  student_id INTEGER,"
        "  present INTEGER DEFAULT 1,"
        "  FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE,"
        "  FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS enrollment ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  student_id INTEGER,"
        "  course_id INTEGER,"
        "  status INTEGER DEFAULT 1,"
        "  FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE,"
        "  FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS announcement ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  course_id INTEGER,"
        "  title TEXT NOT NULL,"
        "  message TEXT NOT NULL,"
        "  announcement_date DATE NOT NULL,"
        "  FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS grade ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  student_id INTEGER,"
        "  course_id INTEGER,"
        "  score REAL NOT NULL,"
        "  FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE,"
        "  FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE"
        ");"
        /* Initial demo seed data */
        "INSERT OR IGNORE INTO user (id, username, password, first_name, last_name, role) VALUES "
        "(1, 'student1', 'password123', 'Alex', 'Rivera', 'student'),"
        "(2, 'student2', 'password123', 'Sophia', 'Chen', 'student'),"
        "(3, 'parent1', 'password123', 'Marcus', 'Rivera', 'parent'),"
        "(4, 'staff1', 'password123', 'Dr. Eleanor', 'Vance', 'staff');"
        "INSERT OR IGNORE INTO parent (id, phone) VALUES (3, '(555) 234-5678');"
        "INSERT OR IGNORE INTO student (id, parent_id, grade_level, gpa) VALUES (1, 3, 11, 3.85), (2, NULL, 12, 3.92);"
        "INSERT OR IGNORE INTO staff (id, phone) VALUES (4, '(555) 987-6543');"
        "INSERT OR IGNORE INTO course (id, staff_id, course_name) VALUES "
        "(1, 4, 'AP Computer Science A'),"
        "(2, 4, 'Advanced Mathematics'),"
        "(3, 4, 'Physics with Calculus'),"
        "(4, 4, 'World History & Civics');"
        "INSERT OR IGNORE INTO enrollment (id, student_id, course_id, status) VALUES "
        "(1, 1, 1, 1), (2, 1, 2, 1), (3, 1, 3, 1), (4, 1, 4, 1), (5, 2, 1, 1), (6, 2, 2, 1);"
        "INSERT OR IGNORE INTO grade (id, student_id, course_id, score) VALUES "
        "(1, 1, 1, 96.5), (2, 1, 2, 92.0), (3, 1, 3, 94.5), (4, 1, 4, 89.0), (5, 2, 1, 98.0), (6, 2, 2, 95.5);"
        "INSERT OR IGNORE INTO attendance (id, course_id, student_id, present) VALUES "
        "(1, 1, 1, 1), (2, 2, 1, 1), (3, 3, 1, 1), (4, 4, 1, 0), (5, 1, 2, 1), (6, 2, 2, 1);"
        "INSERT OR IGNORE INTO assignment (id, course_id, assignment_name, due_date) VALUES "
        "(1, 1, 'C Language Binary Search Tree Lab', '2026-11-15'),"
        "(2, 2, 'Integral Calculus Problem Set 5', '2026-11-18'),"
        "(3, 3, 'Kinematics & Optics Lab Report', '2026-11-20'),"
        "(4, 4, 'Comparative Constitutional Analysis Essay', '2026-11-25');"
        "INSERT OR IGNORE INTO announcement (id, course_id, title, message, announcement_date) VALUES "
        "(1, 1, 'CS Project Milestone Submission', 'Please submit your C server and database integration milestones by Friday 5:00 PM.', '2026-11-06'),"
        "(2, 2, 'Mathematics Review Session', 'Extra tutoring session this Thursday in Room 302 covering differentiation applications.', '2026-11-05'),"
        "(3, NULL, 'Academia Unified Digital Ecosystem Live', 'Welcome students, parents, and staff to the new unified digital portal. Education Today. A Brighter Tomorrow.', '2026-11-01');";

    char *error = NULL;
    int result = sqlite3_exec(database, sql_schema, NULL, NULL, &error);

    if (result != SQLITE_OK) {
        printf("[Database] Table creation error: %s\n", error != NULL ? error : "Unknown error");
        if (error) {
            sqlite3_free(error);
        }
        return 0;
    }

    printf("[Database] Database tables and initial records ready.\n");
    return 1;
}
