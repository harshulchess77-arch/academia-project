/*
Seungho Kim
CS50 C Project
Parent Functions
10.27.26
*/

#include <stdio.h>   // Provides file handling and input/output functions.
#include <string.h>  // Provides string comparison functions.
#include <sqlite3.h> // Provides SQLite database functions.


/*
Escapes special characters so text can be safely written
inside a JavaScript string.
*/
void write_js_string(FILE *out, const char *text)
{
    int i; // Stores the current position in the string.

    // Stop if there is no text to write.
    if (text == NULL)
    {
        return;
    }

    // Check each character in the string.
    for (i = 0; text[i] != '\0'; i++)
    {
        switch (text[i])
        {
            case '\\':
                // Escape a backslash.
                fprintf(out, "\\\\");
                break;

            case '"':
                // Escape a double quotation mark.
                fprintf(out, "\\\"");
                break;

            case '\n':
                // Convert a line break into a JavaScript newline.
                fprintf(out, "\\n");
                break;

            case '\r':
                // Convert a carriage return into a safe form.
                fprintf(out, "\\r");
                break;

            case '\t':
                // Convert a tab into a JavaScript tab.
                fprintf(out, "\\t");
                break;

            default:
                // Write a normal character without changing it.
                fputc(text[i], out);
                break;
        }
    }
}


