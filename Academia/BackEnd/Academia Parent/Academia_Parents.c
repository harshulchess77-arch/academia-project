/*
================================================================================
 Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
 CS50 C Project
 Academia - Parent Module  (Academia_Parents.c)
================================================================================

<<<<<<< HEAD
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
=======
   WHAT THIS FILE DOES
   -------------------
   When a parent signs in, the server calls update_parent_frontend_state(id).
   This file looks up the parent, finds the child linked to that parent, and
   writes the parent's details, the child's profile, grades, attendance and
   the school announcements into a JavaScript file:

       FrontEnd/JS/parents_states.js

   The Parent Portal page reads that file, so a parent sees their own child's
   records and nothing else.

   HOW IT IS ORGANISED
   -------------------
   1. write_js_text() / write_js_quoted() - helpers that copy text into the file
   2. open_output_file()   - creates parents_states.js
   3. find_child_id()      - asks the database which student belongs to this
                             parent; the answer is used by the next sections
   4. one function per section of the state file:
        write_parent_section()        - name, username, phone
        write_child_section()         - the child's name, grade level, GPA
        write_grades_section()        - the child's score in each course
        write_attendance_section()    - the child's Present / Absent record
        write_announcements_section() - notices from the school
   5. update_parent_frontend_state() - the function the server calls
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4
*/

#include "Academia_Parents.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

<<<<<<< HEAD
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
=======
/* without this, a student called Ann "Annie" Lee would end the string
early and the browser would refuse to read the file.
------------------------------------------------------------------------- */
static void write_js_text(FILE *out, const unsigned char *text) {
    int i;
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4

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

/* -------------------------------------------------------------------------
   find_child_id
   A parent does not type a student id anywhere, so the code has to work out
   which student belongs to them. The student table keeps a parent_id column;
   this function returns the id found there, or 0 when there is no match.
   ------------------------------------------------------------------------- */
static int find_child_id(sqlite3 *db, int parent_id) {
    sqlite3_stmt *statement = NULL;
    int child_id = 0;

    // The question mark is filled with parent_id after the query is prepared.
    const char *sql = "SELECT id FROM student WHERE parent_id = ?;";

    // Convert the SQL text into a statement that SQLite can execute.
    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        return 0;
    }

    // Bind parent_id to the first question mark in the query.
    sqlite3_bind_int(statement, 1, parent_id);

    // Read the student ID if the query finds a matching row.
    if (sqlite3_step(statement) == SQLITE_ROW) {
        child_id = sqlite3_column_int(statement, 0);
    }

    // Release the prepared statement after the query is finished.
    sqlite3_finalize(statement);
    return child_id;
}

<<<<<<< HEAD
    /* ---- parent profile ---- */
=======
/* -------------------------------------------------------------------------
   write_parent_section
   Writes the parent's own details.
   ------------------------------------------------------------------------- */
static void write_parent_section(FILE *out, sqlite3 *db, int parent_id) {
    sqlite3_stmt *statement = NULL;

    /* The name comes from user, the phone number from parent. The role test
       confirms that this id belongs to a parent. */
    const char *sql =
        "SELECT user.id, user.username, user.first_name, user.last_name, parent.phone "
        "FROM user "
        "INNER JOIN parent ON user.id = parent.id "
        "WHERE user.id = ? AND user.role = 'parent';";

    // Start the parent object in the generated JavaScript file.
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4
    fputs("  parent: {\n", out);

    // Write an empty object if SQLite cannot prepare the query.
    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, parent_id);

    // Each column number matches its position in the SELECT statement.
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

    // Release the query and close the parent object.
    sqlite3_finalize(statement);
    fputs("  },\n", out);

    /* ---- linked children ---- */
    write_list(out, db, "children",
        "SELECT user.first_name || ' ' || user.last_name AS name, "
        "user.username AS username, student.grade_level AS gradeLevel, "
        "student.gpa AS gpa FROM student INNER JOIN user ON student.id = user.id "
        "WHERE student.parent_id = ? ORDER BY user.id;", parent_user_id);

<<<<<<< HEAD
    /* ---- children's grades ---- */
    write_list(out, db, "grades",
        "SELECT user.first_name || ' ' || user.last_name AS student, "
        "course.course_name AS course, grade.score AS score FROM grade "
        "INNER JOIN student ON grade.student_id = student.id "
=======
    // Join student with user because the name and school data use two tables.
    const char *sql =
        "SELECT user.id, user.first_name, user.last_name, "
        "       student.grade_level, student.gpa "
        "FROM student "
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4
        "INNER JOIN user ON student.id = user.id "
        "WHERE student.id = ?;";

    fputs("  student: {\n", out);

    // Keep the JavaScript valid even when no child is linked to the parent.
    if (child_id <= 0) {
        fputs("  },\n", out);
        return;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  },\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, child_id);

    // Write the student information if the database returns a record.
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

        // Format the GPA with two digits after the decimal point.
        fputs("    gpa: ", out);
        fprintf(out, "%.2f\n", sqlite3_column_double(statement, 4));
    }

    sqlite3_finalize(statement);
    fputs("  },\n", out);
}

