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

// We use this command to make sure SQLite enforces foreign key constraints correctly.
// This ensures that all referenced records actually exist and helps maintain 
// a clean relationship between our different database tables.
*/
PRAGMA foreign_keys = ON;


/* ============================================================
    1. USER TABLE
    ============================================================

    Purpose:
    // Stores the basic account information for every single user in the system.
    // All students, parents, and school staff must first be registered
    // in this table before getting their role-specific information.

    Columns:
    id         - Unique user identification number (Auto-incremented primary key).
    username   - Unique username used for signing into the portal.
    password   - Password associated with the account.
    first_name - User's first name.
    last_name  - User's last name.
    role       - Identifies whether the user is a student, parent, or staff member.
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
    // Stores additional information belonging specifically to parent accounts.
    // Each parent record corresponds directly to an existing account
    // in the user table sharing the exact same ID.

    Columns:
    id    - Parent's user identification number.
    phone - Parent's contact phone number.

    Foreign Key:
    // id references user(id) - This ensures a parent must already exist in the user
    // table before their additional phone information can be stored.
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
    // Stores student-specific academic information and metrics.
    // Each student has a base account in the user table and may be
    // associated with a parent account.

    Columns:
    id          - Student's user identification number.
    parent_id   - ID of the student's associated parent.
    grade_level - Student's current school grade level.
    gpa         - Student's grade point average.

    Foreign Keys:
    // id references user(id) - Links student details to their user account.
    // parent_id references parent(id) - Connects the student to their guardian in the parent table.
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
    // Stores additional professional information for school staff accounts.
    // Staff members can be assigned to manage courses using the course table.

    Columns:
    id    - Staff member's user identification number.
    phone - Staff member's contact phone number.

    Foreign Key:
    // id references user(id) - Ensures staff information belongs to an
    // existing user account in the system.
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
    // Stores courses offered by the school institution.
    // Each course may be associated with a staff member who is
    // responsible for teaching and managing that course.

    Columns:
    id          - Unique course identification number.
    staff_id    - ID of the staff member assigned to the course.
    course_name - Name of the course.

    Foreign Key:
    // staff_id references staff(id) - Links the course to its assigned instructor.
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
    // Stores homework and tests associated with specific courses.
    // This information is displayed on the student portal so students 
    // can easily view their assignments and upcoming due dates.

    Columns:
    id              - Unique assignment identification number.
    course_id       - ID of the course containing the assignment.
    assignment_name - Name or title of the assignment.
    due_date        - Date when the assignment is due (YYYY-MM-DD).

    Foreign Key:
    // course_id references course(id) - Connects the assignment directly to a course.
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
    // Records whether a student was present or absent for a course session.
    // Each attendance record connects a specific student to a specific course.

    Columns:
    id         - Unique attendance record identification number.
    course_id  - ID of the course.
    student_id - ID of the student.
    present    - Boolean value indicating attendance (1 = Present, 0 = Absent).

    Foreign Keys:
    // course_id references course(id).
    // student_id references student(id).
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
    // Records which students are enrolled in which courses.
    // This table creates a many-to-many relationship between
    // students and courses.

    Columns:
    id         - Unique enrollment identification number.
    student_id - ID of the enrolled student.
    course_id  - ID of the course.
    status     - Indicates whether enrollment is active (1 = Active, 0 = Inactive).

    Foreign Keys:
    // student_id references student(id).
    // course_id references course(id).
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
    // Stores announcements related to individual courses.
    // Staff members can create announcements that students
    // and parents can view through their portals.

    Columns:
    id                - Unique announcement identification number.
    course_id         - ID of the related course.
    title             - Announcement title.
    message           - Main announcement content.
    announcement_date - Date the announcement was created.

    Foreign Key:
    // course_id references course(id).
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
    // Stores academic scores assigned to students.
    // Each grade record connects a student to a course
    // and contains the student's numerical score value.

    Columns:
    id         - Unique grade record identification number.
    student_id - ID of the student receiving the grade.
    course_id  - ID of the course.
    score      - Numerical grade value.

    Foreign Keys:
    // student_id references student(id).
    // course_id references course(id).
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
    // Inserts example records into the database for testing and demonstration.
    // These records allow the frontend and C backend to test
    // login, student information, course enrollment, attendance,
    // announcements, assignments, and grade displays.
*/


