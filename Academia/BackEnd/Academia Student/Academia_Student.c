/*
Harshul Gupta, Lawrence Mao, Seungho Kim, Dmitrii Buianov
CS50 C Project
Academia - Student module

Builds the data for the student dashboard. After the server checks the login
token, it calls update_student_frontend_state(), which reads SQLite and writes
student_states.js (profile, courses, grades, attendance, assignments, news).
The student id comes from the session, so a student only sees their own data.

Known limits: text is not escaped in the output file, announcements are
school-wide, and the output path is relative to the project root.
*/

#include "Academia_Student.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

/*
 write_list
 ----------
 Runs ONE SELECT and writes the result as a property holding a JS array.

 Parameters
   out         the already-open student_states.js
   db          an open SQLite connection
   name        the property name to write ("courses", "grades", ...)
   sql         a SELECT with EXACTLY ONE `?` placeholder
   student_id  the integer bound to that placeholder

 How it works
   1. Writes "  <name>: [".
   2. Prepares the statement. If preparing fails, it closes the array and
      returns, so the page receives an empty list instead of broken JavaScript.
   3. Binds student_id with sqlite3_bind_int. Binding (instead of building the
      SQL with sprintf) means the id can never inject SQL.
   4. For every row it writes { column: value, ... }. The property names are the
      `AS` aliases used in each query, so the SQL decides the JS field names.
      Value type handling:
         SQLITE_FLOAT   -> printed with one decimal ("%.1f")
         SQLITE_INTEGER -> printed as an int ("%d")
         anything else  -> printed as a quoted string ("" when NULL)
   3. Finalizes the statement and writes "],".
*/
static void write_list(FILE *out, sqlite3 *db, const char *name, const char *sql, int student_id) {
    sqlite3_stmt *row = NULL;

    fprintf(out, "  %s: [\n", name);

    if (sqlite3_prepare_v2(db, sql, -1, &row, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }
    sqlite3_bind_int(row, 1, student_id);

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
 update_student_frontend_state
 -----------------------------
 Called by the server for GET /api/student-data (role "student" only).

 Parameter
   student_user_id   the logged in user's id (user.id, which equals student.id)

 Returns
   1  the student row was found and the file was written
   0  the database or the output file could not be opened, or no such student

 Steps
   1. Open the database (open_database from Database.h).
   2. Open student_states.js for writing.
   3. Write "const studentState = {".
   4. "student" block: user JOIN student for this id. Every column is written as
      a quoted string, plus userId as a number. Sets `found`.
   5. Write the five lists with write_list:
        courses       enrollment JOIN course        -> course, status
        grades        grade JOIN course             -> course, score
        attendance    attendance JOIN course        -> course, present
        assignments   assignment JOIN course JOIN enrollment, ordered by due date
                                                    -> course, assignmentName, dueDate
        announcements every announcement, newest first
                                                    -> title, message, date
      (The announcements query has no placeholder-dependent filter; 0 is bound
       only because write_list always binds one value.)
   6. Write "success: true|false" and close the object.
   7. Close the file and the database, log a line, return `found`.
*/
int update_student_frontend_state(int student_user_id) {
    sqlite3 *db = open_database();
    sqlite3_stmt *row = NULL;
    FILE *out = NULL;
    int found = 0;

    if (!db) {
        printf("[Student] Cannot open the database.\n");
        return 0;
    }

    out = fopen("Academia/FrontEnd/JS/student_states.js", "w");
    if (!out) {
        printf("[Student] Cannot write student_states.js\n");
        close_database(db);
        return 0;
    }

    fputs("// Written by the Academia C backend.\nconst studentState = {\n", out);

    /* ---- student profile ---- */
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

    /* ---- enrolled courses ---- */
    write_list(out, db, "courses",
        "SELECT course.course_name AS course, enrollment.status AS status FROM enrollment "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? ORDER BY course.id;", student_user_id);

    /* ---- grades ---- */
    write_list(out, db, "grades",
        "SELECT course.course_name AS course, grade.score AS score FROM grade "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE grade.student_id = ? ORDER BY course.id;", student_user_id);

    /* ---- attendance ---- */
    write_list(out, db, "attendance",
        "SELECT course.course_name AS course, attendance.present AS present FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE attendance.student_id = ? ORDER BY course.id;", student_user_id);

    /* ---- assignments of the courses the student is enrolled in ---- */
    write_list(out, db, "assignments",
        "SELECT course.course_name AS course, assignment.assignment_name AS assignmentName, "
        "assignment.due_date AS dueDate FROM assignment "
        "INNER JOIN course ON assignment.course_id = course.id "
        "INNER JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? ORDER BY assignment.due_date;", student_user_id);

    /* ---- announcements (school wide) ---- */
    write_list(out, db, "announcements",
        "SELECT announcement.title AS title, announcement.message AS message, "
        "announcement.announcement_date AS date FROM announcement "
        "ORDER BY announcement.id DESC;", 0);

    fprintf(out, "  success: %s\n};\n", found ? "true" : "false");

    fclose(out);
    close_database(db);
    printf("[Student] student_states.js written for student %d\n", student_user_id);
    return found;
}