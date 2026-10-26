/*
Seungho Kim
CS50 C Project
Parent Functions
10.26.26
*/

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

/*
Parent Logic Engine

1. Finds the parent account
2. Finds students connected to the parent
3. Retrieves each student's grades
4. Calculates average score and GPA
5. Retrieves attendance records
6. Calculates attendance percentage
7. Retrieves school announcements
8. Writes the information to parent_state.js
*/

void update_parent_frontend_state(int parent_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    /* Open the SQLite database */
    if (sqlite3_open(
            "Academia/BackEnd/Academia Database/academia.db",
            &db
        ) != SQLITE_OK)
    {
        printf(
            "[Error] Cannot open database: %s\n",
            sqlite3_errmsg(db)
        );

        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return;
    }

    /* Create the JavaScript state file */
    FILE *out = fopen(
        "Academia/FrontEnd/Academia Parent/parent_state.js",
        "w"
    );

    if (out == NULL)
    {
        printf("[Error] Cannot create parent_state.js.\n");
        sqlite3_close(db);
        return;
    }

    /*
    --------------------------------------------------
    PART A: Get the parent's username
    --------------------------------------------------
    */

    const char *parent_username = "Unknown Parent";

    const char *parent_sql =
        "SELECT username "
        "FROM users "
        "WHERE user_id = ? AND role = 'parent';";

    if (sqlite3_prepare_v2(
            db,
            parent_sql,
            -1,
            &stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, parent_id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *username =
                sqlite3_column_text(stmt, 0);

            if (username != NULL)
            {
                parent_username = (const char *) username;
            }
        }
    }
    else
    {
        printf(
            "[Error] Parent query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    /*
    Copy the username because SQLite column values may
    become invalid after sqlite3_finalize().
    */
    char parent_username_copy[100];

    snprintf(
        parent_username_copy,
        sizeof(parent_username_copy),
        "%s",
        parent_username
    );

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    /* Begin the JavaScript object */
    fprintf(out, "const parentState = {\n");
    fprintf(out, "  parent_id: %d,\n", parent_id);

    fprintf(
        out,
        "  username: \"%s\",\n",
        parent_username_copy
    );

    /*
    --------------------------------------------------
    PART B: Find students connected to the parent
    --------------------------------------------------
    */

    fprintf(out, "  students: [\n");

    const char *student_sql =
        "SELECT "
        "students.student_id, "
        "users.username, "
        "students.grade_level, "
        "students.gpa "
        "FROM parent_student "
        "INNER JOIN students "
        "ON parent_student.student_id = students.student_id "
        "INNER JOIN users "
        "ON students.user_id = users.user_id "
        "WHERE parent_student.parent_id = ?;";

    sqlite3_stmt *student_stmt = NULL;

    if (sqlite3_prepare_v2(
            db,
            student_sql,
            -1,
            &student_stmt,
            NULL
        ) == SQLITE_OK)
    {
        sqlite3_bind_int(student_stmt, 1, parent_id);

        while (sqlite3_step(student_stmt) == SQLITE_ROW)
        {
            int student_id =
                sqlite3_column_int(student_stmt, 0);

            const unsigned char *username_value =
                sqlite3_column_text(student_stmt, 1);

            int grade_level =
                sqlite3_column_int(student_stmt, 2);

            double stored_gpa =
                sqlite3_column_double(student_stmt, 3);

            const char *student_username =
                username_value != NULL
                    ? (const char *) username_value
                    : "Unknown Student";

            fprintf(out, "    {\n");
            fprintf(out, "      student_id: %d,\n", student_id);

            fprintf(
                out,
                "      username: \"%s\",\n",
                student_username
            );

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
            --------------------------------------------------
            PART C: Get grades and calculate GPA
            --------------------------------------------------
            */

            fprintf(out, "      grades: [\n");

            const char *grade_sql =
                "SELECT course_name, score "
                "FROM grades "
                "WHERE student_id = ?;";

            sqlite3_stmt *grade_stmt = NULL;

            double total_score = 0.0;
            int grade_count = 0;

            if (sqlite3_prepare_v2(
                    db,
                    grade_sql,
                    -1,
                    &grade_stmt,
                    NULL
                ) == SQLITE_OK)
            {
                sqlite3_bind_int(
                    grade_stmt,
                    1,
                    student_id
                );

                while (sqlite3_step(grade_stmt) == SQLITE_ROW)
                {
                    const unsigned char *course_value =
                        sqlite3_column_text(grade_stmt, 0);

                    double score =
                        sqlite3_column_double(grade_stmt, 1);

                    const char *course_name =
                        course_value != NULL
                            ? (const char *) course_value
                            : "Unknown Course";

                    fprintf(
                        out,
                        "        { course_name: \"%s\", "
                        "score: %.2f },\n",
                        course_name,
                        score
                    );

                    total_score += score;
                    grade_count++;
                }
            }
            else
            {
                printf(
                    "[Error] Grade query failed: %s\n",
                    sqlite3_errmsg(db)
                );
            }

            if (grade_stmt != NULL)
            {
                sqlite3_finalize(grade_stmt);
            }

            fprintf(out, "      ],\n");

            double average_score = 0.0;
            double calculated_gpa = 0.0;

            if (grade_count > 0)
            {
                average_score =
                    total_score / grade_count;

                calculated_gpa =
                    (average_score / 100.0) * 4.0;
            }

            fprintf(
                out,
                "      average_score: %.2f,\n",
                average_score
            );

            fprintf(
                out,
                "      calculated_gpa: %.2f,\n",
                calculated_gpa
            );

            /*
            --------------------------------------------------
            PART D: Get attendance and calculate rate
            --------------------------------------------------
            */

            fprintf(out, "      attendance: [\n");

            const char *attendance_sql =
                "SELECT date, status "
                "FROM attendance "
                "WHERE student_id = ?;";

            sqlite3_stmt *attendance_stmt = NULL;

            int total_days = 0;
            int present_days = 0;

            if (sqlite3_prepare_v2(
                    db,
                    attendance_sql,
                    -1,
                    &attendance_stmt,
                    NULL
                ) == SQLITE_OK)
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
                    const unsigned char *date_value =
                        sqlite3_column_text(
                            attendance_stmt,
                            0
                        );

                    const unsigned char *status_value =
                        sqlite3_column_text(
                            attendance_stmt,
                            1
                        );

                    const char *date =
                        date_value != NULL
                            ? (const char *) date_value
                            : "";

                    const char *status =
                        status_value != NULL
                            ? (const char *) status_value
                            : "Unknown";

                    fprintf(
                        out,
                        "        { date: \"%s\", "
                        "status: \"%s\" },\n",
                        date,
                        status
                    );

                    total_days++;

                    if (strcmp(status, "Present") == 0)
                    {
                        present_days++;
                    }
                }
            }
            else
            {
                printf(
                    "[Error] Attendance query failed: %s\n",
                    sqlite3_errmsg(db)
                );
            }

            if (attendance_stmt != NULL)
            {
                sqlite3_finalize(attendance_stmt);
            }

            fprintf(out, "      ],\n");

            double attendance_rate = 0.0;

            if (total_days > 0)
            {
                attendance_rate =
                    (present_days * 100.0) / total_days;
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
        printf(
            "[Error] Student query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (student_stmt != NULL)
    {
        sqlite3_finalize(student_stmt);
    }

    fprintf(out, "  ],\n");

    /*
    --------------------------------------------------
    PART E: Get announcements
    --------------------------------------------------
    */

    fprintf(out, "  announcements: [\n");

    const char *announcement_sql =
        "SELECT title, content "
        "FROM announcements;";

    sqlite3_stmt *announcement_stmt = NULL;

    if (sqlite3_prepare_v2(
            db,
            announcement_sql,
            -1,
            &announcement_stmt,
            NULL
        ) == SQLITE_OK)
    {
        while (
            sqlite3_step(announcement_stmt) ==
            SQLITE_ROW
        )
        {
            const unsigned char *title_value =
                sqlite3_column_text(
                    announcement_stmt,
                    0
                );

            const unsigned char *content_value =
                sqlite3_column_text(
                    announcement_stmt,
                    1
                );

            const char *title =
                title_value != NULL
                    ? (const char *) title_value
                    : "";

            const char *content =
                content_value != NULL
                    ? (const char *) content_value
                    : "";

            fprintf(
                out,
                "    { title: \"%s\", content: \"%s\" },\n",
                title,
                content
            );
        }
    }
    else
    {
        printf(
            "[Error] Announcement query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }

    if (announcement_stmt != NULL)
    {
        sqlite3_finalize(announcement_stmt);
    }

    fprintf(out, "  ]\n");
    fprintf(out, "};\n");

    fclose(out);
    sqlite3_close(db);

    printf(
        "[Parent Logic] Successfully updated frontend "
        "state for parent %d!\n",
        parent_id
    );
}

int main(void)
{
    /*
    The value must match a parent user's user_id
    in the users table.
    */
    update_parent_frontend_state(1);

    return 0;
}