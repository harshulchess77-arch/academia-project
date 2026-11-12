/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/

#include "Academia_Student.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

/* write_list runs one query and writes the rows as a JS array */
static void write_list(FILE *out, sqlite3 *db, const char *name, const char *sql, int student_id) {
    sqlite3_stmt *row = NULL;

    fprintf(out, "  %s: [\n", name);

    /* if the query will not prepare, close the array so the page still parses */
    if (sqlite3_prepare_v2(db, sql, -1, &row, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }
    /* bind the id so the query can never be injected */
    sqlite3_bind_int(row, 1, student_id);

    /* one line of JavaScript per row */
    while (sqlite3_step(row) == SQLITE_ROW) {
        int i;
        fputs("    {", out);
        for (i = 0; i < sqlite3_column_count(row); i++) {
            const char *text = (const char *)sqlite3_column_text(row, i);

            fprintf(out, "%s%s: ", i ? ", " : "", sqlite3_column_name(row, i));

            /* numbers go in as numbers, everything else as a quoted string */
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

/* update_student_frontend_state builds student_states.js for the student portal */
int update_student_frontend_state(int student_user_id) {
    sqlite3 *db = open_database();
    sqlite3_stmt *row = NULL;
    /* found stays 0 until the student row is seen */
    FILE *out = NULL;
    int found = 0;

    if (!db) {
        printf("[Student] Cannot open the database.\n");
        return 0;
    }

    /* the portal reads this file */
    out = fopen("Academia/FrontEnd/JS/student_states.js", "w");
    if (!out) {
        printf("[Student] Cannot write student_states.js\n");
        close_database(db);
        return 0;
    }

    fputs("// Written by the Academia C backend.\nconst studentState = {\n", out);

    /* the profile block is written column by column */

    /* student profile */
    fputs("  student: {\n", out);
    if (sqlite3_prepare_v2(db,
            "SELECT user.username AS username, user.first_name AS firstName, "
            "user.last_name AS lastName, student.grade_level AS gradeLevel, "
            "student.gpa AS gpa FROM user "
            "INNER JOIN student ON user.id = student.id WHERE user.id = ?;",
            -1, &row, NULL) == SQLITE_OK) {
        sqlite3_bind_int(row, 1, student_user_id);
        if (sqlite3_step(row) == SQLITE_ROW) {
            int i, n = sqlite3_column_count(row);
            found = 1;
            fprintf(out, "    userId: %d,\n", student_user_id);
            for (i = 0; i < n; i++) {
                const char *text = (const char *)sqlite3_column_text(row, i);
                fprintf(out, "    %s: \"%s\"%s\n", sqlite3_column_name(row, i),
                        text == NULL ? "" : text, i == n - 1 ? "" : ",");
            }
        }
        sqlite3_finalize(row);
    }
    fputs("  },\n", out);

    /* enrolled courses */
    write_list(out, db, "courses",
        "SELECT course.course_name AS course, enrollment.status AS status FROM enrollment "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? ORDER BY course.id;", student_user_id);

    /* grades */
    write_list(out, db, "grades",
        "SELECT course.course_name AS course, grade.score AS score FROM grade "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE grade.student_id = ? ORDER BY course.id;", student_user_id);

    /* attendance */
    write_list(out, db, "attendance",
        "SELECT course.course_name AS course, attendance.present AS present FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE attendance.student_id = ? ORDER BY course.id;", student_user_id);

    /* assignments of the courses the student is enrolled in */
    write_list(out, db, "assignments",
        "SELECT course.course_name AS course, assignment.assignment_name AS assignmentName, "
        "assignment.due_date AS dueDate FROM assignment "
        "INNER JOIN course ON assignment.course_id = course.id "
        "INNER JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? ORDER BY assignment.due_date;", student_user_id);

    /* announcements (school wide) */
    write_list(out, db, "announcements",
        "SELECT announcement.title AS title, announcement.message AS message, "
        "announcement.announcement_date AS date FROM announcement "
        "ORDER BY announcement.id DESC;", 0);

    /* the page checks this flag */
    fprintf(out, "  success: %s\n};\n", found ? "true" : "false");

    /* done, close both */
    fclose(out);
    close_database(db);
    printf("[Student] student_states.js written for student %d\n", student_user_id);
    return found;
}