/* ------------------------------------------------------------
    1. INSERT USERS
    // We insert three example accounts into our user table:
    // - A parent (David Smith)
    // - A student (Alex Smith)
    // - A staff member (Sarah Johnson)
*/

INSERT INTO user
(id, username, password, first_name, last_name, role)
VALUES
(1, 'parent01', 'Parent123', 'David', 'Smith', 'parent'),
(2, 'student01', 'Student123', 'Alex', 'Smith', 'student'),
(3, 'staff01', 'Staff123', 'Sarah', 'Johnson', 'staff');


/* ------------------------------------------------------------
    2. INSERT PARENT
    // We create the additional parent information for David Smith.
    // Parent ID 1 matches user ID 1 and stores his contact phone number.
*/

INSERT INTO parent (id, phone)
VALUES
(1, '5551234567');


/* ------------------------------------------------------------
    3. INSERT STUDENT
    // We create the academic profile for Alex Smith.
    // Student ID 2 matches user ID 2 and parent ID 1 connects Alex to David.
    // We set the grade level to 11 and GPA to 3.85.
*/

INSERT INTO student (id, parent_id, grade_level, gpa)
VALUES
(2, 1, 11, 3.85);


/* ------------------------------------------------------------
    4. INSERT STAFF
    // We create the additional staff information for Sarah Johnson.
    // Staff ID 3 matches user ID 3 and stores her contact phone number.
*/

INSERT INTO staff (id, phone)
VALUES
(3, '5559876543');


/* ------------------------------------------------------------
    5. INSERT COURSE
    // We create an AP Calculus BC course with ID 1 and assign it
    // to staff member Sarah Johnson (Staff ID 3).
*/

INSERT INTO course (id, staff_id, course_name)
VALUES
(1, 3, 'AP Calculus BC');


/* ------------------------------------------------------------
    6. INSERT ASSIGNMENT
    // We create an assignment for AP Calculus BC named 'Unit 1 Homework'
    // with a due date of November 10, 2026.
*/

INSERT INTO assignment
(id, course_id, assignment_name, due_date)
VALUES
(1, 1, 'Unit 1 Homework', '2026-11-10');


/* ------------------------------------------------------------
    7. INSERT ENROLLMENT
    // We enroll student Alex Smith (Student ID 2) into AP Calculus BC (Course ID 1)
    // with an active status of 1.
*/

INSERT INTO enrollment
(id, student_id, course_id, status)
VALUES
(1, 2, 1, 1);


/* ------------------------------------------------------------
    8. INSERT ATTENDANCE
    // We create an attendance record for Alex Smith showing they were
    // present (value 1) for AP Calculus BC.
*/

INSERT INTO attendance
(id, course_id, student_id, present)
VALUES
(1, 1, 2, 1);


/* ------------------------------------------------------------
    9. INSERT GRADE
    // We create a grade record for Alex Smith in AP Calculus BC
    // with a numerical score of 95.
*/

INSERT INTO grade
(id, student_id, course_id, score)
VALUES
(1, 2, 1, 95);


/* ------------------------------------------------------------
    10. INSERT ANNOUNCEMENT
    // We create an announcement for AP Calculus BC titled 'Upcoming Test'
    // to notify students about the unit test scheduled for next Friday.
*/

INSERT INTO announcement
(id, course_id, title, message, announcement_date)
VALUES
(1, 1, 'Upcoming Test',
 'The Unit 1 test is next Friday.',
 '2026-11-04');
