
/*
    Dmitri Buiamov
    CS50 C Project
    Student Module
    10.22.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"


// Writes database text safely inside a JavaScript string
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

    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\\' || text[i] == '"')
        {
            fputc('\\', out);
            fputc(text[i], out);
        }
        else if (text[i] == '\n')
        {
            fprintf(out, "\\n");
        }
        else if (text[i] == '\r')
        {
            fprintf(out, "\\r");
        }
        else if (text[i] == '\t')
        {
            fprintf(out, "\\t");
        }
        else
        {
            fputc(text[i], out);
        }
    }
}


// Writes a JavaScript string
static void write_js_string(
    FILE *out,
    const unsigned char *text
)
{
    fprintf(out, "\"");

    write_student_js_text(out, text);

    fprintf(out, "\"");
}


// Updates the Student Portal data using the Academia database
void update_student_frontend_state(int student_user_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    FILE *out;

    int student_found = 0;
    int query_error = 0;

    int result;


    // Opens the Academia database
    if (sqlite3_open(
        "Academia/BackEnd/Academia Database/academia.db",
        &db
    ) != SQLITE_OK)
    {
        printf("[Error] Cannot open database.\n");

        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return;
    }


    // Creates the student state JavaScript file
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


    fprintf(out, "const studentState = {\n");


    /*
        STUDENT INFORMATION
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


    if (sqlite3_prepare_v2(
        db,
        sql_student,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            student_found = 1;

            fprintf(
                out,
                "    userId: %d,\n",
                sqlite3_column_int(stmt, 0)
            );

            fprintf(out, "    username: ");
            write_js_string(out, sqlite3_column_text(stmt, 1));
            fprintf(out, ",\n");

            fprintf(out, "    firstName: ");
            write_js_string(out, sqlite3_column_text(stmt, 2));
            fprintf(out, ",\n");

            fprintf(out, "    lastName: ");
            write_js_string(out, sqlite3_column_text(stmt, 3));
            fprintf(out, ",\n");

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

        printf("[Error] Student query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  },\n");


    /*
        COURSES
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


    if (sqlite3_prepare_v2(
        db,
        sql_courses,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            fprintf(
                out,
                "    { courseId: %d, course: ",
                sqlite3_column_int(stmt, 0)
            );

            write_js_string(out, sqlite3_column_text(stmt, 1));

            fprintf(
                out,
                ", active: %s },\n",
                sqlite3_column_int(stmt, 2)
                    ? "true"
                    : "false"
            );
        }

        if (result != SQLITE_DONE)
        {
            query_error = 1;
        }
    }
    else
    {
        query_error = 1;

        printf("[Error] Course query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
        GRADES
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


    if (sqlite3_prepare_v2(
        db,
        sql_grades,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            fprintf(out, "    { course: ");

            write_js_string(out, sqlite3_column_text(stmt, 0));

            fprintf(
                out,
                ", score: %.2f },\n",
                sqlite3_column_double(stmt, 1)
            );
        }

        if (result != SQLITE_DONE)
        {
            query_error = 1;
        }
    }
    else
    {
        query_error = 1;

        printf("[Error] Grade query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
        ATTENDANCE
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


    if (sqlite3_prepare_v2(
        db,
        sql_attendance,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            fprintf(out, "    { course: ");

            write_js_string(out, sqlite3_column_text(stmt, 0));

            fprintf(
                out,
                ", status: \"%s\" },\n",
                sqlite3_column_int(stmt, 1)
                    ? "Present"
                    : "Absent"
            );
        }

        if (result != SQLITE_DONE)
        {
            query_error = 1;
        }
    }
    else
    {
        query_error = 1;

        printf("[Error] Attendance query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
        ASSIGNMENTS

        Assignments are retrieved from courses
        in which the student is enrolled.
    */

    fprintf(out, "  assignments: [\n");

    const char *sql_assignments =
        "SELECT "
        "course.course_name, "
        "assignment.assignment_name, "
        "assignment.due_date "
        "FROM assignment "
        "INNER JOIN course "
        "ON assignment.course_id = course.id "
        "INNER JOIN enrollment "
        "ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? "
        "AND enrollment.status = 1 "
        "ORDER BY assignment.due_date ASC;";


    if (sqlite3_prepare_v2(
        db,
        sql_assignments,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            fprintf(out, "    { course: ");

            write_js_string(out, sqlite3_column_text(stmt, 0));

            fprintf(out, ", assignmentName: ");

            write_js_string(out, sqlite3_column_text(stmt, 1));

            fprintf(out, ", dueDate: ");

            write_js_string(out, sqlite3_column_text(stmt, 2));

            fprintf(out, " },\n");
        }

        if (result != SQLITE_DONE)
        {
            query_error = 1;
        }
    }
    else
    {
        query_error = 1;

        printf("[Error] Assignment query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
        ANNOUNCEMENTS

        Announcements are retrieved from the
        student's enrolled courses.
    */

    fprintf(out, "  announcements: [\n");

    const char *sql_announcements =
        "SELECT "
        "course.course_name, "
        "announcement.title, "
        "announcement.message, "
        "announcement.announcement_date "
        "FROM announcement "
        "INNER JOIN course "
        "ON announcement.course_id = course.id "
        "INNER JOIN enrollment "
        "ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? "
        "AND enrollment.status = 1 "
        "ORDER BY announcement.id DESC;";


    if (sqlite3_prepare_v2(
        db,
        sql_announcements,
        -1,
        &stmt,
        NULL
    ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, student_user_id);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            fprintf(out, "    { course: ");

            write_js_string(out, sqlite3_column_text(stmt, 0));

            fprintf(out, ", title: ");

            write_js_string(out, sqlite3_column_text(stmt, 1));

            fprintf(out, ", message: ");

            write_js_string(out, sqlite3_column_text(stmt, 2));

            fprintf(out, ", date: ");

            write_js_string(out, sqlite3_column_text(stmt, 3));

            fprintf(out, " },\n");
        }

        if (result != SQLITE_DONE)
        {
            query_error = 1;
        }
    }
    else
    {
        query_error = 1;

        printf("[Error] Announcement query failed.\n");
    }


    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    fprintf(out, "  ],\n");


    /*
        FINAL STATUS
    */

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
