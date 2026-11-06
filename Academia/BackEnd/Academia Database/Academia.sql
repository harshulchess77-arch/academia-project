/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026

Database Description:
This SQL file creates the SQLite database structure for Academia,
a school management system that connects students, parents, and
school staff.

The database stores user accounts, student information, courses,
assignments, attendance, enrollment, announcements, and grades.

Primary keys uniquely identify each record.
Foreign keys connect records between related tables.
*/

/*
Enable foreign key enforcement.

SQLite does not always enforce foreign key constraints by default.
This command ensures that referenced records exist and helps
maintain relationships between tables.
*/
PRAGMA foreign_keys = ON;


/* ============================================================
   1. USER TABLE
   ============================================================

   Purpose:
   Stores the basic account information for every user.

   All students, parents, and school staff are first registered
   in this table before receiving their role-specific information.

   Columns:
   id         - Unique user identification number.
   username   - Unique username used for signing in.
   password   - Password associated with the account.
   first_name - User's first name.
   last_name  - User's last name.
   role       - Identifies whether the user is a student,
                parent, or staff member.

   AUTOINCREMENT automatically creates a new ID when one
   is not manually provided.

   UNIQUE prevents two users from sharing the same username.
   NOT NULL prevents required fields from being empty.
*/

CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    first_name TEXT,
    last_name TEXT,
    role TEXT NOT NULL
);


/* ============================================================
   2. PARENT TABLE
   ============================================================

   Purpose:
   Stores additional information belonging to parent accounts.

   Each parent record corresponds to an existing account
   in the user table.

   Columns:
   id    - Parent's user identification number.
   phone - Parent's contact phone number.

   Foreign Key:
   id references user(id).

   This means that a parent must already exist in the user
   table before their additional information can be stored.

   The same ID is used in both tables.
*/

CREATE TABLE IF NOT EXISTS parent (
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY (id) REFERENCES user(id)
);


/* ============================================================
   3. STUDENT TABLE
   ============================================================

   Purpose:
   Stores student-specific academic information.

   Each student has an account in the user table and may be
   associated with a parent.

   Columns:
   id          - Student's user identification number.
   parent_id   - ID of the student's associated parent.
   grade_level - Student's current school grade level.
   gpa         - Student's grade point average.

   Foreign Keys:
   id references user(id).
   parent_id references parent(id).

   These relationships allow the system to connect students
   to their accounts and their associated parents.

   A parent can be connected to multiple students.
*/

CREATE TABLE IF NOT EXISTS student (
    id INTEGER PRIMARY KEY,
    parent_id INTEGER,
    grade_level INTEGER,
    gpa REAL,

    FOREIGN KEY (id) REFERENCES user(id),
    FOREIGN KEY (parent_id) REFERENCES parent(id)
);


/* ============================================================
   4. STAFF TABLE
   ============================================================

   Purpose:
   Stores additional information for school staff accounts.

   Staff members can be assigned to courses using the
   course table.

   Columns:
   id    - Staff member's user identification number.
   phone - Staff member's contact phone number.

   Foreign Key:
   id references user(id).

   This ensures that staff information belongs to an
   existing user account.
*/

CREATE TABLE IF NOT EXISTS staff (
    id INTEGER PRIMARY KEY,
    phone TEXT,

    FOREIGN KEY (id) REFERENCES user(id)
);


/* ============================================================
   5. COURSE TABLE
   ============================================================

   Purpose:
   Stores courses offered by the school.

   Each course may be associated with a staff member
   responsible for that course.

   Columns:
   id          - Unique course identification number.
   staff_id    - ID of the staff member assigned to the course.
   course_name - Name of the course.

   Foreign Key:
   staff_id references staff(id).

   One staff member can be responsible for multiple courses.

   Course records are used by the assignment, attendance,
   enrollment, announcement, and grade tables.
*/

CREATE TABLE IF NOT EXISTS course (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_id INTEGER,
    course_name TEXT NOT NULL,

    FOREIGN KEY (staff_id) REFERENCES staff(id)
);