/*
Reads parent portal data from academia.db and writes the data
to parent_state.js for the frontend to use.
*/
void update_parent_frontend_state(int parent_user_id)
{
    // Database connection.
    sqlite3 *db;

    // Prepared SQL statements for each type of data.
    sqlite3_stmt *parent_stmt;
    sqlite3_stmt *student_stmt;
    sqlite3_stmt *grade_stmt;
    sqlite3_stmt *attendance_stmt;
    sqlite3_stmt *announcement_stmt;

    // Output JavaScript file.
    FILE *out;

    // Stores the result of opening the database.
    int open_result;

    // Stores the parent's ID from the parents table.
    int parent_id;

    // Stores parent account information.
    char parent_username[100];
    char parent_first_name[100];
    char parent_last_name[100];

    // Initialize database and statement pointers.
    db = NULL;
    parent_stmt = NULL;
    student_stmt = NULL;
    grade_stmt = NULL;
    attendance_stmt = NULL;
    announcement_stmt = NULL;

    // Use -1 to indicate that no parent has been found yet.
    parent_id = -1;

    // Set a default username in case the database value is missing.
    snprintf(
        parent_username,
        sizeof(parent_username),
        "%s",
        "Unknown Parent"
    );

    // Start the name strings as empty strings.
    parent_first_name[0] = '\0';
    parent_last_name[0] = '\0';

    // Open the Academia SQLite database.
    open_result = sqlite3_open(
        "Academia/BackEnd/Academia Database/academia.db",
        &db
    );

    // Stop the function if the database cannot be opened.
    if (open_result != SQLITE_OK)
    {
        fprintf(
            stderr,
            "[Error] Cannot open database: %s\n",
            db != NULL
                ? sqlite3_errmsg(db)
                : "Unknown SQLite error"
        );

        // Close the database if SQLite created a database object.
        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return;
    }

    // Open parent_state.js and replace its previous contents.
    out = fopen(
        "Academia/FrontEnd/Academia Parents/parent_state.js",
        "w"
    );

    // Stop if the output file cannot be created.
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
    PART A: Find the parent account.
    */

    {
        // Select the parent connected to the provided users.id.
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

        // Prepare the SQL query before running it.
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
            // Replace the question mark with the parent user ID.
            sqlite3_bind_int(
                parent_stmt,
                1,
                parent_user_id
            );

            // Check whether the query returned a parent.
            if (sqlite3_step(parent_stmt) == SQLITE_ROW)
            {
                // Store temporary pointers to text returned by SQLite.
                const unsigned char *username_value;
                const unsigned char *first_name_value;
                const unsigned char *last_name_value;

                // Read the parent's username.
                username_value =
                    sqlite3_column_text(parent_stmt, 0);

                // Read the parent's parents-table ID.
                parent_id =
                    sqlite3_column_int(parent_stmt, 1);

                // Read the parent's first name.
                first_name_value =
                    sqlite3_column_text(parent_stmt, 2);

                // Read the parent's last name.
                last_name_value =
                    sqlite3_column_text(parent_stmt, 3);

                // Copy the username into a local character array.
                if (username_value != NULL)
                {
                    snprintf(
                        parent_username,
                        sizeof(parent_username),
                        "%s",
                        (const char *) username_value
                    );
                }

                // Copy the first name if it is not NULL.
                if (first_name_value != NULL)
                {
                    snprintf(
                        parent_first_name,
                        sizeof(parent_first_name),
                        "%s",
                        (const char *) first_name_value
                    );
                }

                // Copy the last name if it is not NULL.
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
                // Report that the provided user is not a parent.
                fprintf(
                    stderr,
                    "[Error] No parent account found for users.id = %d.\n",
                    parent_user_id
                );
            }
        }
        else
        {
            // Report an error if SQLite cannot prepare the query.
            fprintf(
                stderr,
                "[Error] Parent query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    // Release the completed parent query.
    if (parent_stmt != NULL)
    {
        sqlite3_finalize(parent_stmt);
        parent_stmt = NULL;
    }

    // Start the JavaScript parentState object.
    fprintf(out, "const parentState = {\n");

    // Write the parent user's users-table ID.
    fprintf(
        out,
        "  user_id: %d,\n",
        parent_user_id
    );

    // Write the parent's parents-table ID.
    fprintf(
        out,
        "  parent_id: %d,\n",
        parent_id
    );

    // Write the parent's username safely.
    fprintf(out, "  username: \"");
    write_js_string(out, parent_username);
    fprintf(out, "\",\n");

    // Write the parent's first name safely.
    fprintf(out, "  first_name: \"");
    write_js_string(out, parent_first_name);
    fprintf(out, "\",\n");

    // Write the parent's last name safely.
    fprintf(out, "  last_name: \"");
    write_js_string(out, parent_last_name);
    fprintf(out, "\",\n");

    /*
    PART B: Find students connected to the parent.
    */

    // Start the JavaScript students array.
    fprintf(out, "  students: [\n");

    {
        // Join the related tables to find the parent's students.
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

        // Prepare the student query.
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
            // Bind the parent's users-table ID to the query.
            sqlite3_bind_int(
                student_stmt,
                1,
                parent_user_id
            );

            // Process every student connected to the parent.
            while (sqlite3_step(student_stmt) == SQLITE_ROW)
            {
                int student_id;    // Stores the student's ID.
                int grade_level;   // Stores the student's grade level.

                double stored_gpa;      // GPA already stored in the database.
                double total_grade;     // Sum of all course grades.
                double average_grade;   // Average of all course grades.
                double calculated_gpa;  // GPA estimated from the average grade.
                double attendance_rate; // Percentage of present school days.

                int grade_count;  // Number of grade records.
                int total_days;   // Number of attendance records.
                int present_days; // Number of days marked present.

                // Temporary SQLite text pointers.
                const unsigned char *username_value;
                const unsigned char *first_name_value;
                const unsigned char *last_name_value;

                // Final text values used when writing JavaScript.
                const char *student_username;
                const char *student_first_name;
                const char *student_last_name;

                // Read the student's database ID.
                student_id =
                    sqlite3_column_int(student_stmt, 0);

                // Read the student's username.
                username_value =
                    sqlite3_column_text(student_stmt, 1);

                // Read the student's first name.
                first_name_value =
                    sqlite3_column_text(student_stmt, 2);

                // Read the student's last name.
                last_name_value =
                    sqlite3_column_text(student_stmt, 3);

                // Read the student's grade level.
                grade_level =
                    sqlite3_column_int(student_stmt, 4);

                // Read the student's stored GPA.
                stored_gpa =
                    sqlite3_column_double(student_stmt, 5);

                // Use default values if any text field is NULL.
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

                // Start one student object.
                fprintf(out, "    {\n");

                // Write the student's ID.
                fprintf(
                    out,
                    "      student_id: %d,\n",
                    student_id
                );

                // Write the student's username.
                fprintf(out, "      username: \"");
                write_js_string(out, student_username);
                fprintf(out, "\",\n");

                // Write the student's first name.
                fprintf(out, "      first_name: \"");
                write_js_string(out, student_first_name);
                fprintf(out, "\",\n");

                // Write the student's last name.
                fprintf(out, "      last_name: \"");
                write_js_string(out, student_last_name);
                fprintf(out, "\",\n");

                // Write the student's grade level.
                fprintf(
                    out,
                    "      grade_level: %d,\n",
                    grade_level
                );

                // Write the GPA stored in the students table.
                fprintf(
                    out,
                    "      stored_gpa: %.2f,\n",
                    stored_gpa
                );

                /*
                PART C: Get the student's grades.
                */

                // Start the student's grades array.
                fprintf(out, "      grades: [\n");

                // Reset grade calculations for the current student.
                total_grade = 0.0;
                grade_count = 0;

                {
                    // Select all grades belonging to this student.
                    const char *grade_sql =
                        "SELECT course, grade "
                        "FROM grades "
                        "WHERE student_id = ? "
                        "ORDER BY course;";

                    // Prepare the grade query.
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
                        // Bind the current student's ID.
                        sqlite3_bind_int(
                            grade_stmt,
                            1,
                            student_id
                        );

                        // Process each grade record.
                        while (
                            sqlite3_step(grade_stmt) == SQLITE_ROW
                        )
                        {
                            const unsigned char *course_value;
                            const char *course;
                            double grade;

                            // Read the course name.
                            course_value =
                                sqlite3_column_text(
                                    grade_stmt,
                                    0
                                );

                            // Read the numerical grade.
                            grade =
                                sqlite3_column_double(
                                    grade_stmt,
                                    1
                                );

                            // Use a default course name for NULL values.
                            course =
                                course_value != NULL
                                    ? (const char *) course_value
                                    : "Unknown Course";

                            // Start one grade object.
                            fprintf(
                                out,
                                "        { course: \""
                            );

                            // Write the course name safely.
                            write_js_string(out, course);

                            // Write the numerical grade and close the object.
                            fprintf(
                                out,
                                "\", grade: %.2f },\n",
                                grade
                            );

                            // Add this grade to the student's total.
                            total_grade += grade;

                            // Count this grade for the average.
                            grade_count++;
                        }
                    }
                    else
                    {
                        // Report a failure to prepare the grade query.
                        fprintf(
                            stderr,
                            "[Error] Grade query failed: %s\n",
                            sqlite3_errmsg(db)
                        );
                    }
                }

                // Release the completed grade query.
                if (grade_stmt != NULL)
                {
                    sqlite3_finalize(grade_stmt);
                    grade_stmt = NULL;
                }

                // Close the grades array.
                fprintf(out, "      ],\n");

                // Set default values in case the student has no grades.
                average_grade = 0.0;
                calculated_gpa = 0.0;

                // Calculate the average and estimated GPA.
                if (grade_count > 0)
                {
                    average_grade =
                        total_grade / grade_count;

                    calculated_gpa =
                        (average_grade / 100.0) * 4.0;
                }

                // Write the student's average grade.
                fprintf(
                    out,
                    "      average_grade: %.2f,\n",
                    average_grade
                );

                // Write the student's estimated GPA.
                fprintf(
                    out,
                    "      calculated_gpa: %.2f,\n",
                    calculated_gpa
                );

                /*
                PART D: Get the student's attendance.
                */

                // Start the student's attendance array.
                fprintf(out, "      attendance: [\n");

                // Reset attendance counters for the current student.
                total_days = 0;
                present_days = 0;

                {
                    // Select all attendance records for this student.
                    const char *attendance_sql =
                        "SELECT date, status "
                        "FROM attendance "
                        "WHERE student_id = ? "
                        "ORDER BY date DESC;";

                    // Prepare the attendance query.
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
                        // Bind the current student's ID.
                        sqlite3_bind_int(
                            attendance_stmt,
                            1,
                            student_id
                        );

                        // Process each attendance record.
                        while (
                            sqlite3_step(attendance_stmt) ==
                            SQLITE_ROW
                        )
                        {
                            const unsigned char *date_value;
                            const unsigned char *status_value;

                            const char *date;
                            const char *status;

                            // Read the attendance date.
                            date_value =
                                sqlite3_column_text(
                                    attendance_stmt,
                                    0
                                );

                            // Read the attendance status.
                            status_value =
                                sqlite3_column_text(
                                    attendance_stmt,
                                    1
                                );

                            // Use an empty string if the date is NULL.
                            date =
                                date_value != NULL
                                    ? (const char *) date_value
                                    : "";

                            // Use "Unknown" if the status is NULL.
                            status =
                                status_value != NULL
                                    ? (const char *) status_value
                                    : "Unknown";

                            // Start one attendance object.
                            fprintf(
                                out,
                                "        { date: \""
                            );

                            // Write the attendance date.
                            write_js_string(out, date);

                            // Write the beginning of the status property.
                            fprintf(
                                out,
                                "\", status: \""
                            );

                            // Write the attendance status.
                            write_js_string(out, status);

                            // Close the attendance object.
                            fprintf(out, "\" },\n");

                            // Count the attendance record as one school day.
                            total_days++;

                            // Count the day as present when the value matches.
                            if (strcmp(status, "Present") == 0)
                            {
                                present_days++;
                            }
                        }
                    }
                    else
                    {
                        // Report a failure to prepare the attendance query.
                        fprintf(
                            stderr,
                            "[Error] Attendance query failed: %s\n",
                            sqlite3_errmsg(db)
                        );
                    }
                }

                // Release the completed attendance query.
                if (attendance_stmt != NULL)
                {
                    sqlite3_finalize(attendance_stmt);
                    attendance_stmt = NULL;
                }

                // Close the attendance array.
                fprintf(out, "      ],\n");

                // Use zero when there are no attendance records.
                attendance_rate = 0.0;

                // Calculate the percentage of days marked present.
                if (total_days > 0)
                {
                    attendance_rate =
                        (present_days * 100.0) /
                        total_days;
                }

                // Write the student's attendance percentage.
                fprintf(
                    out,
                    "      attendance_rate: %.1f\n",
                    attendance_rate
                );

                // Close the current student object.
                fprintf(out, "    },\n");
            }
        }
        else
        {
            // Report a failure to prepare the student query.
            fprintf(
                stderr,
                "[Error] Student query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    // Release the completed student query.
    if (student_stmt != NULL)
    {
        sqlite3_finalize(student_stmt);
        student_stmt = NULL;
    }

    // Close the students array.
    fprintf(out, "  ],\n");

    /*
    PART E: Get school announcements.
    */

    // Start the announcements array.
    fprintf(out, "  announcements: [\n");

    {
        // Select announcements from newest to oldest.
        const char *announcement_sql =
            "SELECT title, message "
            "FROM announcements "
            "ORDER BY id DESC;";

        // Prepare the announcement query.
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
            // Process every announcement.
            while (
                sqlite3_step(announcement_stmt) ==
                SQLITE_ROW
            )
            {
                const unsigned char *title_value;
                const unsigned char *message_value;

                const char *title;
                const char *message;

                // Read the announcement title.
                title_value =
                    sqlite3_column_text(
                        announcement_stmt,
                        0
                    );

                // Read the announcement message.
                message_value =
                    sqlite3_column_text(
                        announcement_stmt,
                        1
                    );

                // Replace a NULL title with an empty string.
                title =
                    title_value != NULL
                        ? (const char *) title_value
                        : "";

                // Replace a NULL message with an empty string.
                message =
                    message_value != NULL
                        ? (const char *) message_value
                        : "";

                // Start one announcement object.
                fprintf(out, "    { title: \"");

                // Write the announcement title safely.
                write_js_string(out, title);

                // Start the message property.
                fprintf(out, "\", message: \"");

                // Write the announcement message safely.
                write_js_string(out, message);

                // Close the announcement object.
                fprintf(out, "\" },\n");
            }
        }
        else
        {
            // Report a failure to prepare the announcement query.
            fprintf(
                stderr,
                "[Error] Announcement query failed: %s\n",
                sqlite3_errmsg(db)
            );
        }
    }

    // Release the completed announcement query.
    if (announcement_stmt != NULL)
    {
        sqlite3_finalize(announcement_stmt);
        announcement_stmt = NULL;
    }

    // Close the announcements array and parentState object.
    fprintf(out, "  ]\n");
    fprintf(out, "};\n");

    // Save and close parent_state.js.
    fclose(out);

    // Close the SQLite database connection.
    sqlite3_close(db);

    // Confirm that the JavaScript file was generated.
    printf(
        "[Parent Logic] Successfully created parent_state.js "
        "for parent user %d.\n",
        parent_user_id
    );
}


int main(void)
{
    /*
    Generate frontend data for the parent whose users.id is 1.
    The selected user must have the role "parent".
    */
    update_parent_frontend_state(1);

    // End the program successfully.
    return 0;
}