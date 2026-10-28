/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/

--user table holds all the user infos in general
CREATE TABLE user(
    id INTEGER PRIMARY KEY,
    email TEXT,
    password TEXT,
    first_name TEXT,
    last_name TEXT,
    role TEXT
);

--student table
CREATE TABLE student(
    id INTEGER PRIMARY KEY,
    gpa REAL
);

--parent table
CREATE TABLE parent(
    id INTEGER PRIMARY KEY,
    phone TEXT
    
);

--staff table
CREATE TABLE staff(
    id INTEGER PRIMARY KEY,
    phone TEXT
);



-----------these are the relatively shared info------------
--course info
CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    staff_id INTEGER 
);

--assignment info
CREATE TABLE assignment(
    id INTEGER PRIMARY KEY,
    course_id INTEGER
);

--student attendence info
CREATE TABLE attendence(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN
);

--student enrollment status
CREATE TABLE enrollment(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN
);

--announcement inside a course
CREATE TABLE announcement(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    message TEXT
);

--grade student earned for the course
CREATE TABLE grade(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER
);

