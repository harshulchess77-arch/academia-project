/*
   ==========================================================================
   ACADEMIA - STUDENT MODULE  (Academia_Student.c)
   Writes student_states.js, the file the Student Portal reads
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================

      WHAT THIS FILE DOES
   -------------------
   When a student signs in, the server calls update_student_frontend_state(id).
   This file then asks the database for that student's record, their courses,
   grades, attendance, assignments and the school announcements, and writes
   every answer into a JavaScript file:

       FrontEnd/JS/student_states.js

   The Student Portal page reads that file, so the page always shows the
   records that are really in the database.

   HOW IT IS ORGANISED
   -------------------
   1. write_js_text()      - copies one piece of text into the output file
   2. write_js_quoted()    - the same, wrapped in quotation marks
   3. open_output_file()   - creates student_states.js
   4. one function per section of the state file:
        write_student_section()      - name, ID, grade level, GPA
        write_courses_section()      - the courses the student takes
        write_grades_section()       - the score in each course
        write_attendance_section()   - Present or Absent, per course
        write_assignments_section()  - work due, earliest first
        write_announcements_section()- school and course notices
   5. update_student_frontend_state() - the function the server calls;
      it opens everything, calls the functions above, then closes up.

*/
<<<<<<< HEAD

#include "academia_export.h"
=======
>>>>>>> 57558642637b144e2b83d4799c7f861c409ca802

#include "Academia_Student.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsqlite/winsqlite3.h>

/* -------------------------------------------------------------------------
   write_js_text
   Copies one piece of text into the output file, adding a backslash before
   any character that would otherwise break a JavaScript string.

   Without this, a student called  Ann "Annie" Lee  would end the string
   early and the browser would refuse to read the file.
   ------------------------------------------------------------------------- */
static void write_js_text(FILE *out, const unsigned char *text) {
    int i;

    /* A database column can be empty. If so there is nothing to write. */
    if (text == NULL) {
        return;
    }

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\\' || text[i] == '"') {
            /* A quote or a backslash needs a backslash in front of it. */
            fputc('\\', out);
            fputc(text[i], out);
        } else if (text[i] == '\n') {
            /* A new line becomes the two characters \ and n. */
            fputs("\\n", out);
        } else if (text[i] == '\t') {
            /* A tab becomes the two characters \ and t. */
            fputs("\\t", out);
        } else if (text[i] == '\r') {
            /* Carriage returns are dropped; they upset JavaScript. */
        } else {
            /* An ordinary character is copied as it is. */
            fputc(text[i], out);
        }
    }
}

/* -------------------------------------------------------------------------
   write_js_quoted
   Writes a piece of text surrounded by quotation marks, using the helper
   above for the text itself.
   ------------------------------------------------------------------------- */
static void write_js_quoted(FILE *out, const unsigned char *text) {
    fputc('"', out);
    write_js_text(out, text);
    fputc('"', out);
}

/* -------------------------------------------------------------------------
   write_student_section
   Writes the student's own details: name, username, ID, grade level and GPA.
   ------------------------------------------------------------------------- */
