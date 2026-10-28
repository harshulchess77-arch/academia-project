/*
Dmitri Buiamov
Cs50 c project
sql database
10.22.26
*/

CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT,
    password TEXT,
    role TEXT
);

CREATE TABLE students (
    id INTEGER PRIMARY KEY,
    user_id INTEGER,
    first_name TEXT,
    last_name TEXT,
    grade_level INTEGER,
    gpa REAL
);

CREATE TABLE parents (
    id INTEGER PRIMARY KEY,
    user_id INTEGER,
    first_name TEXT,
    last_name TEXT
);

CREATE TABLE grades (
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course TEXT,
    grade INTEGER
);

CREATE TABLE attendance (
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    date TEXT,
    status TEXT
);

CREATE TABLE announcements (
    id INTEGER PRIMARY KEY,
    title TEXT,
    message TEXT
);
