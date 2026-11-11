/*
   ==========================================================================
   ACADEMIA - PARENT MODULE  (Academia_Parents.c)
   Writes parents_states.js, the file the Parent Portal reads
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================

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
*/

#include "Academia_Parents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsqlite/winsqlite3.h>

/* -------------------------------------------------------------------------
   write_js_text
   Copies one piece of text into the output file, adding a backslash before
   any character that would otherwise break a JavaScript string.

   Without this, a student called Ann "Annie" Lee would end the string
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
}

/* -------------------------------------------------------------------------
   write_child_section
   Writes the linked student's profile. child_id == 0 means the parent has no
   student attached, in which case the section is written empty.
   ------------------------------------------------------------------------- */
static void write_child_section(FILE *out, sqlite3 *db, int child_id) {
    sqlite3_stmt *statement = NULL;

    // Join student with user because the name and school data use two tables.
    const char *sql =
        "SELECT user.id, user.first_name, user.last_name, "
        "       student.grade_level, student.gpa "
        "FROM student "
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