/* ============================================================
   6. ASSIGNMENT TABLE
   ============================================================

   Purpose:
   Stores assignments associated with courses.

   This information can be displayed on the student portal
   so students can view their assignments and due dates.

   Columns:
   id              - Unique assignment identification number.
   course_id       - ID of the course containing the assignment.
   assignment_name - Name or title of the assignment.
   due_date        - Date when the assignment is due.

   Foreign Key:
   course_id references course(id).

   A single course can contain multiple assignments.

   Dates are stored using the YYYY-MM-DD format.
*/

CREATE TABLE IF NOT EXISTS assignment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    assignment_name TEXT NOT NULL,
    due_date DATE NOT NULL,

    FOREIGN KEY (course_id) REFERENCES course(id)
);


/* ============================================================
   7. ATTENDANCE TABLE
   ============================================================

   Purpose:
   Records whether a student was present or absent
   for a course.

   Each attendance record connects a student to a course.

   Columns:
   id         - Unique attendance record identification number.
   course_id  - ID of the course.
   student_id - ID of the student.
   present    - Boolean value indicating attendance.

   Boolean Values:
   1 = Present
   0 = Absent

   Foreign Keys:
   course_id references course(id).
   student_id references student(id).

   A student can have multiple attendance records
   associated with different courses.
*/

CREATE TABLE IF NOT EXISTS attendance (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    student_id INTEGER,
    present BOOLEAN,

    FOREIGN KEY (course_id) REFERENCES course(id),
    FOREIGN KEY (student_id) REFERENCES student(id)
);


/* ============================================================
   8. ENROLLMENT TABLE
   ============================================================

   Purpose:
   Records which students are enrolled in which courses.

   This table creates a many-to-many relationship between
   students and courses.

   A student can enroll in multiple courses.
   A course can contain multiple students.

   Columns:
   id         - Unique enrollment identification number.
   student_id - ID of the enrolled student.
   course_id  - ID of the course.
   status     - Indicates whether enrollment is active.

   Boolean Values:
   1 = Active enrollment
   0 = Inactive enrollment

   Foreign Keys:
   student_id references student(id).
   course_id references course(id).

   The system can use these records to determine which
   courses should appear on a student's dashboard.
*/

CREATE TABLE IF NOT EXISTS enrollment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    status BOOLEAN,

    FOREIGN KEY (student_id) REFERENCES student(id),
    FOREIGN KEY (course_id) REFERENCES course(id)
);


/* ============================================================
   9. ANNOUNCEMENT TABLE
   ============================================================

   Purpose:
   Stores announcements related to individual courses.

   Staff members can create announcements that students
   and parents may view through their portals.

   Columns:
   id                - Unique announcement identification number.
   course_id         - ID of the related course.
   title             - Announcement title.
   message           - Main announcement content.
   announcement_date - Date the announcement was created.

   Foreign Key:
   course_id references course(id).

   A single course may contain multiple announcements.
*/

CREATE TABLE IF NOT EXISTS announcement (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id INTEGER,
    title TEXT,
    message TEXT,
    announcement_date DATE,

    FOREIGN KEY (course_id) REFERENCES course(id)
);


/* ============================================================
   10. GRADE TABLE
   ============================================================

   Purpose:
   Stores academic scores assigned to students.

   Each grade record connects a student to a course
   and contains the student's numerical score.

   Columns:
   id         - Unique grade record identification number.
   student_id - ID of the student receiving the grade.
   course_id  - ID of the course.
   score      - Numerical grade value.

   Foreign Keys:
   student_id references student(id).
   course_id references course(id).

   A student may receive multiple grade records
   for the same course.

   The backend can retrieve these records to display
   academic performance on student and parent portals.
*/

CREATE TABLE IF NOT EXISTS grade (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    score REAL,

    FOREIGN KEY (student_id) REFERENCES student(id),
    FOREIGN KEY (course_id) REFERENCES course(id)
);


/* ============================================================
   SAMPLE DATA INSERTION
   ============================================================

   Purpose:
   Inserts example records into the database for testing.

   These records allow the frontend and C backend to demonstrate
   login, student information, course enrollment, attendance,
   announcements, assignments, and grade displays.

   All relationships use matching identification numbers.

   Note:
   These INSERT commands are intended for initial setup.
   Running them again may cause primary key or username
   conflicts if the records already exist.
*/


