/*
Harshul Gupta
CS50 C Project
Student Module
10.20.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

/*
Reads student information from the Academia database.
The data is written to student_states.js for the frontend.
*/


// Writes database text safely inside a JavaScript string.
static void write_student_js_text(
    FILE *out,
    const unsigned char *text
)
{
    int i;

    if (text == NULL)
    {
        return;
    }

    // Escapes characters that could break a JavaScript string.
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


// Creates the JavaScript state file used by the student portal.
// The user ID determines which student's data is exported.
void update_student_frontend_state(int student_user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    FILE *out;

    int student_found = 0;
    int query_error = 0;


    // Opens the Academia database.
    // The function stops if the connection fails.
    if (sqlite3_open(
            "Academia/BackEnd/Academia Database/academia.db",
            &db
        ) != SQLITE_OK)
    {
        printf(
            "[Error] Cannot open database: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);
        return;
    }


    // Creates the Student Portal state file.
    // Previous contents are replaced with current database data.
    out = fopen(
        "Academia/BackEnd/Academia Student/student_states.js",
        "w"
    );

    if (out == NULL)
    {
        printf("[Error] Cannot create student_states.js.\n");

        sqlite3_close(db);
        return;
    }


    // Starts the studentState JavaScript object.
    fprintf(out, "const studentState = {\n");


    /*
    Retrieves the student's account and academic information.
    The user and student tables are connected through matching IDs.
    */

    fprintf(out, "  student: {\n");

    const char *sql_student =
        "SELECT "
        "user.id, "
        "user.username, "
        "user.first_name, "
        "user.last_name, "
        "student.grade_level, "
        "student.gpa "
        "FROM user "
        "INNER JOIN student "
        "ON user.id = student.id "
        "WHERE user.id = ? "
        "AND user.role = 'student';";


    // Prepares the student query and binds the user ID.
    // A returned row confirms that the student account exists.
    if (sqlite3_prepare_v2(
            db,
            sql_student,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(
            stmt,
            1,
            student_user_id
        );

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            student_found = 1;

            fprintf(
                out,
                "    userId: %d,\n",
                sqlite3_column_int(stmt, 0)
            );

            fprintf(out, "    username: \"");
            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );
            fprintf(out, "\",\n");

            fprintf(out, "    firstName: \"");
            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );
            fprintf(out, "\",\n");

            fprintf(out, "    lastName: \"");
            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );
            fprintf(out, "\",\n");

            fprintf(
                out,
                "    gradeLevel: %d,\n",
                sqlite3_column_int(stmt, 4)
            );

            fprintf(
                out,
                "    gpa: %.2f\n",
                sqlite3_column_double(stmt, 5)
            );
        }
        else
        {
            printf("[Error] Student account not found.\n");
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Student query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  },\n");


    /*
    Retrieves courses in which the student is enrolled.
    The enrollment and course tables provide the course information.
    */

    fprintf(out, "  courses: [\n");

    const char *sql_courses =
        "SELECT "
        "course.id, "
        "course.course_name, "
        "enrollment.status "
        "FROM enrollment "
        "INNER JOIN course "
        "ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ?;";


    // Prepares the course query and binds the student ID.
    // Each returned row is written as one course object.
    if (sqlite3_prepare_v2(
            db,
            sql_courses,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(
            stmt,
            1,
            student_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { courseId: %d, course: \"",
                sqlite3_column_int(stmt, 0)
            );

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            fprintf(
                out,
                "\", active: %s },\n",
                sqlite3_column_int(stmt, 2)
                    ? "true"
                    : "false"
            );
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Course query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
    Retrieves the student's grades and related course names.
    Each returned grade becomes one object in the grades array.
    */

    fprintf(out, "  grades: [\n");

    const char *sql_grades =
        "SELECT "
        "course.course_name, "
        "grade.score "
        "FROM grade "
        "INNER JOIN course "
        "ON grade.course_id = course.id "
        "WHERE grade.student_id = ?;";


    // Prepares the grade query and binds the student ID.
    // The query returns every course grade for this student.
    if (sqlite3_prepare_v2(
            db,
            sql_grades,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(
            stmt,
            1,
            student_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out, "    { course: \"");

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 0)
            );

            fprintf(
                out,
                "\", score: %.2f },\n",
                sqlite3_column_double(stmt, 1)
            );
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Grade query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
    Retrieves attendance records for the selected student.
    Boolean values are converted into Present or Absent.
    */

    fprintf(out, "  attendance: [\n");

    const char *sql_attendance =
        "SELECT "
        "course.course_name, "
        "attendance.present "
        "FROM attendance "
        "INNER JOIN course "
        "ON attendance.course_id = course.id "
        "WHERE attendance.student_id = ?;";


    // Prepares the attendance query and binds the student ID.
    // Each result contains a course and attendance status.
    if (sqlite3_prepare_v2(
            db,
            sql_attendance,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(
            stmt,
            1,
            student_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out, "    { course: \"");

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 0)
            );

            fprintf(
                out,
                "\", status: \"%s\" },\n",
                sqlite3_column_int(stmt, 1)
                    ? "Present"
                    : "Absent"
            );
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Attendance query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
    Retrieves all school announcements from the database.
    Newer announcements are displayed first.
    */

    fprintf(out, "  announcements: [\n");

    const char *sql_announcements =
        "SELECT "
        "title, "
        "message, "
        "announcement_date "
        "FROM announcement "
        "ORDER BY id DESC;";


    // Prepares and processes the announcement query.
    // Each result becomes one announcement object.
    if (sqlite3_prepare_v2(
            db,
            sql_announcements,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out, "    { title: \"");

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 0)
            );

            fprintf(out, "\", message: \"");

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            fprintf(out, "\", date: \"");

            write_student_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            fprintf(out, "\" },\n");
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Announcement query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    // Writes the final result for the Student Portal.
    if (!student_found)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Student account not found.\"\n"
        );
    }
    else if (query_error)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Some student data could not be loaded.\"\n"
        );
    }
    else
    {
        fprintf(out, "  success: true,\n");
        fprintf(out, "  error: \"\"\n");
    }

    fprintf(out, "};\n");

    fclose(out);
    sqlite3_close(db);

    printf(
        "[Student Logic] Successfully updated student_states.js.\n"
    );
}