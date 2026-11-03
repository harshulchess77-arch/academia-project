/*
Dmitri Buiamov
CS50 C Project
SQL Database
10.22.26
*/

PRAGMA foreign_keys = ON;


/*
Stores login and personal information for every account.
*/
CREATE TABLE user(
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    first_name TEXT,
    last_name TEXT,
    role TEXT NOT NULL
);


/*
Stores information for student accounts.
parent_id connects the student to a parent.
*/
CREATE TABLE student(
    id INTEGER PRIMARY KEY,
    parent_id INTEGER,
    grade_level INTEGER,
    gpa REAL,

    FOREIGN KEY(id) REFERENCES user(id),
    FOREIGN KEY(parent_id) REFERENCES parent(id)
);


/*
Stores information for parent accounts.
The parent ID matches the related user ID.
*/
CREATE TABLE parent(
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id)
);


/*
Stores information for staff accounts.
The staff ID matches the related user ID.
*/
CREATE TABLE staff(
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY(id) REFERENCES user(id)
);


/*
Stores courses and connects each course to a staff member.
*/
CREATE TABLE course(
    id INTEGER PRIMARY KEY,
    staff_id INTEGER,
    course_name TEXT NOT NULL,

    FOREIGN KEY(staff_id) REFERENCES staff(id)
);


/*
Stores assignments connected to courses.
*/
CREATE TABLE assignment(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    assignment_name TEXT NOT NULL,
    due_date DATE NOT NULL,

    FOREIGN KEY(course_id) REFERENCES course(id)
);


/*
Stores student attendance for each course.
*/
CREATE TABLE attendance(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN,

    FOREIGN KEY(course_id) REFERENCES course(id),
    FOREIGN KEY(student_id) REFERENCES student(id)
);


/*
Connects students to courses.
*/
CREATE TABLE enrollment(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN,

    FOREIGN KEY(student_id) REFERENCES student(id),
    FOREIGN KEY(course_id) REFERENCES course(id)
);


/*
Stores school and course announcements.
*/
CREATE TABLE announcement(
    id INTEGER PRIMARY KEY,
    course_id INTEGER,
    title TEXT,
    message TEXT,
    announcement_date DATE,

    FOREIGN KEY(course_id) REFERENCES course(id)
);


/*
Stores student grades for each course.
*/
CREATE TABLE grade(
    id INTEGER PRIMARY KEY,
    student_id INTEGER,
    course_id INTEGER,
    score REAL,

    FOREIGN KEY(student_id) REFERENCES student(id),
    FOREIGN KEY(course_id) REFERENCES course(id)
);


/*
Creates the three test user accounts.
The IDs are specified so the table relationships are predictable.
*/

INSERT INTO user
(id, username, password, first_name, last_name, role)
VALUES
(1, 'Stu1', '1234', 'A', 'B', 'student');

INSERT INTO user
(id, username, password, first_name, last_name, role)
VALUES
(2, 'Par1', '5678', 'C', 'D', 'parent');

INSERT INTO user
(id, username, password, first_name, last_name, role)
VALUES
(3, 'Sta1', '7891', 'E', 'F', 'staff');


/*
Creates the role-specific records.
Student 1 is connected to Parent 2.
*/

INSERT INTO parent
(id, phone)
VALUES
(2, '770-555-0100');

INSERT INTO student
(id, parent_id, grade_level, gpa)
VALUES
(1, 2, 11, 3.50);

INSERT INTO staff
(id, phone)
VALUES
(3, '770-555-0200');


/*
Creates three courses taught by Staff 3.
*/

INSERT INTO course
(id, staff_id, course_name)
VALUES
(1, 3, 'Math');

INSERT INTO course
(id, staff_id, course_name)
VALUES
(2, 3, 'Science');

INSERT INTO course
(id, staff_id, course_name)
VALUES
(3, 3, 'Spanish');


/*
Enrolls Student 1 in the three courses.
*/

INSERT INTO enrollment
(student_id, course_id, status)
VALUES
(1, 1, 1);

INSERT INTO enrollment
(student_id, course_id, status)
VALUES
(1, 2, 1);

INSERT INTO enrollment
(student_id, course_id, status)
VALUES
(1, 3, 1);


/*
Adds sample grades for Student 1.
*/

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


/*
Adds sample attendance records.
One means present and zero means absent.
*/

INSERT INTO attendance
(course_id, student_id, present)
VALUES
(1, 1, 1);

INSERT INTO attendance
(course_id, student_id, present)
VALUES
(2, 1, 1);

INSERT INTO attendance
(course_id, student_id, present)
VALUES
(3, 1, 0);


/*
Adds sample school announcements.
*/

INSERT INTO announcement
(course_id, title, message, announcement_date)
VALUES
(
    1,
    'Math Test',
    'The Math test is scheduled for Friday.',
    '2026-11-06'
);

INSERT INTO announcement
(course_id, title, message, announcement_date)
VALUES
(
    2,
    'Science Project',
    'The Science project is due next week.',
    '2026-11-09'
);


/*
Adds sample assignments.
*/

INSERT INTO assignment
(course_id, assignment_name, due_date)
VALUES
(
    1,
    'Algebra Practice',
    '2026-11-05'
);

INSERT INTO assignment
(course_id, assignment_name, due_date)
VALUES
(
    2,
    'Lab Report',
    '2026-11-10'
);