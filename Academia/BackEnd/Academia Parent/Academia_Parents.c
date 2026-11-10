/*
   ==========================================================================
   ACADEMIA - PARENT MODULE  (Academia_Parents.c)
   Writes parents_states.js, the file the Parent Portal reads
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================
*/

#include "Academia_Parents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsqlite/winsqlite3.h>

/* -------------------------------------------------------------------------
   write_js_text
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
            /* Carriage returns are dropped. */
        } else {
            fputc(text[i], out);
        }
    }
}

/* -------------------------------------------------------------------------
   write_js_quoted
   ------------------------------------------------------------------------- */
static void write_js_quoted(FILE *out, const unsigned char *text) {
    fputc('"', out);
    write_js_text(out, text);
    fputc('"', out);
}

/* -------------------------------------------------------------------------
   find_child_id
   A parent does not type a student id anywhere, so the code works out which
   student belongs to them from student.parent_id. Returns 0 when there is no
   match, and every caller below guards on that.
   ------------------------------------------------------------------------- */
static int find_child_id(sqlite3 *db, int parent_id) {
    sqlite3_stmt *statement = NULL;
    int child_id = 0;

    const char *sql = "SELECT id FROM student WHERE parent_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(statement, 1, parent_id);

    if (sqlite3_step(statement) == SQLITE_ROW) {
        child_id = sqlite3_column_int(statement, 0);
    }

    sqlite3_finalize(statement);
    return child_id;
}

/* -------------------------------------------------------------------------
   write_parent_section - the parent's own details.
   ------------------------------------------------------------------------- */
static void write_parent_section(FILE *out, sqlite3 *db, int parent_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT user.id, user.username, user.first_name, user.last_name, parent.phone "
        "FROM user "
        "INNER JOIN parent ON user.id = parent.id "
        "WHERE user.id = ? AND user.role = 'parent';";

    fputs("  parent: {\n", out);

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, parent_id);

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
   write_child_section - the linked student's profile.
   ------------------------------------------------------------------------- */
static void write_child_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT user.id, user.first_name, user.last_name, "
        "       student.grade_level, student.gpa "
        "FROM student "
        "INNER JOIN user ON student.id = user.id "
        "WHERE student.id = ?;";

    fputs("  student: {\n", out);

    if (child_id <= 0) {
        fputs("  },\n", out);
        return;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, child_id);

    if (sqlite3_step(statement) == SQLITE_ROW) {
        fputs("    studentId: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 0));
        fputs(",\n", out);

        fputs("    firstName: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 1));
        fputs(",\n", out);

        fputs("    lastName: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 2));
        fputs(",\n", out);

        fputs("    gradeLevel: ", out);
        fprintf(out, "%d", sqlite3_column_int(statement, 3));
        fputs(",\n", out);

        fputs("    gpa: ", out);
        fprintf(out, "%.2f\n", sqlite3_column_double(statement, 4));
    }

    sqlite3_finalize(statement);
    fputs("  },\n", out);
}

/* -------------------------------------------------------------------------
   write_grades_section - the child's score in each course.
   ------------------------------------------------------------------------- */
static void write_grades_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT course.course_name, grade.score "
        "FROM grade "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE grade.student_id = ?;";

    fputs("  grades: [\n", out);

    if (child_id <= 0) {
        fputs("  ],\n", out);
        return;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, child_id);

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
   write_attendance_section - the child's attendance record.
   ------------------------------------------------------------------------- */
static void write_attendance_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    const char *sql =
        "SELECT course.course_name, attendance.present "
        "FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE attendance.student_id = ?;";

    fputs("  attendance: [\n", out);

    if (child_id <= 0) {
        fputs("  ],\n", out);
        return;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, child_id);

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
   write_announcements_section - the school announcements, newest first.
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
   update_parent_frontend_state
   The function the server calls.
   ------------------------------------------------------------------------- */
void update_parent_frontend_state(int parent_user_id) {
    sqlite3 *db = NULL;
    FILE *out = NULL;
    int child_id;

    if (sqlite3_open("academia.db", &db) != SQLITE_OK) {
        printf("[Parent Module] Cannot open academia.db\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return;
    }

    out = fopen("FrontEnd/JS/parents_states.js", "w");
    if (out == NULL) {
        out = fopen("BackEnd/Academia Parent/parents_states.js", "w");
    }
    if (out == NULL) {
        printf("[Parent Module] Cannot open parents_states.js for writing.\n");
        sqlite3_close(db);
        return;
    }

    /* Find the child once, then hand the id to the sections that need it.
       Asking four times would be slower and repeat the same SQL. */
    child_id = find_child_id(db, parent_user_id);

    fputs("// Auto-generated by the Academia C backend. Do not edit by hand.\n", out);
    fputs("const parentState = {\n", out);

    write_parent_section(out, db, parent_user_id);
    write_child_section(out, db, child_id);
    write_grades_section(out, db, child_id);
    write_attendance_section(out, db, child_id);
    write_announcements_section(out, db);

    fputs("  success: true\n", out);
    fputs("};\n", out);

    fclose(out);
    sqlite3_close(db);

    printf("[Parent Module] parents_states.js written for parent %d (child %d)\n",
           parent_user_id, child_id);
}