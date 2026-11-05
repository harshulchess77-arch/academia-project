/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
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


-- value insertion
-- 1. USER
INSERT INTO user (id, username, password, first_name, last_name, role)
VALUES
(1, 'parent01', 'Parent123', 'David', 'Smith', 'parent'),
(2, 'student01', 'Student123', 'Alex', 'Smith', 'student'),
(3, 'staff01', 'Staff123', 'Sarah', 'Johnson', 'staff');


-- 2. PARENT
INSERT INTO parent (id, phone)
VALUES
(1, '5551234567');


-- 3. STUDENT
INSERT INTO student (id, parent_id, grade_level, gpa)
VALUES
(2, 1, 11, 3.85);


-- 4. STAFF
INSERT INTO staff (id, phone)
VALUES
(3, '5559876543');


-- 5. COURSE
INSERT INTO course (id, staff_id, course_name)
VALUES
(1, 3, 'AP Calculus BC');


-- 6. ASSIGNMENT
INSERT INTO assignment (id, course_id, assignment_name, due_date)
VALUES
(1, 1, 'Unit 1 Homework', '2026-11-10');


-- 7. ENROLLMENT
INSERT INTO enrollment (id, student_id, course_id, status)
VALUES
(1, 2, 1, 'active');


-- 8. ATTENDANCE
INSERT INTO attendance (id, course_id, student_id, present)
VALUES
(1, 1, 2, 1);


-- 9. GRADE
INSERT INTO grade (id, student_id, course_id, score)
VALUES
(1, 2, 1, 95);


-- 10. ANNOUNCEMENT
INSERT INTO announcement
(id, course_id, title, message, announcement_date)
VALUES
(1, 1, 'Upcoming Test', 'The Unit 1 test is next Friday.', '2026-11-04');