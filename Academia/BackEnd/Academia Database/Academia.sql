/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/

CREATE TABLE user(
    id INTEGER PRIMARY KEY,
    email TEXT,
    password TEXT,
    first_name TEXT,
    last_name TEXT,
    role TEXT
);

CREATE TABLE student(
    id INTEGER PRIMARY KEY,
    gpa REAL
);

CREATE TABLE parent(
    id INTEGER PRIMARY KEY,
    phone TEXT
    
);

CREATE TABLE staff(
    id INTEGER PRIMARY KEY,
    phone TEXT
);




CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    staff_id INTEGER 
);

CREATE TABLE assignment(
    id INTEGER PRIMARY KEY,
    course_id INTEGER
);

CREATE TABLE attendence(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN
);

CREATE TABLE enrollment(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN
);

CREATE TABLE announcement(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    message TEXT
);

CREATE TABLE grade(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER
);