/* ------------------------------------------------------------
   1. INSERT USERS

   Creates three example accounts:
   - A parent
   - A student
   - A staff member

   Each account receives a unique ID, username, password,
   first name, last name, and role.

   These IDs will be referenced by the role-specific tables.
*/

INSERT INTO user
(id, username, password, first_name, last_name, role)
VALUES
(1, 'parent01', 'Parent123', 'David', 'Smith', 'parent'),
(2, 'student01', 'Student123', 'Alex', 'Smith', 'student'),
(3, 'staff01', 'Staff123', 'Sarah', 'Johnson', 'staff');


/* ------------------------------------------------------------
   2. INSERT PARENT

   Creates the additional parent information for David Smith.

   Parent ID 1 matches user ID 1.

   The phone number is stored for contact purposes.
*/

INSERT INTO parent (id, phone)
VALUES
(1, '5551234567');


/* ------------------------------------------------------------
   3. INSERT STUDENT

   Creates the academic profile for Alex Smith.

   Student ID 2 matches user ID 2.
   Parent ID 1 connects Alex Smith to David Smith.

   Grade level: 11
   GPA: 3.85
*/

INSERT INTO student (id, parent_id, grade_level, gpa)
VALUES
(2, 1, 11, 3.85);


/* ------------------------------------------------------------
   4. INSERT STAFF

   Creates the additional staff information for Sarah Johnson.

   Staff ID 3 matches user ID 3.

   This account will be assigned to the example course.
*/

INSERT INTO staff (id, phone)
VALUES
(3, '5559876543');


/* ------------------------------------------------------------
   5. INSERT COURSE

   Creates an AP Calculus BC course.

   Course ID: 1
   Staff ID: 3

   The staff ID links this course to Sarah Johnson.

   This course will also be connected to assignments,
   enrollment, attendance, grades, and announcements.
*/

INSERT INTO course (id, staff_id, course_name)
VALUES
(1, 3, 'AP Calculus BC');


/* ------------------------------------------------------------
   6. INSERT ASSIGNMENT

   Creates an assignment for AP Calculus BC.

   Assignment ID: 1
   Course ID: 1
   Assignment name: Unit 1 Homework
   Due date: November 10, 2026

   Students enrolled in the course can view this assignment
   through the frontend.
*/

INSERT INTO assignment
(id, course_id, assignment_name, due_date)
VALUES
(1, 1, 'Unit 1 Homework', '2026-11-10');


/* ------------------------------------------------------------
   7. INSERT ENROLLMENT

   Enrolls Alex Smith in AP Calculus BC.

   Student ID: 2
   Course ID: 1
   Status: 1 (Active)

   The enrollment table allows the backend to determine
   which courses belong to each student.
*/

INSERT INTO enrollment
(id, student_id, course_id, status)
VALUES
(1, 2, 1, 1);


/* ------------------------------------------------------------
   8. INSERT ATTENDANCE

   Creates an attendance record for Alex Smith.

   Student ID: 2
   Course ID: 1
   Present: 1 (Yes)

   The student and parent portals can use this information
   to display attendance records.
*/

INSERT INTO attendance
(id, course_id, student_id, present)
VALUES
(1, 1, 2, 1);


/* ------------------------------------------------------------
   9. INSERT GRADE

   Creates a grade record for Alex Smith.

   Student ID: 2
   Course ID: 1
   Score: 95

   This score can be displayed in the student portal
   and the associated parent's portal.
*/

INSERT INTO grade
(id, student_id, course_id, score)
VALUES
(1, 2, 1, 95);


/* ------------------------------------------------------------
   10. INSERT ANNOUNCEMENT

   Creates an announcement for AP Calculus BC.

   Course ID: 1
   Title: Upcoming Test
   Date: November 4, 2026

   The message informs students about an upcoming test.

   The backend can retrieve the announcement and display
   it on the appropriate frontend pages.
*/

INSERT INTO announcement
(id, course_id, title, message, announcement_date)
VALUES
(1, 1, 'Upcoming Test',
 'The Unit 1 test is next Friday.',
 '2026-11-04');
