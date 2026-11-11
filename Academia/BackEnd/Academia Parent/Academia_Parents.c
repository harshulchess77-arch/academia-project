/*
================================================================================
 Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
 CS50 C Project
 Academia - Parent Module  (Academia_Parents.c)
================================================================================

 FULL DOCUMENTATION
 ------------------

 1. PURPOSE
    Builds everything the Parent Portal needs. The server calls
    update_parent_frontend_state() after checking the login token. The function
    reads SQLite and writes
        Academia/FrontEnd/JS/parents_states.js
    which the server sends back for GET /api/parent-data.

 2. REQUEST FLOW
      Browser (Academia.js, initParent)
        -> GET /api/parent-data   with header  X-Auth-Token: <token>
      Academia_Server.c
        -> get_token(), auth_check()   (no token -> 401, wrong role -> 403)
        -> update_parent_frontend_state(user_id)        <-- THIS FILE
        -> send_file("Academia/FrontEnd/JS/parents_states.js")
      Browser
        -> evaluates the text and gets `parentState`

    SECURITY NOTE: the parent id comes from the session. A parent only sees
    students whose student.parent_id equals that id, so one parent can never
    read another family's grades.

 3. HOW A PARENT IS LINKED TO A CHILD
    student.parent_id -> parent.id. One parent can have many children; each
    student has at most one linked parent. In the seed data parent1 (id 3) is
    linked to student1 (Alex Rivera); student2 (Sophia Chen) has no parent.

 4. DATABASE TABLES READ (see Academia.sql)
      user        id, username, first_name, last_name
      parent      id (= user.id), phone
      student     id, parent_id, grade_level, gpa
      course      id, course_name
      grade       student_id, course_id, score
      attendance  student_id, course_id, present        (1 present, 0 absent)
      announcement title, message, announcement_date

 5. OUTPUT FILE SHAPE (parents_states.js)
      const parentState = {
        parent: { userId: 3, username: "parent1", firstName: "Marcus",
                  lastName: "Rivera", phone: "(555) 234-5678" },
        children:   [ { name: "Alex Rivera", username: "student1",
                        gradeLevel: 11, gpa: 3.9 } ],
        grades:     [ { student: "Alex Rivera", course: "...", score: 97.0 } ],
        attendance: [ { student: "Alex Rivera", course: "...", present: 1 } ],
        announcements: [ { title: "...", message: "...", date: "..." } ],
        success: true,
        parentId: 3
      };

 6. FUNCTIONS
      static void write_list(...)          shared "SELECT -> JS array" writer
      int update_parent_frontend_state()   public entry point, called by server

 7. KNOWN LIMITS
      - gpa is a REAL column, so write_list prints it with ONE decimal
        (3.85 becomes 3.9). Change "%.1f" to "%.2f" in write_list, or select
        the gpa as text, if you want two decimals.
      - Strings are not escaped (a double quote in the data breaks the file).
      - Relative output path: start the server from the repository root.
      - Announcements are school wide, not filtered to the children's courses.
      - Shared output file; safe only while the server is single threaded.
      - Plain text passwords (see Academia_Auth.c).

 8. CHANGES FROM THE VERSION IN THE REPOSITORY
      write_list() is now `static` so it no longer collides with the functions
      of the same name in the Student and Staff modules. Logic is unchanged.
================================================================================
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
