/*
Seungho Kim
CS50 C Project
Parent Functions
10.27.26
*/

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

/*
Writes text safely inside a JavaScript string.
This prevents quotes, backslashes, and line breaks
from breaking parent_state.js.
*/
void write_js_string(FILE *out, const char *text)
{
    int i;

    if (text == NULL)
    {
        return;
    }

    for (i = 0; text[i] != '\0'; i++)
    {
        switch (text[i])
        {
            case '\\':
                fprintf(out, "\\\\");
                break;

            case '"':
                fprintf(out, "\\\"");
                break;

            case '\n':
                fprintf(out, "\\n");
                break;

            case '\r':
                fprintf(out, "\\r");
                break;

            case '\t':
                fprintf(out, "\\t");
                break;

            default:
                fputc(text[i], out);
                break;
        }
    }
}


/*
Creates parent_state.js using information from academia.db.

parent_user_id means users.id for an account whose
role is "parent".
*/
void update_parent_frontend_state(int parent_user_id)
{
    sqlite3 *db;
    sqlite3_stmt *parent_stmt;
    sqlite3_stmt *student_stmt;
    sqlite3_stmt *grade_stmt;
    sqlite3_stmt *attendance_stmt;
    sqlite3_stmt *announcement_stmt;

    FILE *out;

    int open_result;
    int parent_id;

    char parent_username[100];
    char parent_first_name[100];
    char parent_last_name[100];

    db = NULL;
    parent_stmt = NULL;
    student_stmt = NULL;
    grade_stmt = NULL;
    attendance_stmt = NULL;
    announcement_stmt = NULL;

    parent_id = -1;

    snprintf(
        parent_username,
        sizeof(parent_username),
        "%s",
        "Unknown Parent"
    );

    parent_first_name[0] = '\0';
    parent_last_name[0] = '\0';

    /*
    This path works when the program is run from the
    academia-project-team main folder.
    */
    open_result = sqlite3_open(
        "Academia/BackEnd/Academia Database/academia.db",
        &db
    );

    if (open_result != SQLITE_OK)
    {
        fprintf(
            stderr,
            "[Error] Cannot open database: %s\n",
            db != NULL
                ? sqlite3_errmsg(db)
                : "Unknown SQLite error"
        );

        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return;
    }

    /*
    parent_state.js will be created in the same folder
    as Academia_Parents.html.
    */
    out = fopen(
        "Academia/FrontEnd/Academia Parents/parent_state.js",
        "w"
    );

    if (out == NULL)
    {
        fprintf(
            stderr,
            "[Error] Cannot create parent_state.js.\n"
        );

        sqlite3_close(db);
        return;
    }

    /*
    --------------------------------------------------
    PART A: Find the parent account
    --------------------------------------------------
    */

    {
        const char *parent_sql =
            "SELECT "
            "users.username, "
            "parents.id, "
            "parents.first_name, "
            "parents.last_name "
            "FROM users "
            "INNER JOIN parents "
            "ON users.id = parents.user_id "
            "WHERE users.id = ? "
            "AND users.role = 'parent';";

        if (
            sqlite3_prepare_v2(
                db,
                parent_sql,
                -1,
                &parent_stmt,
                NULL
            ) == SQLITE_OK
        )
        {
            sqlite3_bind_int(
                parent_stmt,
                1,
                parent_user_id
            );

            if (sqlite3_step(parent_stmt) == SQLITE_ROW)
            {
                const unsigned char *username_value;
                const unsigned char *first_name_value;
                const unsigned char *last_name_value;

                username_value =
                    sqlite3_column_text(parent_stmt, 0);

                parent_id =
                    sqlite3_column_int(parent_stmt, 1);

                first_name_value =
                    sqlite3_column_text(parent_stmt, 2);

                last_name_value =
                    sqlite3_column_text(parent_stmt, 3);

                if (username_value != NULL)
                {
                    snprintf(
                        parent_username,
                        sizeof(parent_username),
                        "%s",
                        (const char *) username_value
                    );
                }

                if (first_name_value != NULL)
                {
                    snprintf(
                        parent_first_name,
                        sizeof(parent_first_name),
                        "%s",
                        (const char *) first_name_value
                    );
                }

                if (last_name_value != NULL)
                {
                    snprintf(
                        parent_last_name,
                        sizeof(parent_last_name),
                        "%s",
                        (const char *) last_name_value
                    );
                }
            }
            else
            {
                fprintf(
                    stderr,
                    "[Error] No parent account found for users.id = %d.\n",
                    parent_user_id
                );
            }
        }
        else
        {
            fprintf(
                stderr,
                "[Error] Parent query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    if (parent_stmt != NULL)
    {
        sqlite3_finalize(parent_stmt);
        parent_stmt = NULL;
    }

    /*
    Start the JavaScript object.
    */

    fprintf(out, "const parentState = {\n");

    fprintf(
        out,
        "  user_id: %d,\n",
        parent_user_id
    );

    fprintf(
        out,
        "  parent_id: %d,\n",
        parent_id
    );

    fprintf(out, "  username: \"");
    write_js_string(out, parent_username);
    fprintf(out, "\",\n");

    fprintf(out, "  first_name: \"");
    write_js_string(out, parent_first_name);
    fprintf(out, "\",\n");

    fprintf(out, "  last_name: \"");
    write_js_string(out, parent_last_name);
    fprintf(out, "\",\n");

    /*
    --------------------------------------------------
    PART B: Find students connected to the parent
    --------------------------------------------------
    */

    fprintf(out, "  students: [\n");

    {
        const char *student_sql =
            "SELECT "
            "students.id, "
            "users.username, "
            "students.first_name, "
            "students.last_name, "
            "students.grade_level, "
            "students.gpa "
            "FROM parents "
            "INNER JOIN parent_student "
            "ON parents.id = parent_student.parent_id "
            "INNER JOIN students "
            "ON parent_student.student_id = students.id "
            "INNER JOIN users "
            "ON students.user_id = users.id "
            "WHERE parents.user_id = ? "
            "ORDER BY students.id;";

        if (
            sqlite3_prepare_v2(
                db,
                student_sql,
                -1,
                &student_stmt,
                NULL
            ) == SQLITE_OK
        )
        {
            sqlite3_bind_int(
                student_stmt,
                1,
                parent_user_id
            );

            while (
                sqlite3_step(student_stmt) == SQLITE_ROW
            )
            {
                int student_id;
                int grade_level;

                double stored_gpa;
                double total_grade;
                double average_grade;
                double calculated_gpa;
                double attendance_rate;

                int grade_count;
                int total_days;
                int present_days;

                const unsigned char *username_value;
                const unsigned char *first_name_value;
                const unsigned char *last_name_value;

                const char *student_username;
                const char *student_first_name;
                const char *student_last_name;

                student_id =
                    sqlite3_column_int(student_stmt, 0);

                username_value =
                    sqlite3_column_text(student_stmt, 1);

                first_name_value =
                    sqlite3_column_text(student_stmt, 2);

                last_name_value =
                    sqlite3_column_text(student_stmt, 3);

                grade_level =
                    sqlite3_column_int(student_stmt, 4);

                stored_gpa =
                    sqlite3_column_double(student_stmt, 5);

                student_username =
                    username_value != NULL
                        ? (const char *) username_value
                        : "Unknown Student";

                student_first_name =
                    first_name_value != NULL
                        ? (const char *) first_name_value
                        : "";

                student_last_name =
                    last_name_value != NULL
                        ? (const char *) last_name_value
                        : "";

                fprintf(out, "    {\n");

                fprintf(
                    out,
                    "      student_id: %d,\n",
                    student_id
                );

                fprintf(out, "      username: \"");
                write_js_string(out, student_username);
                fprintf(out, "\",\n");

                fprintf(out, "      first_name: \"");
                write_js_string(out, student_first_name);
                fprintf(out, "\",\n");

                fprintf(out, "      last_name: \"");
                write_js_string(out, student_last_name);
                fprintf(out, "\",\n");

                fprintf(
                    out,
                    "      grade_level: %d,\n",
                    grade_level
                );

                fprintf(
                    out,
                    "      stored_gpa: %.2f,\n",
                    stored_gpa
                );

                /*
                ------------------------------------------
                PART C: Get student grades
                ------------------------------------------
                */

                fprintf(out, "      grades: [\n");

                total_grade = 0.0;
                grade_count = 0;

                {
                    const char *grade_sql =
                        "SELECT course, grade "
                        "FROM grades "
                        "WHERE student_id = ? "
                        "ORDER BY course;";

                    if (
                        sqlite3_prepare_v2(
                            db,
                            grade_sql,
                            -1,
                            &grade_stmt,
                            NULL
                        ) == SQLITE_OK
                    )
                    {
                        sqlite3_bind_int(
                            grade_stmt,
                            1,
                            student_id
                        );

                        while (
                            sqlite3_step(grade_stmt) ==
                            SQLITE_ROW
                        )
                        {
                            const unsigned char *course_value;
                            const char *course;
                            double grade;

                            course_value =
                                sqlite3_column_text(
                                    grade_stmt,
                                    0
                                );

                            grade =
                                sqlite3_column_double(
                                    grade_stmt,
                                    1
                                );

                            course =
                                course_value != NULL
                                    ? (const char *) course_value
                                    : "Unknown Course";

                            fprintf(
                                out,
                                "        { course: \""
                            );

                            write_js_string(out, course);

                            fprintf(
                                out,
                                "\", grade: %.2f },\n",
                                grade
                            );

                            total_grade += grade;
                            grade_count++;
                        }
                    }
                    else
                    {
                        fprintf(
                            stderr,
                            "[Error] Grade query failed: %s\n",
                            sqlite3_errmsg(db)
                        );
                    }
                }

                if (grade_stmt != NULL)
                {
                    sqlite3_finalize(grade_stmt);
                    grade_stmt = NULL;
                }

                fprintf(out, "      ],\n");

                average_grade = 0.0;
                calculated_gpa = 0.0;

                if (grade_count > 0)
                {
                    average_grade =
                        total_grade / grade_count;

                    calculated_gpa =
                        (average_grade / 100.0) * 4.0;
                }

                fprintf(
                    out,
                    "      average_grade: %.2f,\n",
                    average_grade
                );

                fprintf(
                    out,
                    "      calculated_gpa: %.2f,\n",
                    calculated_gpa
                );

                /*
                ------------------------------------------
                PART D: Get attendance
                ------------------------------------------
                */

                fprintf(out, "      attendance: [\n");

                total_days = 0;
                present_days = 0;

                {
                    const char *attendance_sql =
                        "SELECT date, status "
                        "FROM attendance "
                        "WHERE student_id = ? "
                        "ORDER BY date DESC;";

                    if (
                        sqlite3_prepare_v2(
                            db,
                            attendance_sql,
                            -1,
                            &attendance_stmt,
                            NULL
                        ) == SQLITE_OK
                    )
                    {
                        sqlite3_bind_int(
                            attendance_stmt,
                            1,
                            student_id
                        );

                        while (
                            sqlite3_step(attendance_stmt) ==
                            SQLITE_ROW
                        )
                        {
                            const unsigned char *date_value;
                            const unsigned char *status_value;

                            const char *date;
                            const char *status;

                            date_value =
                                sqlite3_column_text(
                                    attendance_stmt,
                                    0
                                );

                            status_value =
                                sqlite3_column_text(
                                    attendance_stmt,
                                    1
                                );

                            date =
                                date_value != NULL
                                    ? (const char *) date_value
                                    : "";

                            status =
                                status_value != NULL
                                    ? (const char *) status_value
                                    : "Unknown";

                            fprintf(
                                out,
                                "        { date: \""
                            );

                            write_js_string(out, date);

                            fprintf(
                                out,
                                "\", status: \""
                            );

                            write_js_string(out, status);

                            fprintf(out, "\" },\n");

                            total_days++;

                            if (
                                strcmp(
                                    status,
                                    "Present"
                                ) == 0
                            )
                            {
                                present_days++;
                            }
                        }
                    }
                    else
                    {
                        fprintf(
                            stderr,
                            "[Error] Attendance query failed: %s\n",
                            sqlite3_errmsg(db)
                        );
                    }
                }

                if (attendance_stmt != NULL)
                {
                    sqlite3_finalize(attendance_stmt);
                    attendance_stmt = NULL;
                }

                fprintf(out, "      ],\n");

                attendance_rate = 0.0;

                if (total_days > 0)
                {
                    attendance_rate =
                        (present_days * 100.0) /
                        total_days;
                }

                fprintf(
                    out,
                    "      attendance_rate: %.1f\n",
                    attendance_rate
                );

                fprintf(out, "    },\n");
            }
        }
        else
        {
            fprintf(
                stderr,
                "[Error] Student query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    if (student_stmt != NULL)
    {
        sqlite3_finalize(student_stmt);
        student_stmt = NULL;
    }

    fprintf(out, "  ],\n");

    /*
    --------------------------------------------------
    PART E: Get school announcements
    --------------------------------------------------
    */

    fprintf(out, "  announcements: [\n");

    {
        const char *announcement_sql =
            "SELECT title, message "
            "FROM announcements "
            "ORDER BY id DESC;";

        if (
            sqlite3_prepare_v2(
                db,
                announcement_sql,
                -1,
                &announcement_stmt,
                NULL
            ) == SQLITE_OK
        )
        {
            while (
                sqlite3_step(announcement_stmt) ==
                SQLITE_ROW
            )
            {
                const unsigned char *title_value;
                const unsigned char *message_value;

                const char *title;
                const char *message;

                title_value =
                    sqlite3_column_text(
                        announcement_stmt,
                        0
                    );

                message_value =
                    sqlite3_column_text(
                        announcement_stmt,
                        1
                    );

                title =
                    title_value != NULL
                        ? (const char *) title_value
                        : "";

                message =
                    message_value != NULL
                        ? (const char *) message_value
                        : "";

                fprintf(out, "    { title: \"");
                write_js_string(out, title);

                fprintf(out, "\", message: \"");
                write_js_string(out, message);

                fprintf(out, "\" },\n");
            }
        }
        else
        {
            fprintf(
                stderr,
                "[Error] Announcement query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    if (announcement_stmt != NULL)
    {
        sqlite3_finalize(announcement_stmt);
        announcement_stmt = NULL;
    }

    fprintf(out, "  ]\n");
    fprintf(out, "};\n");

    fclose(out);
    sqlite3_close(db);

    printf(
        "[Parent Logic] Successfully created parent_state.js "
        "for parent user %d.\n",
        parent_user_id
    );
}


int main(void)
{
    /*
    This must be users.id for a user whose role is parent.

    Example:
    users.id = 1
    users.role = "parent"
    */
    update_parent_frontend_state(1);

    return 0;
}