/* -------------------------------------------------------------------------
   write_grades_section
   Writes the child's score in each course.
   ------------------------------------------------------------------------- */
static void write_grades_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    // Join grade with course so the frontend receives the course name.
    const char *sql =
        "SELECT course.course_name, grade.score "
        "FROM grade "
        "INNER JOIN course ON grade.course_id = course.id "
        "WHERE student.parent_id = ? ORDER BY user.id, course.id;", parent_user_id);

<<<<<<< HEAD
    /* ---- children's attendance ---- */
    write_list(out, db, "attendance",
        "SELECT user.first_name || ' ' || user.last_name AS student, "
        "course.course_name AS course, attendance.present AS present FROM attendance "
        "INNER JOIN student ON attendance.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
=======
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

    // Continue until every grade row for the student has been written.
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
   Writes the child's attendance record.
   ------------------------------------------------------------------------- */
static void write_attendance_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    // Join attendance with course to include the readable course name.
    const char *sql =
        "SELECT course.course_name, attendance.present "
        "FROM attendance "
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4
        "INNER JOIN course ON attendance.course_id = course.id "
        "WHERE student.parent_id = ? ORDER BY user.id, course.id;", parent_user_id);

    /* ---- announcements (school wide) ---- */
    write_list(out, db, "announcements",
        "SELECT announcement.title AS title, announcement.message AS message, "
        "announcement.announcement_date AS date FROM announcement "
        "ORDER BY announcement.id DESC;", 0);

    fprintf(out, "  success: %s,\n  parentId: %d\n};\n",
            found ? "true" : "false", parent_user_id);

<<<<<<< HEAD
    fclose(out);
    close_database(db);
    printf("[Parent] parents_states.js written for parent %d\n", parent_user_id);
    return found;
}
=======
    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }

    sqlite3_bind_int(statement, 1, child_id);

    // Process every attendance record returned for the student.
    while (sqlite3_step(statement) == SQLITE_ROW) {
        int present;

        fputs("    { course: ", out);
        write_js_quoted(out, sqlite3_column_text(statement, 0));

        // Convert the numeric database value into readable status text.
        present = sqlite3_column_int(statement, 1);
        fputs(", status: ", out);
        write_js_quoted(
            out,
            (const unsigned char *)(present ? "Present" : "Absent")
        );

        fputs(" },\n", out);
    }

    sqlite3_finalize(statement);
    fputs("  ],\n", out);
}

/* -------------------------------------------------------------------------
   write_announcements_section
   Writes the school announcements, newest first.
   ------------------------------------------------------------------------- */
static void write_announcements_section(FILE *out, sqlite3 *db) {
    sqlite3_stmt *statement = NULL;

    // LEFT JOIN keeps general announcements that do not have a course ID.
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

    // Write every announcement returned by the query.
    while (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char *course = sqlite3_column_text(statement, 0);

        fputs("    { course: ", out);

        // Use General if the announcement is not linked to a course.
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
   The function the server calls. It opens the database and the output file,
   works out which child belongs to this parent, writes each section and then
   closes both.
   ------------------------------------------------------------------------- */
void update_parent_frontend_state(int parent_user_id) {
    sqlite3 *db = NULL;
    FILE *out = NULL;
    int child_id;

    /* Step 1: open the database. */
    if (sqlite3_open("academia.db", &db) != SQLITE_OK) {
        printf("[Parent Module] Cannot open academia.db\n");

        // SQLite may create a database object even when opening fails.
        if (db != NULL) {
            sqlite3_close(db);
        }

        return;
    }

    /* Step 2: open the file the portal reads. */
    out = fopen("FrontEnd/JS/parents_states.js", "w");

    // Try a second path if the server started in a different folder.
    if (out == NULL) {
        out = fopen("BackEnd/Academia Parent/parents_states.js", "w");
    }

    if (out == NULL) {
        printf("[Parent Module] Cannot open parents_states.js for writing.\n");
        sqlite3_close(db);
        return;
    }

    /* Step 3: find the child once, then hand the id to the sections that need
       it. Asking four times would be slower and would repeat the same SQL. */
    child_id = find_child_id(db, parent_user_id);

    // Start the generated JavaScript state object.
    fputs(
        "// Auto-generated by the Academia C backend. Do not edit by hand.\n",
        out
    );
    fputs("const parentState = {\n", out);

    // Each function writes one section of the parent portal state.
    write_parent_section(out, db, parent_user_id);
    write_child_section(out, db, child_id);
    write_grades_section(out, db, child_id);
    write_attendance_section(out, db, child_id);
    write_announcements_section(out, db);

    // Mark the state as successfully generated and close the object.
    fputs("  success: true\n", out);
    fputs("};\n", out);

    /* Step 4: close both. */
    fclose(out);
    sqlite3_close(db);

    printf(
        "[Parent Module] parents_states.js written for parent %d (child %d)\n",
        parent_user_id,
        child_id
    );
}
>>>>>>> 343e1edff3a9b596c6861bf939d4762787ad76f4
