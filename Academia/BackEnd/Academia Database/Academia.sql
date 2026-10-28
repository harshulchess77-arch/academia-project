/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/

<<<<<<< HEAD
<<<<<<< HEAD
CREATE TABLE IF NOT EXISTS users (
=======
CREATE TABLE users (
>>>>>>> 9d33377fabf30fa9951f72e1bfc51ff44adbcbeb
    id INTEGER PRIMARY KEY,
    username TEXT,
    password TEXT,
    role TEXT
);

CREATE TABLE IF NOT EXISTS students (
=======
/*make sure to do the check for the ids*/

CREATE TABLE user(
>>>>>>> eb0d9dc6fbf57d4e1546fb2663603334b1e7aa64
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    role TEXT NOT NULL,

<<<<<<< HEAD
CREATE TABLE IF NOT EXISTS parents (
    id INTEGER PRIMARY KEY,
    user_id INTEGER,
=======
>>>>>>> eb0d9dc6fbf57d4e1546fb2663603334b1e7aa64
    first_name TEXT,
    last_name TEXT
);

<<<<<<< HEAD
CREATE TABLE IF NOT EXISTS parent_student (
    parent_id INTEGER,
    student_id INTEGER
);

CREATE TABLE IF NOT EXISTS grades (
=======

CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    course_name TEXT
);

CREATE TABLE attendance(
>>>>>>> eb0d9dc6fbf57d4e1546fb2663603334b1e7aa64
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN
);

<<<<<<< HEAD
CREATE TABLE IF NOT EXISTS attendance (
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    date TEXT,
    status TEXT
);

CREATE TABLE IF NOT EXISTS announcements (
=======
CREATE TABLE announcement(
>>>>>>> eb0d9dc6fbf57d4e1546fb2663603334b1e7aa64
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    message TEXT    
);

<<<<<<< HEAD
/*
Test data
INSERT OR IGNORE prevents duplicate primary key errors.
*/

INSERT OR IGNORE INTO users (
    id,
    username,
    password,
    role
)
VALUES
    (1, 'parent01', 'password123', 'parent'),
    (2, 'student01', 'password123', 'student');

INSERT OR IGNORE INTO parents (
    id,
    user_id,
    first_name,
    last_name
)
VALUES (
    1,
    1,
    'John',
    'Smith'
);

INSERT OR IGNORE INTO students (
    id,
    user_id,
    first_name,
    last_name,
    grade_level,
    gpa
)
VALUES (
    1,
    2,
    'Alex',
    'Smith',
    11,
    3.67
);

INSERT OR IGNORE INTO parent_student (
    parent_id,
    student_id
)
VALUES (
    1,
    1
);

INSERT OR IGNORE INTO grades (
    id,
    student_id,
    course,
    grade
)
VALUES
    (1, 1, 'Computer Science', 95),
    (2, 1, 'Calculus', 88),
    (3, 1, 'Physics', 92);

INSERT OR IGNORE INTO attendance (
    id,
    student_id,
    date,
    status
)
VALUES
    (1, 1, '2026-10-21', 'Present'),
    (2, 1, '2026-10-22', 'Present'),
    (3, 1, '2026-10-23', 'Absent');

INSERT OR IGNORE INTO announcements (
    id,
    title,
    message
)
VALUES (
    1,
    'Quarter 1 Grades',
    'Quarter 1 grades are posted.'
);
=======
CREATE TABLE assignment(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    description TEXT    
);

CREATE TABLE enrollment(
    course_id INTEGER,
    student_id INTEGER,
    PRIMARY KEY(course_id, student_id)
);

CREATE TABLE grade(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    assignment_id INTEGER,
    grade REAL    
);



>>>>>>> eb0d9dc6fbf57d4e1546fb2663603334b1e7aa64
