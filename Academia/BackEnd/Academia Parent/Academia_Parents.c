/*
Harshul Gupta, Dmitrii Buianov, Lawrence Mao, Seungho Kim
November 12, 2026
Academia
CS50 3rd period
*/

#include "Academia_Parents.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

/*
 write_list
 ----------
 Runs ONE SELECT and writes the rows as a property holding a JS array.

 Parameters
   out        the already-open parents_states.js
   db         an open SQLite connection
   name       property name to write ("children", "grades", ...)
   sql        a SELECT with EXACTLY ONE `?` placeholder
   parent_id  the integer bound to that placeholder

 How it works
   1. Writes "  <name>: [".
   2. If the statement cannot be prepared, closes the array and returns, so the
      page gets an empty list instead of broken JavaScript.
   3. Binds parent_id with sqlite3_bind_int (no SQL injection possible).
   4. For each row writes { alias: value, ... }; the SQL `AS` names become the
      JS property names. FLOAT -> "%.1f", INTEGER -> "%d", else quoted string
      ("" for NULL).
   5. Finalizes and writes "],".
*/
static void write_list(FILE *out, sqlite3 *db, const char *name, const char *sql, int parent_id) {
    sqlite3_stmt *row = NULL;

    fprintf(out, "  %s: [\n", name);

    if (sqlite3_prepare_v2(db, sql, -1, &row, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }
    sqlite3_bind_int(row, 1, parent_id);

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

/*
 update_parent_frontend_state
 ----------------------------
 Called by the server for GET /api/parent-data (role "parent" only).

 Parameter
   parent_user_id   the logged in user's id (user.id, which equals parent.id)

 Returns
   1  parent found and file written
   0  database/file could not be opened, or no such parent

 Steps
   1. Open the database.
   2. Open parents_states.js for writing.
   3. Write "const parentState = {".
   4. "parent" block: user JOIN parent for this id (username, firstName,
      lastName, phone as strings, plus userId). Sets `found`.
   5. Lists, each bound to the parent's id:
        children      student JOIN user WHERE student.parent_id = ?
                      -> name, username, gradeLevel, gpa
        grades        grade JOIN student JOIN user JOIN course
                      WHERE student.parent_id = ?  -> student, course, score
        attendance    attendance joined the same way
                      -> student, course, present
        announcements every announcement, newest first -> title, message, date
      `name` and `student` are built in SQL as first_name || ' ' || last_name,
      so the page can tell several children apart.
   6. Write "success" and "parentId", close the object.
   7. Close the file and database, log a line, return `found`.
*/
int update_parent_frontend_state(int parent_user_id) {
    sqlite3 *db = open_database();
    sqlite3_stmt *row = NULL;
    FILE *out = NULL;
    int found = 0;

    if (!db) {
        printf("[Parent] Cannot open the database.\n");
        return 0;
    }

    out = fopen("Academia/FrontEnd/JS/parents_states.js", "w");
    if (!out) {
        printf("[Parent] Cannot write parents_states.js\n");
        close_database(db);
        return 0;
    }

    fputs("// Written by the Academia C backend.\nconst parentState = {\n", out);

    /* ---- parent profile ---- */
    fputs("  parent: {\n", out);
    if (sqlite3_prepare_v2(db,
            "SELECT user.username AS username, user.first_name AS firstName, "
            "user.last_name AS lastName, parent.phone AS phone FROM user "
            "INNER JOIN parent ON user.id = parent.id WHERE user.id = ?;",
            -1, &row, NULL) == SQLITE_OK) {
        sqlite3_bind_int(row, 1, parent_user_id);
        if (sqlite3_step(row) == SQLITE_ROW) {
            int i, n = sqlite3_column_count(row);
            found = 1;
            fprintf(out, "    userId: %d,\n", parent_user_id);
            for (i = 0; i < n; i++) {
                const char *text = (const char *)sqlite3_column_text(row, i);
                fprintf(out, "    %s: \"%s\"%s\n", sqlite3_column_name(row, i),
                        text == NULL ? "" : text, i == n - 1 ? "" : ",");
            }
        }
        sqlite3_finalize(row);
    }
    fputs("  },\n", out);

    /* ---- linked children ---- */
    write_list(out, db, "children",
        "SELECT user.first_name || ' ' || user.last_name AS name, "
        "user.username AS username, student.grade_level AS gradeLevel, "
        "student.gpa AS gpa FROM student INNER JOIN user ON student.id = user.id "
        "WHERE student.parent_id = ? ORDER BY user.id;", parent_user_id);

    /* ---- children's grades ---- */
    write_list(out, db, "grades",
        "SELECT user.first_name || ' ' || user.last_name AS student, "
        "course.course_name AS course, grade.score AS score FROM grade "
        "INNER JOIN student ON grade.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE student.parent_id = ? ORDER BY user.id, course.id;", parent_user_id);

    /* ---- children's attendance ---- */
    write_list(out, db, "attendance",
        "SELECT user.first_name || ' ' || user.last_name AS student, "
        "course.course_name AS course, attendance.present AS present FROM attendance "
        "INNER JOIN student ON attendance.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE student.parent_id = ? ORDER BY user.id, course.id;", parent_user_id);

    /* ---- announcements (school wide) ---- */
    write_list(out, db, "announcements",
        "SELECT announcement.title AS title, announcement.message AS message, "
        "announcement.announcement_date AS date FROM announcement "
        "ORDER BY announcement.id DESC;", 0);

    fprintf(out, "  success: %s,\n  parentId: %d\n};\n",
            found ? "true" : "false", parent_user_id);

    fclose(out);
    close_database(db);
    printf("[Parent] parents_states.js written for parent %d\n", parent_user_id);
    return found;
}