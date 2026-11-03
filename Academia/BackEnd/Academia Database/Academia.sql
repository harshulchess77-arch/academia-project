/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/


PRAGMA foreign_keys = ON;


-- USER TABLE

CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    first_name TEXT,
    last_name TEXT,
    role TEXT NOT NULL
);


-- PARENT TABLE

CREATE TABLE IF NOT EXISTS parent (
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY (id) REFERENCES user(id)
);


-- STUDENT TABLE

CREATE TABLE IF NOT EXISTS student (
    id INTEGER PRIMARY KEY,
    parent_id INTEGER,
    grade_level INTEGER,
    gpa REAL,

    FOREIGN KEY (id) REFERENCES user(id),
    FOREIGN KEY (parent_id) REFERENCES parent(id)
);


-- STAFF TABLE

CREATE TABLE IF NOT EXISTS staff (
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY (id) REFERENCES user(id)
);


-- COURSE TABLE

CREATE TABLE IF NOT EXISTS course (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_id INTEGER,
    course_name TEXT NOT NULL,

    FOREIGN KEY (staff_id) REFERENCES staff(id)
);


-- ASSIGNMENT TABLE

CREATE TABLE IF NOT EXISTS assignment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    assignment_name TEXT NOT NULL,
    due_date DATE NOT NULL,

    FOREIGN KEY (course_id) REFERENCES course(id)
);


-- ATTENDANCE TABLE

CREATE TABLE IF NOT EXISTS attendance (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN,

    FOREIGN KEY (course_id) REFERENCES course(id),
    FOREIGN KEY (student_id) REFERENCES student(id)
);


-- ENROLLMENT TABLE

CREATE TABLE IF NOT EXISTS enrollment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN,

    FOREIGN KEY (student_id) REFERENCES student(id),
    FOREIGN KEY (course_id) REFERENCES course(id)
);


-- ANNOUNCEMENT TABLE

CREATE TABLE IF NOT EXISTS announcement (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    title TEXT,
    message TEXT,
    announcement_date DATE,

    FOREIGN KEY (course_id) REFERENCES course(id)
);


-- GRADE TABLE

CREATE TABLE IF NOT EXISTS grade (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    score REAL,

    FOREIGN KEY (student_id) REFERENCES student(id),
    FOREIGN KEY (course_id) REFERENCES course(id)
);
