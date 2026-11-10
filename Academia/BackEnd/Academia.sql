-- Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
-- CS50 C Project - Academia Database Schema & Initial Data
-- Database with SQLite3

PRAGMA foreign_keys = ON;

-- 1. Users Table
CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('student', 'parent', 'staff'))
);

-- 2. Parents Table
CREATE TABLE IF NOT EXISTS parent (
    id INTEGER PRIMARY KEY,
    phone TEXT,
    FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE
);

-- 3. Students Table
CREATE TABLE IF NOT EXISTS student (
    id INTEGER PRIMARY KEY,
    parent_id INTEGER,
    grade_level INTEGER DEFAULT 11,
    gpa REAL DEFAULT 3.85,
    FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE,
    FOREIGN KEY(parent_id) REFERENCES parent(id) ON DELETE SET NULL
);

-- 4. Staff Table
CREATE TABLE IF NOT EXISTS staff (
    id INTEGER PRIMARY KEY,
    phone TEXT,
    FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE
);

-- 5. Courses Table
CREATE TABLE IF NOT EXISTS course (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_id INTEGER,
    course_name TEXT NOT NULL,
    FOREIGN KEY(staff_id) REFERENCES staff(id) ON DELETE SET NULL
);

-- 6. Assignments Table
CREATE TABLE IF NOT EXISTS assignment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    assignment_name TEXT NOT NULL,
    due_date DATE NOT NULL,
    FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE
);

-- 7. Attendance Table
CREATE TABLE IF NOT EXISTS attendance (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    student_id INTEGER,
    present INTEGER DEFAULT 1,
    FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE,
    FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE
);

-- 8. Enrollment Table
CREATE TABLE IF NOT EXISTS enrollment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    status INTEGER DEFAULT 1,
    FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE,
    FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE
);

-- 9. Announcements Table
CREATE TABLE IF NOT EXISTS announcement (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    title TEXT NOT NULL,
    message TEXT NOT NULL,
    announcement_date DATE NOT NULL,
    FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE
);

-- 10. Grades Table
CREATE TABLE IF NOT EXISTS grade (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    score REAL NOT NULL,
    FOREIGN KEY(student_id) REFERENCES student(id) ON DELETE CASCADE,
    FOREIGN KEY(course_id) REFERENCES course(id) ON DELETE CASCADE
);

-- Seed Data (Initial Users & School Records)
-- Passwords: password123 for all demo accounts
INSERT OR IGNORE INTO user (id, username, password, first_name, last_name, role) VALUES
(1, 'student1', 'password123', 'Alex', 'Rivera', 'student'),
(2, 'student2', 'password123', 'Sophia', 'Chen', 'student'),
(3, 'parent1', 'password123', 'Marcus', 'Rivera', 'parent'),
(4, 'staff1', 'password123', 'Dr. Eleanor', 'Vance', 'staff');

INSERT OR IGNORE INTO parent (id, phone) VALUES
(3, '(555) 234-5678');

INSERT OR IGNORE INTO student (id, parent_id, grade_level, gpa) VALUES
(1, 3, 11, 3.85),
(2, NULL, 12, 3.92);

INSERT OR IGNORE INTO staff (id, phone) VALUES
(4, '(555) 987-6543');

INSERT OR IGNORE INTO course (id, staff_id, course_name) VALUES
(1, 4, 'Complete HTML Tutorial'),
(2, 4, 'Complete CSS Tutorial'),
(3, 4, 'Complete JavaScript Tutorial');

INSERT OR IGNORE INTO enrollment (id, student_id, course_id, status) VALUES
(1, 1, 1, 1),
(2, 1, 2, 1),
(3, 1, 3, 1),
(4, 2, 1, 1),
(5, 2, 2, 1);

INSERT OR IGNORE INTO grade (id, student_id, course_id, score) VALUES
(1, 1, 1, 97.0),
(2, 1, 2, 96.5),
(3, 1, 3, 95.0),
(4, 2, 1, 98.0),
(5, 2, 2, 94.0);

INSERT OR IGNORE INTO attendance (id, course_id, student_id, present) VALUES
(1, 1, 1, 1),
(2, 2, 1, 1),
(3, 3, 1, 1),
(4, 1, 2, 1),
(5, 2, 2, 1);
[cite: 9]```

---

### 2. Database Module (`BackEnd/Database.h` & `BackEnd/Academia_Database.c`)

#### `BackEnd/Database.h`
```c
/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

sqlite3 *open_database(void);
void close_database(sqlite3 *database);
int create_tables(sqlite3 *database);

#endif