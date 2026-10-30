/*
Harshul Gupta
Cs50 c project
Student Module
10.20.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

/*
This file reads parent and student information from the database.
It creates parents_states.js for the parent portal frontend.
*/


// Writes database text safely inside a JavaScript string.
// Special characters are escaped to avoid JavaScript syntax errors.
void write_js_text(FILE *out, const unsigned char *text)
{
    int i;

    // Stops if the database value is NULL.
    if (text == NULL)
    {
        return;
    }

    // Checks and writes every character in the string.
    for (i = 0; text[i] != '\0'; i++)
    {
        // Escapes backslashes and double quotation marks.
        if (text[i] == '\\' || text[i] == '"')
        {
            fputc('\\', out);
        }

        // Converts a real line break into a JavaScript newline.
        if (text[i] == '\n')
        {
            fprintf(out, "\\n");
        }
        else
        {
            fputc(text[i], out);
        }
    }
}


// Creates the JavaScript state file used by the parent portal.
// The provided user ID determines which parent's data is exported.
void update_parent_frontend_state(int parent_user_id)
{
    // Stores the database connection, SQL statement, and output file.
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    FILE *out;

    // Tracks whether the parent exists and whether a query fails.
    int parent_found = 0;
    int query_error = 0;


    // Opens the Academia SQLite database.
    // The function stops if the connection cannot be created.
    if (sqlite3_open(
            "Academia/BackEnd/Academia Database/academia.db",
            &db) != SQLITE_OK)
    {
        printf(
            "[Error] Cannot open database: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);
        return;
    }


    // Opens parents_states.js and removes its previous contents.
    // The file will contain the latest data for the selected parent.
    out = fopen(
        "Academia/BackEnd/Academia Parents/parents_states.js",
        "w"
    );

    // Stops if the JavaScript file cannot be created.
    // The database is closed before the function returns.
    if (out == NULL)
    {
        printf("[Error] Cannot create parents_states.js.\n");

        sqlite3_close(db);
        return;
    }


    // Starts the parentState JavaScript object.
    fprintf(out, "const parentState = {\n");


    /*
    Exports the selected parent's account information.
    The user and parent tables are connected using matching IDs.
    */

    // Starts the parent object.
    fprintf(out, "  parent: {\n");

    // Selects the parent's account and contact information.
    const char *sql_parent =
        "SELECT "
        "user.id, "
        "user.username, "
        "user.first_name, "
        "user.last_name, "
        "parent.phone "
        "FROM user "
        "INNER JOIN parent "
        "ON user.id = parent.id "
        "WHERE user.id = ? "
        "AND user.role = 'parent';";


    // Prepares the parent query so SQLite can execute it.
    // The parent ID is added to the query after preparation.
    if (sqlite3_prepare_v2(
            db,
            sql_parent,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Inserts the parent user ID into the question mark.
        sqlite3_bind_int(
            stmt,
            1,
            parent_user_id
        );

        // Executes the query and checks for a matching parent.
        // The returned information is written to the JavaScript file.
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            parent_found = 1;

            // Writes the parent's user ID.
            fprintf(
                out,
                "    userId: %d,\n",
                sqlite3_column_int(stmt, 0)
            );

            // Writes the parent's username.
            fprintf(out, "    username: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );
            fprintf(out, "\",\n");

            // Writes the parent's first name.
            fprintf(out, "    firstName: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );
            fprintf(out, "\",\n");

            // Writes the parent's last name.
            fprintf(out, "    lastName: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );
            fprintf(out, "\",\n");

            // Writes the parent's phone number.
            fprintf(out, "    phone: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 4)
            );
            fprintf(out, "\"\n");
        }
        else
        {
            // Reports that the given ID does not belong to a parent.
            // The generated state will contain an error result.
            printf("[Error] Parent account not found.\n");
        }
    }
    else
    {
        // Records that the parent query could not be prepared.
        // The SQLite error message is printed for debugging.
        query_error = 1;

        printf(
            "[Error] Parent query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    // Releases the parent query so the statement can be reused.
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    // Closes the parent object.
    fprintf(out, "  },\n");


    /*
    Exports all students connected to the selected parent.
    Each returned student is added to the students array.
    */

    // Starts the students array.
    fprintf(out, "  students: [\n");

    // Selects each student's account and academic information.
    const char *sql_students =
        "SELECT "
        "student.id, "
        "user.username, "
        "user.first_name, "
        "user.last_name, "
        "student.grade_level, "
        "student.gpa "
        "FROM student "
        "INNER JOIN user "
        "ON student.id = user.id "
        "WHERE student.parent_id = ? "
        "AND user.role = 'student';";


    // Prepares the student query and connects it to the parent ID.
    // The query may return multiple students for one parent.
    if (sqlite3_prepare_v2(
            db,
            sql_students,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Inserts the parent ID into the question mark.
        sqlite3_bind_int(
            stmt,
            1,
            parent_user_id
        );

        // Processes every student returned by the database.
        // Each row becomes one JavaScript student object.
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // Starts the student object and writes the ID.
            fprintf(
                out,
                "    { studentId: %d, username: \"",
                sqlite3_column_int(stmt, 0)
            );

            // Writes the student's username.
            write_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            // Writes the student's first name.
            fprintf(out, "\", firstName: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            // Writes the student's last name.
            fprintf(out, "\", lastName: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );

            // Writes the student's grade level and GPA.
            fprintf(
                out,
                "\", gradeLevel: %d, gpa: %.2f },\n",
                sqlite3_column_int(stmt, 4),
                sqlite3_column_double(stmt, 5)
            );
        }
    }
    else
    {
        // Records that the student query could not be prepared.
        // The SQLite error message is printed for debugging.
        query_error = 1;

        printf(
            "[Error] Student query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    // Releases the student query so the statement can be reused.
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    // Closes the students array.
    fprintf(out, "  ],\n");


    /*
    Exports grades belonging to the parent's students.
    The course table provides the course name for each grade.
    */

    // Starts the grades array.
    fprintf(out, "  grades: [\n");

    // Selects each student's course name and numerical score.
    const char *sql_grades =
        "SELECT "
        "grade.student_id, "
        "course.course_name, "
        "grade.score "
        "FROM grade "
        "INNER JOIN student "
        "ON grade.student_id = student.id "
        "INNER JOIN course "
        "ON grade.course_id = course.id "
        "WHERE student.parent_id = ?;";


    // Prepares the grade query and limits it to this parent.
    // Every returned row represents one student's course grade.
    if (sqlite3_prepare_v2(
            db,
            sql_grades,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Inserts the parent ID into the question mark.
        sqlite3_bind_int(
            stmt,
            1,
            parent_user_id
        );

        // Processes every grade returned by the database.
        // Each row becomes one JavaScript grade object.
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // Starts the grade object and writes the student ID.
            fprintf(
                out,
                "    { studentId: %d, course: \"",
                sqlite3_column_int(stmt, 0)
            );

            // Writes the course name.
            write_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            // Writes the numerical score.
            fprintf(
                out,
                "\", score: %.2f },\n",
                sqlite3_column_double(stmt, 2)
            );
        }
    }
    else
    {
        // Records that the grade query could not be prepared.
        // The SQLite error message is printed for debugging.
        query_error = 1;

        printf(
            "[Error] Grade query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    // Releases the grade query so the statement can be reused.
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    // Closes the grades array.
    fprintf(out, "  ],\n");


    /*
    Exports attendance records for the parent's students.
    The Boolean value is converted into Present or Absent.
    */

    // Starts the attendance array.
    fprintf(out, "  attendance: [\n");

    // Selects the student ID, course name, and attendance value.
    const char *sql_attendance =
        "SELECT "
        "attendance.student_id, "
        "course.course_name, "
        "attendance.present "
        "FROM attendance "
        "INNER JOIN student "
        "ON attendance.student_id = student.id "
        "INNER JOIN course "
        "ON attendance.course_id = course.id "
        "WHERE student.parent_id = ?;";


    // Prepares the attendance query for the selected parent.
    // Every returned row represents one attendance record.
    if (sqlite3_prepare_v2(
            db,
            sql_attendance,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Inserts the parent ID into the question mark.
        sqlite3_bind_int(
            stmt,
            1,
            parent_user_id
        );

        // Processes each attendance record from the database.
        // The Boolean value is converted into readable text.
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int present;

            // Reads the Boolean attendance value.
            present = sqlite3_column_int(stmt, 2);

            // Starts the attendance object and writes the student ID.
            fprintf(
                out,
                "    { studentId: %d, course: \"",
                sqlite3_column_int(stmt, 0)
            );

            // Writes the course name.
            write_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            // Writes Present when true and Absent when false.
            fprintf(
                out,
                "\", status: \"%s\" },\n",
                present ? "Present" : "Absent"
            );
        }
    }
    else
    {
        // Records that the attendance query could not be prepared.
        // The SQLite error message is printed for debugging.
        query_error = 1;

        printf(
            "[Error] Attendance query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    // Releases the attendance query so the statement can be reused.
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    // Closes the attendance array.
    fprintf(out, "  ],\n");


    /*
    Exports every school announcement.
    Announcements are ordered from the newest ID to the oldest.
    */

    // Starts the announcements array.
    fprintf(out, "  announcements: [\n");

    // Selects the title, message, and date of every announcement.
    const char *sql_announcements =
        "SELECT "
        "title, "
        "message, "
        "announcement_date "
        "FROM announcement "
        "ORDER BY id DESC;";


    // Prepares the announcement query for execution.
    // This query does not require a bound parent ID.
    if (sqlite3_prepare_v2(
            db,
            sql_announcements,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Processes every announcement returned by the database.
        // Each row becomes one JavaScript announcement object.
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // Starts the announcement object and writes its title.
            fprintf(out, "    { title: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 0)
            );

            // Writes the announcement message.
            fprintf(out, "\", message: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            // Writes the announcement date.
            fprintf(out, "\", date: \"");
            write_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            fprintf(out, "\" },\n");
        }
    }
    else
    {
        // Records that the announcement query could not be prepared.
        // The SQLite error message is printed for debugging.
        query_error = 1;

        printf(
            "[Error] Announcement query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    // Releases the final prepared statement.
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    // Closes the announcements array.
    fprintf(out, "  ],\n");


    /*
    Writes the final result used by the parent HTML.
    The frontend displays either the parent data or an error message.
    */

    // Reports an error if the parent account was not found.
    // This prevents the frontend from displaying an empty account.
    if (!parent_found)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Parent account not found.\"\n"
        );
    }

    // Reports an error if one or more data queries failed.
    // The parent may exist even if some portal data cannot be loaded.
    else if (query_error)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Some parent data could not be loaded.\"\n"
        );
    }

    // Reports success when all required queries are completed.
    // An empty error string indicates that no error occurred.
    else
    {
        fprintf(out, "  success: true,\n");
        fprintf(out, "  error: \"\"\n");
    }


    // Closes the parentState JavaScript object.
    fprintf(out, "};\n");

    // Saves and closes the generated JavaScript file.
    fclose(out);

    // Closes the SQLite database connection.
    sqlite3_close(db);

    // Confirms that the state file was updated.
    printf(
        "[Parent Logic] Successfully updated parents_states.js.\n"
    );
}