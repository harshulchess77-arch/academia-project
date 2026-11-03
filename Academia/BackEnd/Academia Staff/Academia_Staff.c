/*
Lawrence Mao
CS50 C Project
School-Staff Functions
10.22.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

/*
Reads staff information from the Academia database.
The data is written to staff_states.js for the frontend.
*/


// Writes database text safely inside a JavaScript string.
static void write_staff_js_text(
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


// Creates the JavaScript state file used by the Staff Portal.
// The staff user ID determines which staff data is exported.
void update_staff_frontend_state(int staff_user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    FILE *out;

    int staff_found = 0;
    int query_error = 0;


    // Opens the Academia database.
    // The function stops if the database connection fails.
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


    // Creates the Staff Portal state file.
    // Previous contents are replaced with current data.
    out = fopen(
        "Academia/BackEnd/Academia Staff/staff_states.js",
        "w"
    );

    if (out == NULL)
    {
        printf("[Error] Cannot create staff_states.js.\n");

        sqlite3_close(db);
        return;
    }


    // Starts the staffState JavaScript object.
    fprintf(out, "const staffState = {\n");


    /*
    Retrieves the selected staff member's account information.
    The user and staff tables are connected through matching IDs.
    */

    fprintf(out, "  staff: {\n");

    const char *sql_staff =
        "SELECT "
        "user.id, "
        "user.username, "
        "user.first_name, "
        "user.last_name, "
        "staff.phone "
        "FROM user "
        "INNER JOIN staff "
        "ON user.id = staff.id "
        "WHERE user.id = ? "
        "AND user.role = 'staff';";


    // Prepares the staff query and binds the user ID.
    // A returned row confirms that the staff account exists.
    if (sqlite3_prepare_v2(
            db,
            sql_staff,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(
            stmt,
            1,
            staff_user_id
        );

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            staff_found = 1;

            fprintf(
                out,
                "    userId: %d,\n",
                sqlite3_column_int(stmt, 0)
            );

            fprintf(out, "    username: \"");
            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );
            fprintf(out, "\",\n");

            fprintf(out, "    firstName: \"");
            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );
            fprintf(out, "\",\n");

            fprintf(out, "    lastName: \"");
            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );
            fprintf(out, "\",\n");

            fprintf(out, "    phone: \"");
            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 4)
            );
            fprintf(out, "\"\n");
        }
        else
        {
            printf("[Error] Staff account not found.\n");
        }
    }
    else
    {
        query_error = 1;

        printf(
            "[Error] Staff query failed: %s\n",
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
    Retrieves all courses assigned to the staff member.
    Each course is written as an object in the courses array.
    */

    fprintf(out, "  courses: [\n");

    const char *sql_courses =
        "SELECT "
        "id, "
        "course_name "
        "FROM course "
        "WHERE staff_id = ?;";


    // Prepares the course query and binds the staff ID.
    // The query may return multiple assigned courses.
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
            staff_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { courseId: %d, course: \"",
                sqlite3_column_int(stmt, 0)
            );

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 1)
            );

            fprintf(out, "\" },\n");
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
    Retrieves student grades for courses taught by this staff member.
    Each result includes the student, course, and numerical score.
    */

    fprintf(out, "  grades: [\n");

    const char *sql_grades =
        "SELECT "
        "grade.id, "
        "grade.student_id, "
        "user.first_name, "
        "user.last_name, "
        "course.course_name, "
        "grade.score "
        "FROM grade "
        "INNER JOIN student "
        "ON grade.student_id = student.id "
        "INNER JOIN user "
        "ON student.id = user.id "
        "INNER JOIN course "
        "ON grade.course_id = course.id "
        "WHERE course.staff_id = ?;";


    // Prepares the grade query and binds the staff ID.
    // Every returned grade belongs to one of the staff member's courses.
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
            staff_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { gradeId: %d, studentId: %d, student: \"",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_int(stmt, 1)
            );

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            fprintf(out, " ");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );

            fprintf(out, "\", course: \"");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 4)
            );

            fprintf(
                out,
                "\", score: %.2f },\n",
                sqlite3_column_double(stmt, 5)
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
    Retrieves attendance for courses taught by the staff member.
    Boolean values are converted into Present or Absent.
    */

    fprintf(out, "  attendance: [\n");

    const char *sql_attendance =
        "SELECT "
        "attendance.id, "
        "attendance.student_id, "
        "user.first_name, "
        "user.last_name, "
        "course.course_name, "
        "attendance.present "
        "FROM attendance "
        "INNER JOIN student "
        "ON attendance.student_id = student.id "
        "INNER JOIN user "
        "ON student.id = user.id "
        "INNER JOIN course "
        "ON attendance.course_id = course.id "
        "WHERE course.staff_id = ?;";


    // Prepares the attendance query and binds the staff ID.
    // Each returned row belongs to one of the staff member's courses.
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
            staff_user_id
        );

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { attendanceId: %d, studentId: %d, student: \"",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_int(stmt, 1)
            );

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            fprintf(out, " ");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );

            fprintf(out, "\", course: \"");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 4)
            );

            fprintf(
                out,
                "\", status: \"%s\" },\n",
                sqlite3_column_int(stmt, 5)
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
    Retrieves every announcement from the database.
    Newer announcements are placed first.
    */

    fprintf(out, "  announcements: [\n");

    const char *sql_announcements =
        "SELECT "
        "id, "
        "course_id, "
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
            fprintf(
                out,
                "    { announcementId: %d, courseId: %d, title: \"",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_int(stmt, 1)
            );

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 2)
            );

            fprintf(out, "\", message: \"");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 3)
            );

            fprintf(out, "\", date: \"");

            write_staff_js_text(
                out,
                sqlite3_column_text(stmt, 4)
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


    // Writes the final result for the Staff Portal.
    if (!staff_found)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Staff account not found.\"\n"
        );
    }
    else if (query_error)
    {
        fprintf(out, "  success: false,\n");
        fprintf(
            out,
            "  error: \"Some staff data could not be loaded.\"\n"
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
        "[Staff Logic] Successfully updated staff_states.js.\n"
    );
}