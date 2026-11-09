/*
   ==========================================================================
   ACADEMIA - STAFF MODULE  (Academia_Staff.c)
   Writes staff_states.js, the file the Staff Portal reads
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================

      WHAT THIS FILE DOES
   -------------------
   When a member of staff signs in, the server calls
   update_staff_frontend_state(id). This file writes that teacher's profile,
   the courses they teach, the students enrolled in those courses, every
   grade and attendance record for those courses, and the announcements into
   a JavaScript file:

       FrontEnd/JS/staff_states.js

   The Staff Portal reads that file, so a teacher sees only their own classes.

   HOW IT IS ORGANISED
   -------------------
   1. write_js_text() / write_js_quoted() - helpers that copy text into the file
   2. open_output_file()   - creates staff_states.js
   3. one function per section of the state file:
        write_staff_section()          - name, ID, phone
        write_courses_section()        - the courses this teacher runs
        write_enrollments_section()    - who is enrolled in those courses
        write_grades_section()         - grades recorded in those courses
        write_attendance_section()     - attendance for those courses
        write_announcements_section()  - notices, newest first
   4. update_staff_frontend_state() - the function the server calls

   Note: the four questions about courses, enrollments, grades and attendance
   all filter on the same value, the teacher's id, so each function is handed
   that id and uses it in its own WHERE clause.

*/

#include "Academia_Staff.h"

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
   write_staff_section
   Writes the teacher's own details.
   ------------------------------------------------------------------------- */
static void write_staff_section(FILE *out, sqlite3 *db, int staff_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT user.id, user.username, user.first_name, user.last_name, staff.phone "
        "FROM user "
        "INNER JOIN staff ON user.id = staff.id "
        "WHERE user.id = ? AND user.role = 'staff';";

    fputs("  staff: {\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, staff_id);

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

        fputs("    phone: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 4));
        fputs("\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  },\n", out);
}

/* -------------------------------------------------------------------------
   write_courses_section
   Writes the courses this teacher runs.
   ------------------------------------------------------------------------- */
static void write_courses_section(FILE *out, sqlite3 *db, int staff_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql = "SELECT id, course_name FROM course WHERE staff_id = ?;";

    fputs("  courses: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, staff_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { courseId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_enrollments_section
   Writes one row per student on each of this teacher's courses.
   ------------------------------------------------------------------------- */
static void write_enrollments_section(FILE *out, sqlite3 *db, int staff_id) {
    sqlite3_stmt *statement = NULL;

    /* The two pipe symbols join two text columns into one full name, so the
       C code receives "Alex Rivera" instead of two separate words. */
    const char *sql =
        "SELECT DISTINCT user.id, "
        "       user.first_name || ' ' || user.last_name, "
        "       course.id, course.course_name "
        "FROM enrollment "
        "INNER JOIN user ON enrollment.student_id = user.id "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE course.staff_id = ?;";

    fputs("  enrollments: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, staff_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { studentId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", student: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(", courseId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 2));

        fputs(", course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 3));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_grades_section
   Writes every grade recorded in this teacher's courses, so the grades table
   on the staff page shows the whole class.
   ------------------------------------------------------------------------- */
static void write_grades_section(FILE *out, sqlite3 *db, int staff_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT grade.id, "
        "       user.first_name || ' ' || user.last_name, "
        "       course.course_name, grade.score "
        "FROM grade "
        "INNER JOIN user ON grade.student_id = user.id "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE course.staff_id = ?;";

    fputs("  grades: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, staff_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { gradeId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", student: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(", course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));

        fputs(", score: ", out);
        fprintf(out, "%.1f },\n", sqlite3_column_double(statement, 3));
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_attendance_section
   Writes every attendance record for this teacher's courses.
   ------------------------------------------------------------------------- */
static void write_attendance_section(FILE *out, sqlite3 *db, int staff_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT attendance.id, "
        "       user.first_name || ' ' || user.last_name, "
        "       course.course_name, attendance.present "
        "FROM attendance "
        "INNER JOIN user ON attendance.student_id = user.id "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE course.staff_id = ?;";

    fputs("  attendance: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, staff_id);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    { attendanceId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", student: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));

        fputs(", course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));

        /* The JavaScript expects a number here: 1 for present, 0 for absent. */
        fputs(", present: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 3));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_announcements_section
   Writes the announcements, newest first.
   ------------------------------------------------------------------------- */
static void write_announcements_section(FILE *out, sqlite3 *db) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT announcement.id, course.course_name, announcement.title, "
        "       announcement.message, announcement.announcement_date "
        "FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC;";

    fputs("  announcements: [\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char *course = sqlite3_column_text(statement, 1);

        fputs("    { announcementId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));

        fputs(", course: ", out);
        if (course == NULL) {
            write_js_quoted(out, (const unsigned char *)"General");
        } else {
            write_js_quoted(out, course);
        }

        fputs(", title: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));

        fputs(", message: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 3));

        fputs(", date: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 4));

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   update_staff_frontend_state
   The function the server calls. It opens the database and the output file,
   writes every section and then closes both.
   ------------------------------------------------------------------------- */
void update_staff_frontend_state(int staff_user_id) {
    sqlite3 *db = NULL;
    FILE *out = NULL;

    /* Step 1: open the database. */
    if (sqlite3_open("academia.db", &db) != SQLITE_OK) {
        printf("[Staff Module] Cannot open academia.db\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return;
    }

    /* Step 2: open the file the portal reads. */
    out = fopen("FrontEnd/JS/staff_states.js", "w");
    if (out == NULL) {
        out = fopen("BackEnd/Academia Staff/staff_states.js", "w");
    }
    if (out == NULL) {
        printf("[Staff Module] Cannot open staff_states.js for writing.\n");
        sqlite3_close(db);
        return;
    }

    fputs("// Auto-generated by the Academia C backend. Do not edit by hand.\n", out);
    fputs("const staffState = {\n", out);

    write_staff_section(out, db, staff_user_id);
    write_courses_section(out, db, staff_user_id);
    write_enrollments_section(out, db, staff_user_id);
    write_grades_section(out, db, staff_user_id);
    write_attendance_section(out, db, staff_user_id);
    write_announcements_section(out, db);

    fputs("  success: true\n", out);
    fputs("};\n", out);

    /* Step 3: close both. */
    fclose(out);
    sqlite3_close(db);

    printf("[Staff Module] staff_states.js written for staff %d\n", staff_user_id);
}