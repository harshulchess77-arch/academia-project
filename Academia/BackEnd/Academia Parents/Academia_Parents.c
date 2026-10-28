/*
Seungho Kim
CS50 C Project
Parent Functions
10.27.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

/*
This file reads parent and student information from the
database and creates a JavaScript file for the parent frontend.
*/


// Writes database text safely into a JavaScript string
// javascript strings need to escape certain characters like backslashes and double quotes, and newlines should be represented as \\n to avoid syntax errors in the generated JavaScript file.
void write_js_text(FILE *out, const unsigned char *text)
{
    int i;

    // returns if the text is NULL
    if (text == NULL)
    {
        return;
    }

    // Write each character while escaping special characters
    // aviod java script syntax errors
    for (i = 0; text[i] != '\0'; i++)
    {
        // if the character is a backslash or double quote, print a backslash before it
        if (text[i] == '\\' || text[i] == '"')
        {
            fputc('\\', out);
        }

        // if the character is a newline, print \\n instead of \n
        if (text[i] == '\n')
        {
            fprintf(out, "\\n");
        }

        // if the character is not \n then it is printed as is
        else
        {
            fputc(text[i], out);
        }
    }
}


// Creates the JavaScript state file used by the parent frontend
void update_parent_frontend_state(int parent_user_id)
{
    // Database connection and statement objects
    sqlite3 *db;
    sqlite3_stmt *stmt;
    FILE *out;

    // Open the Academia database
    // provide the path to the database file
    // if the database cannot be opened, print an error message and return
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK)
    {
        // Print an error message if the database cannot be opened
        printf("[Error] Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Create the JavaScript state file
    // if there is a parent_state.js file already, it will be overwritten
    out = fopen("Academia/FrontEnd/Academia Parents/parent_state.js", "w");

    // check if the file was created successfully
    // if not, print an error message and close the database connection
    if (out == NULL)
    {
        // Print an error message if the file cannot be created
        printf("[Error] Cannot create parent state file.\n");

        // Close the database connection before returning
        sqlite3_close(db);
        return;
    }

    // Start the parentState JavaScript object
    fprintf(out, "const parentState = {\n");


    // taking the parent information from the database and writing it to the JavaScript file
    // writing the parent information to the JavaScript file in a structured format
    fprintf(out, "  parent: {\n");

    // SQL query to get the parent information from the database
    const char *sql_parent =
        "SELECT id, first_name, last_name "
        "FROM user "
        "WHERE id = ? AND role = 'parent';";

    // Prepare the parent query
    if (sqlite3_prepare_v2(
            db,
            sql_parent,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        // Replace the question mark with the parent user ID
        sqlite3_bind_int(stmt, 1, parent_user_id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    userId: %d,\n",
                sqlite3_column_int(stmt, 0)
            );

            fprintf(out, "    username: \"");
            write_js_text(out, sqlite3_column_text(stmt, 1));
            fprintf(out, "\",\n");

            fprintf(
                out,
                "    parentId: %d,\n",
                sqlite3_column_int(stmt, 2)
            );

            fprintf(out, "    firstName: \"");
            write_js_text(out, sqlite3_column_text(stmt, 3));
            fprintf(out, "\",\n");

            fprintf(out, "    lastName: \"");
            write_js_text(out, sqlite3_column_text(stmt, 4));
            fprintf(out, "\"\n");
        }
        else
        {
            printf("[Error] Parent account not found.\n");
        }
    }
    else
    {
        printf("[Error] Parent query failed: %s\n", sqlite3_errmsg(db));
    }

    // Release the parent query
    sqlite3_finalize(stmt);

    fprintf(out, "  },\n");

    /*
    Export students connected to the parent
    */

    fprintf(out, "  students: [\n");

    const char *sql_students =
        "SELECT students.id, users.username, "
        "students.first_name, students.last_name, "
        "students.grade_level, students.gpa "
        "FROM parents "
        "INNER JOIN parent_student "
        "ON parents.id = parent_student.parent_id "
        "INNER JOIN students "
        "ON students.id = parent_student.student_id "
        "INNER JOIN users "
        "ON users.id = students.user_id "
        "WHERE parents.user_id = ?;";

    // Prepare the student query
    if (sqlite3_prepare_v2(
            db,
            sql_students,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, parent_user_id);

        // Export every student connected to the parent
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { studentId: %d, username: \"",
                sqlite3_column_int(stmt, 0)
            );

            write_js_text(out, sqlite3_column_text(stmt, 1));

            fprintf(out, "\", firstName: \"");
            write_js_text(out, sqlite3_column_text(stmt, 2));

            fprintf(out, "\", lastName: \"");
            write_js_text(out, sqlite3_column_text(stmt, 3));

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
        printf("[Error] Student query failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ],\n");

    /*
    Export grades for the parent's students
    */

    fprintf(out, "  grades: [\n");

    const char *sql_grades =
        "SELECT grades.student_id, grades.course, grades.grade "
        "FROM grades "
        "INNER JOIN parent_student "
        "ON grades.student_id = parent_student.student_id "
        "INNER JOIN parents "
        "ON parents.id = parent_student.parent_id "
        "WHERE parents.user_id = ?;";

    // Prepare the grade query
    if (sqlite3_prepare_v2(
            db,
            sql_grades,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, parent_user_id);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { studentId: %d, course: \"",
                sqlite3_column_int(stmt, 0)
            );

            write_js_text(out, sqlite3_column_text(stmt, 1));

            fprintf(
                out,
                "\", grade: %.2f },\n",
                sqlite3_column_double(stmt, 2)
            );
        }
    }
    else
    {
        printf("[Error] Grade query failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ],\n");

    /*
    Export attendance for the parent's students
    */

    fprintf(out, "  attendance: [\n");

    const char *sql_attendance =
        "SELECT attendance.student_id, "
        "attendance.date, attendance.status "
        "FROM attendance "
        "INNER JOIN parent_student "
        "ON attendance.student_id = parent_student.student_id "
        "INNER JOIN parents "
        "ON parents.id = parent_student.parent_id "
        "WHERE parents.user_id = ?;";

    // Prepare the attendance query
    if (sqlite3_prepare_v2(
            db,
            sql_attendance,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, parent_user_id);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { studentId: %d, date: \"",
                sqlite3_column_int(stmt, 0)
            );

            write_js_text(out, sqlite3_column_text(stmt, 1));

            fprintf(out, "\", status: \"");
            write_js_text(out, sqlite3_column_text(stmt, 2));

            fprintf(out, "\" },\n");
        }
    }
    else
    {
        printf(
            "[Error] Attendance query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ],\n");

    /*
    Export school announcements
    */

    fprintf(out, "  announcements: [\n");

    const char *sql_announcements =
        "SELECT title, message "
        "FROM announcements "
        "ORDER BY id DESC;";

    // Prepare the announcement query
    if (sqlite3_prepare_v2(
            db,
            sql_announcements,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out, "    { title: \"");
            write_js_text(out, sqlite3_column_text(stmt, 0));

            fprintf(out, "\", message: \"");
            write_js_text(out, sqlite3_column_text(stmt, 1));

            fprintf(out, "\" },\n");
        }
    }
    else
    {
        printf(
            "[Error] Announcement query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);

    // Close the JavaScript object
    fprintf(out, "  ]\n");
    fprintf(out, "};\n");

    // Close the JavaScript file
    fclose(out);

    // Close the database
    sqlite3_close(db);

    printf("[Parent Logic] Successfully updated frontend state.\n");
}


int main(void)
{
    // Create the frontend state for the parent with users.id 1
    update_parent_frontend_state(1);

    return 0;
}
