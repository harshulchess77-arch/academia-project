/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia - Staff Module

Writes staff_states.js, the file the Faculty Portal reads.

The version in the repository still had git conflict markers in it
(<<<<<<< HEAD ... >>>>>>> bdb42e4), which is a syntax error, so it never
compiled and the backend would not build.
*/

#include "Academia_Staff.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

/* Runs one SELECT and writes it out as a JavaScript array. */
void write_list(FILE *out, sqlite3 *db, const char *name, const char *sql, int staff_id) {
    sqlite3_stmt *row = NULL;

    fprintf(out, "  %s: [\n", name);

    if (sqlite3_prepare_v2(db, sql, -1, &row, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }
    sqlite3_bind_int(row, 1, staff_id);

    while (sqlite3_step(row) == SQLITE_ROW) {
        int i;
        fputs("    {", out);
        for (i = 0; i < sqlite3_column_count(row); i++) {
            const char *text = (const char *)sqlite3_column_text(row, i);

            fprintf(out, "%s%s: ", i ? ", " : "", sqlite3_column_name(row, i));

            if (sqlite3_column_type(row, i) == SQLITE_FLOAT) {
                fprintf(out, "%.1f", sqlite3_column_double(row, i));
            } else if (sqlite3_column_type(row, i) == SQLITE_INTEGER) {
                fprintf(out, "%d", sqlite3_column_int(row, i));
            } else {
                fprintf(out, "\"%s\"", text == NULL ? "" : text);
            }
        }
        fputs(" },\n", out);
    }

    sqlite3_finalize(row);
    fputs("  ],\n", out);
}

/* Called by the server. Returns 1 if the staff member was found. */
int update_staff_frontend_state(int staff_user_id) {
    sqlite3 *db = open_database();
    sqlite3_stmt *row = NULL;
    FILE *out = NULL;
    int found = 0;

    if (!db) {
        printf("[Staff] Cannot open the database.\n");
        return 0;
    }

    out = fopen("Academia/FrontEnd/JS/staff_states.js", "w");
    if (!out) {
        printf("[Staff] Cannot write staff_states.js\n");
        close_database(db);
        return 0;
    }

    fputs("// Written by the Academia C backend.\nconst staffState = {\n", out);

    fputs("  staff: {\n", out);
    if (sqlite3_prepare_v2(db,
            "SELECT user.username AS username, user.first_name AS firstName, "
            "user.last_name AS lastName, staff.phone AS phone FROM user "
            "INNER JOIN staff ON user.id = staff.id WHERE user.id = ?;",
            -1, &row, NULL) == SQLITE_OK) {
        sqlite3_bind_int(row, 1, staff_user_id);
        if (sqlite3_step(row) == SQLITE_ROW) {
            int i, n = sqlite3_column_count(row);
            found = 1;
            fprintf(out, "    userId: %d,\n", staff_user_id);
            for (i = 0; i < n; i++) {
                const char *text = (const char *)sqlite3_column_text(row, i);
                fprintf(out, "    %s: \"%s\"%s\n", sqlite3_column_name(row, i),
                        text == NULL ? "" : text, i == n - 1 ? "" : ",");
            }
        }
        sqlite3_finalize(row);
    }
    fputs("  },\n", out);

    write_list(out, db, "courses",
        "SELECT course.id AS courseId, course.course_name AS course, "
        "(SELECT COUNT(*) FROM enrollment WHERE enrollment.course_id = course.id "
        "AND enrollment.status = 1) AS students FROM course "
        "WHERE course.staff_id = ? ORDER BY course.id;", staff_user_id);

    write_list(out, db, "students",
        "SELECT user.id AS studentId, user.first_name || ' ' || user.last_name AS name, "
        "user.username AS username, course.course_name AS course, "
        "(SELECT score FROM grade WHERE grade.student_id = user.id "
        "AND grade.course_id = course.id) AS score FROM enrollment "
        "INNER JOIN student ON enrollment.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE course.staff_id = ? ORDER BY user.last_name;", staff_user_id);

    write_list(out, db, "attendance",
        "SELECT course.course_name AS course, "
        "user.first_name || ' ' || user.last_name AS student, "
        "attendance.present AS present FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "INNER JOIN student ON attendance.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "WHERE course.staff_id = ? ORDER BY course.id;", staff_user_id);

    write_list(out, db, "announcements",
        "SELECT course.course_name AS course, announcement.title AS title, "
        "announcement.message AS message, announcement.announcement_date AS date "
        "FROM announcement INNER JOIN course ON announcement.course_id = course.id "
        "WHERE course.staff_id = ? ORDER BY announcement.id DESC;", staff_user_id);

    fprintf(out, "  success: %s,\n  staffId: %d\n};\n",
            found ? "true" : "false", staff_user_id);

    fclose(out);
    close_database(db);
    printf("[Staff] staff_states.js written for staff %d\n", staff_user_id);
    return found;
}
