/*
Dmitri Buiamov
Cs50 c project
sql database
10.22.26
*/

/*make sure to do the check for the ids*/

CREATE TABLE user(
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    role TEXT NOT NULL,

    first_name TEXT,
    last_name TEXT
);


CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    course_name TEXT
);

CREATE TABLE attendance(
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



