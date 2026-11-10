/*
   ==========================================================================
   ACADEMIA - STUDENT MODULE  (Academia_Student.c)
   Writes student_states.js, the file the Student Portal reads
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================
*/

#include "Academia_Student.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsqlite/winsqlite3.h>

/* -------------------------------------------------------------------------
   write_js_text
   Copies one piece of text into the output file, adding a backslash before
   any character that would otherwise break a JavaScript string.
   ------------------------------------------------------------------------- */
static void write_js_text(FILE *out, const unsigned char *text) {
    int i;

    if (text == NULL) {
        return;
    }

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\\' || text[i] == '"') {
            fputc('\\', out);
            fputc(text[i], out);
        } else if (text[i] == '\n') {
            fputs("\\n", out);
        } else if (text[i] == '\t') {
            fputs("\\t", out);
        } else if (text[i] == '\r') {
            /* Carriage returns are dropped; they upset JavaScript. */
        } else {
            fputc(text[i], out);
        }
    }
}

/* -------------------------------------------------------------------------
   write_js_quoted
   Writes a piece of text surrounded by quotation marks.
   ------------------------------------------------------------------------- */
static void write_js_quoted(FILE *out, const unsigned char *text) {
    fputc('"', out);
    write_js_text(out, text);
    fputc('"', out);
}

/* -------------------------------------------------------------------------
   write_student_section
   The student's own details: name, username, ID, grade level and GPA.
   ------------------------------------------------------------------------- */
static void write_student_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT user.id, user.username, user.first_name, user.last_name, "
        "       student.grade_level, student.gpa "
        "FROM user "
        "INNER JOIN student ON user.id = student.id "
        "WHERE user.id = ? AND user.role = 'student';";

    fputs("  student: {\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
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

    sqlite3_finalize(statement);
    fputs("  },\n", out);
}

/* -------------------------------------------------------------------------
   write_courses_section
   The courses this student is enrolled in.
   ------------------------------------------------------------------------- */
static void write_courses_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

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
   The score this student has in each course.
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
   For each course, whether the student was in class.
   ------------------------------------------------------------------------- */
static void write_attendance_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

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
   The student's assignments, the nearest due date first.
   ------------------------------------------------------------------------- */
static void write_assignments_section(FILE *out, sqlite3 *db, int student_id) {
    sqlite3_stmt *statement = NULL;

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
   The school announcements. Everyone sees these, so no id is needed.
   ------------------------------------------------------------------------- */
static void write_announcements_section(FILE *out, sqlite3 *db) {
    sqlite3_stmt *statement = NULL;

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
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK) {
        printf("[Student Module] Cannot open academia.db\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return;
    }

    /* Step 2: open the file the portal reads. If the usual place is missing
       (for example when the program is run from another folder) it tries a
       second location before giving up. */
    out = fopen("Academia/FrontEnd/JS/student_states.js", "w");

    if (out == NULL) {
        printf("[Student Module] Cannot open student_states.js for writing.\n");
        sqlite3_close(db);
        return;
    }

    fputs("// Auto-generated by the Academia C backend. Do not edit by hand.\n", out);
    fputs("const studentState = {\n", out);

    write_student_section(out, db, student_user_id);
    write_courses_section(out, db, student_user_id);
    write_grades_section(out, db, student_user_id);
    write_attendance_section(out, db, student_user_id);
    write_assignments_section(out, db, student_user_id);
    write_announcements_section(out, db);

    fputs("  success: true\n", out);
    fputs("};\n", out);

    fclose(out);
    sqlite3_close(db);

    printf("[Student Module] student_states.js written for student %d\n", student_user_id);
}