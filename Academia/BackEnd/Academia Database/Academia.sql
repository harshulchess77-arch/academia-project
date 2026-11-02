/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/


PRAGMA foreign key = ON;

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
    grade_level INTEGER,
    gpa REAL,

    FOREIGN KEY(id) REFERENCES user(id)

);

CREATE TABLE parent(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id),
    FOREIGN KEY(student_id) REFERENCES student(id)

);

CREATE TABLE staff(
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id)
);



CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    staff_id INTEGER,
    course_name TEXT not null

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

INSERT INTO user
(usrename, password, first_name, last_name, role)
VALUES
('Par1','5678','C','D','parent');

INSERT INTO user
(username, password, first_name, last_name,role)
VALUES
('Sta1','7891','E','F','staff');

INSERT INTO course
(staff_id, course_name)
VALUES
(1, 'Math');

INSERT INTO course
(staff_id, course_name)
VALUES
(1, 'Science');

INSERT INTO course
(staff_id, course_name)
VALUES
(1, 'Spanish');

INSERT INTO enrollment
(student_id, course_id, status)
VALUES
(1, 1, 1);

INSERT INTO enrollment
(student_id, course_id, status)
VALUES
(1, 2, 0);

INSERT INTO
(student_id, course_id, status)
VALUES
(1, 3, 1);

INSERT INTO grade
(student_id, course_id, score)
VALUES
(1, 1, 90);

INSERT INTO grade
(student_id, course_id, score)
VALUES
(1, 2, 80);

INSERT INTO grade
(student_id, course_id, score)
VALUES
(1, 3, 70);
