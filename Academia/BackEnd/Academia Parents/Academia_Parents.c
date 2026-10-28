/*
Seungho Kim
CS50 C Project
Parent Functions
10.27.26
*/

#include <stdio.h>
#include <sqlite3.h>

/*
This file reads parent and student information from the
database and creates a JavaScript file for the parent frontend.
*/


// Writes database text safely into a JavaScript string
void write_js_text(FILE *out, const unsigned char *text)
{
    int i;

    // Do not write anything if the database value is NULL
    if (text == NULL)
    {
        return;
    }

    // Write each character while escaping special characters
    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\\' || text[i] == '"')
        {
            fputc('\\', out);
        }

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


// Creates the JavaScript state file used by the parent frontend
void update_parent_frontend_state(int parent_user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    FILE *out;

    // Open the Academia database
    if (sqlite3_open(
            "Academia/BackEnd/Academia Database/academia.db",
            &db) != SQLITE_OK)
    {
        printf("[Error] Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Create the JavaScript state file
    out = fopen(
        "Academia/FrontEnd/Academia Parents/parent_state.js",
        "w"
    );

    if (out == NULL)
    {
        printf("[Error] Cannot create parent state file.\n");
        sqlite3_close(db);
        return;
    }

    // Start the parentState JavaScript object
    fprintf(out, "const parentState = {\n");

    /*
    Export the parent account information
    */

    fprintf(out, "  parent: {\n");

    const char *sql_parent =
        "SELECT users.id, users.username, "
        "parents.id, parents.first_name, parents.last_name "
        "FROM users "
        "INNER JOIN parents ON users.id = parents.user_id "
        "WHERE users.id = ? AND users.role = 'parent';";

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
