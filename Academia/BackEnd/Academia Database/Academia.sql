/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/

CREATE TABLE user(
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE not null,
    password TEXT not null,
    first_name TEXT,
    last_name TEXT,
    role TEXT not null
);

CREATE TABLE student(
    id INTEGER PRIMARY KEY,
    parent_id INTEGER,
    grade_level INTEGER,
    gpa REAL,

    FOREIGN KEY(id) REFERENCES user(id),
    FOREIGN KEY(parent_id) REFERENCES parent(id)

);

CREATE TABLE parent(
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id)

);

CREATE TABLE staff(
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id)
);



CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    staff_id INTEGER,
    course_name TEXT not null,

    FOREIGN KEY(staff_id) REFERENCES staff(id)
);

CREATE TABLE assignment(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    assignment_name TEXT not null,
    due_date DATE not null,

    FOREIGN KEY(course_id) REFERENCES course(id)
);

CREATE TABLE attendance(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN,

    FOREIGN KEY(course_id) REFERENCES course(id),
    FOREIGN KEY(student_id) REFERENCES student(id)
);

CREATE TABLE enrollment(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN,

    FOREIGN KEY(student_id) REFERENCES student(id),
    FOREIGN KEY(course_id) REFERENCES course(id)
);

CREATE TABLE announcement(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    message TEXT,
    announcement_date DATE,

    FOREIGN KEY(course_id) REFERENCES course(id)
);

CREATE TABLE grade(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    score REAL,

    FOREIGN KEY(student_id) REFERENCES student(id),
    FOREIGN KEY(course_id) REFERENCES course(id)
);


INSERT INTO user
(username, password, first_name, last_name, role)
VALUES
('Stu1','1234','A','B','student');