static void write_student_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    /* The student's name and grade details live in two tables: user holds the
       name, student holds the grade level and GPA. The join pairs the two
       rows that share the same id. The role test makes sure this id really
       belongs to a student. */
    const char *sql =
        "SELECT user.id, user.username, user.first_name, user.last_name, "
        "       student.grade_level, student.gpa "
        "FROM user "
        "INNER JOIN student ON user.id = student.id "
        "WHERE user.id = ? AND user.role = 'student';";

    fputs("  student: {\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        /* The statement would not compile. Close the section and stop. */
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, student_id);

    if (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    userId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));
        fputs(",\n", out);

        fputs("    username: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));
        fputs(",\n", out);

        fputs("    firstName: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));
        fputs(",\n", out);

        fputs("    lastName: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 3));
        fputs(",\n", out);

        fputs("    gradeLevel: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 4));
        fputs(",\n", out);

        fputs("    gpa: ", out);
        fprintf(out, "%.2f\n", sqlite3_column_double(statement, 5));
    }

    /* Every prepared statement has to be finalised, or the memory leaks. */
    sqlite3_finalize(statement);
    fputs("  },\n", out);
}

/* -------------------------------------------------------------------------
   write_courses_section
   Writes the courses this student is enrolled in.
   ------------------------------------------------------------------------- */
static void write_courses_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    /* enrollment says who is on which course; course holds the course name.
       The join gives us the name for each enrolment of this student. */
    const char *sql =
        "SELECT course.id, course.course_name, enrollment.status "
        "FROM enrollment "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ?;";

    fputs("  courses: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, student_id);

    /* sqlite3_step returns one row at a time until there are no rows left. */
    while (sqlite3_step(statement) == SQLITE_ROW) {
        int status;

        fputs("    { courseId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        /* status 1 means the enrolment is active. JavaScript wants true or
           false, so it is turned into one of those words here. */
        status = sqlite3_column_int(statement, 2);
        fputs(", active: ", out);
        fputs(status ? "true" : "false", out);

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_grades_section
   Writes the score this student has in each course.
   ------------------------------------------------------------------------- */
static void write_grades_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT course.course_name, grade.score "
        "FROM grade "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE grade.student_id = ?;";

    fputs("  grades: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, student_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 0));
        fputs(", score: ", out);
        fprintf(out, "%.1f },\n", sqlite3_column_double(statement, 1));
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_attendance_section
   Writes, for each course, whether the student was in class.
   ------------------------------------------------------------------------- */
static void write_attendance_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    /* present is 1 when the student was there and 0 when they were not. */
    const char *sql =
        "SELECT course.course_name, attendance.present "
        "FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE attendance.student_id = ?;";

    fputs("  attendance: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, student_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        int present;

        fputs("    { course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 0));

        present = sqlite3_column_int(statement, 1);
        fputs(", status: ", out);
        write_js_quoted(out, (const unsigned char *)(present ? "Present" : "Absent"));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_assignments_section
   Writes the student's assignments, the nearest due date first.
   ------------------------------------------------------------------------- */
static void write_assignments_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    /* Only courses the student is enrolled on appear, and only the active
       enrolments (status 1). ORDER BY due_date puts the next deadline at the
       top of the list. */
    const char *sql =
        "SELECT course.course_name, assignment.assignment_name, assignment.due_date "
        "FROM assignment "
        "INNER JOIN course ON assignment.course_id = course.id "
        "INNER JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? AND enrollment.status = 1 "
        "ORDER BY assignment.due_date ASC;";

    fputs("  assignments: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, student_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 0));

        fputs(", assignmentName: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(", dueDate: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_announcements_section
   Writes the school announcements. Everyone sees these, so no id is needed.
   ------------------------------------------------------------------------- */
static void write_announcements_section(FILE *out, sqlite3 *db) {
    sqlite3_stmt *statement = NULL;

    /* An announcement may belong to one course, or to the whole school. In
       that second case its course_id is empty, so LEFT JOIN keeps the row and
       the course name comes back empty - the code below shows "General". */
    const char *sql =
        "SELECT course.course_name, announcement.title, announcement.message, "
        "       announcement.announcement_date "
        "FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC;";

    fputs("  announcements: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char *course = sqlite3_column_text(statement, 0);

        fputs("    { course: ", out);
        if (course == NULL) {
            write_js_quoted(out, (const unsigned char *)"General");
        } else {
            write_js_quoted(out, course);
        }

        fputs(", title: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(", message: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));

        fputs(", date: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 3));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   update_student_frontend_state
   The one function the server calls. It receives the signed-in student's id,
   opens the database and the output file, writes every section, then closes
   both.
   ------------------------------------------------------------------------- */
void update_student_frontend_state(int student_user_id) {
    sqlite3 *db = NULL;
    FILE *out = NULL;

    /* Step 1: open the database. Every function below uses this same
       connection, so it is opened once here. */
    if (sqlite3_open("academia.db", &db) != SQLITE_OK) {
        printf("[Student Module] Cannot open academia.db\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return;
    }

    /* Step 2: open the file the portal reads. If the usual place is missing
       (for example when the program is run from another folder) it tries a
       second location before giving up. */
    out = fopen("FrontEnd/JS/student_states.js", "w");
    if (out == NULL) {
        out = fopen("BackEnd/Academia Student/student_states.js", "w");
    }
    if (out == NULL) {
        printf("[Student Module] Cannot open student_states.js for writing.\n");
        sqlite3_close(db);
        return;
    }

    /* Step 3: write the file. The line below declares one JavaScript object
       called studentState; each section function adds one field to it. */
    fputs("// Auto-generated by the Academia C backend. Do not edit by hand.\n", out);
    fputs("const studentState = {\n", out);

    write_student_section(out, db, student_user_id);
    write_courses_section(out, db, student_user_id);
    write_grades_section(out, db, student_user_id);
    write_attendance_section(out, db, student_user_id);
    write_assignments_section(out, db, student_user_id);
    write_announcements_section(out, db);

    /* success is true when the student was found in the database. */
    fputs("  success: true\n", out);
    fputs("};\n", out);

    /* Step 4: close both. Leaving either open keeps the file locked. */
    fclose(out);
    sqlite3_close(db);

    printf("[Student Module] student_states.js written for student %d\n", student_user_id);
}
