/*
Lawrence Mao
CS50 c Project
School-Staff Functions
10.22.26
*/

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

/*
This file contains the c functions of school staff that includes
the attendence system, the grade system, and the connection between
c and database (write) which push the records into the frontface.
*/

// Adds a grade record to the database
void add_grade(char student_id[], char course[], int grade)
{
    //point to the database file
    sqlite3 *db;

    //create the string that can store up to 511 characters
    char sql[512];

    //open the sqlite file
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK)
    {
        printf("[Error] Database connection failed.\n");
        return;
    }

    //insert the records given by the teacher into the database
    snprintf(sql,
             sizeof(sql),
             "INSERT INTO grades"
             "(student_id, course, grade)"
             "VALUES('%s','%s',%d);",
             student_id,
             course,
             grade);

    //execute the sql code
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    //close the sql
    sqlite3_close(db);

    printf("[Staff Logic] Grade successfully saved.\n");
}

// Records student attendance
void add_attendance(char student_id[], char date[], char status[])
{
    sqlite3 *db;
    char sql[512];

    //open the sql file
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK)
    {
        printf("[Error] Database connection failed.\n");
        return;
    }

    //insert the new records
    snprintf(sql,
             sizeof(sql),
             "INSERT INTO attendance "
             "(student_id, date, status) "
             "VALUES('%s','%s','%s');",
             student_id,
             date,
             status);

    //run the sql lines
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    //close the database
    sqlite3_close(db);

    printf("[Staff Logic] Attendance successfully recorded.\n");
}

// Publishes a school announcement
void add_announcement(char title[], char message[])
{
    sqlite3 *db;
    char sql[1000];

    //open the sql file
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK)
    {
        printf("[Error] Database connection failed.\n");
        return;
    }

    //insert the new announcements records
    snprintf(sql,
             sizeof(sql),
             "INSERT INTO announcements "
             "(title, message) "
             "VALUES('%s','%s');",
             title,
             message);

    //run the sql lines
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    //close the database
    sqlite3_close(db);

    printf("[Staff Logic] Announcement successfully published.\n");
}


// Creates the JavaScript state file used by the Staff frontend
void update_staff_frontend_state(void)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK)
    {
        printf("[Error] Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    //create the file
    FILE *out = fopen("Academia/FrontEnd/Academia School Staff/staff_state.js", "w");

    if (out == NULL)
    {
        printf("[Error] Cannot create frontend state file.\n", sqlite3_close(db)); 
        return;
    }

    fprintf(out, "const staffState = {\n");

    // Export all grades
    fprintf(out, "  grades: [\n");

    const char *sql_grades =
        "SELECT student_id, course, grade "
        "FROM grades;";

    if (sqlite3_prepare_v2(
            db,
            sql_grades,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out,
                    "    { studentId: \"%s\", course: \"%s\", score: %d },\n",
                    sqlite3_column_text(stmt, 0),
                    sqlite3_column_text(stmt, 1),
                    sqlite3_column_int(stmt, 2));
        }
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ],\n");

    // Export attendance records
    fprintf(out, "  attendance: [\n");

    const char *sql_att =
        "SELECT student_id, date, status "
        "FROM attendance;";

    if (sqlite3_prepare_v2(
            db,
            sql_att,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out,
                    "    { studentId: \"%s\", date: \"%s\", status: \"%s\" },\n",
                    sqlite3_column_text(stmt, 0),
                    sqlite3_column_text(stmt, 1),
                    sqlite3_column_text(stmt, 2));
        }
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ],\n");

    // Export announcements
    fprintf(out, "  announcements: [\n");

    const char *sql_ann =
        "SELECT title, message "
        "FROM announcements;";

    if (sqlite3_prepare_v2(
            db,
            sql_ann,
            -1,
            &stmt,
            NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            fprintf(out,
                    "    { title: \"%s\", message: \"%s\" },\n",
                    sqlite3_column_text(stmt, 0),
                    sqlite3_column_text(stmt, 1));
        }
    }

    sqlite3_finalize(stmt);

    fprintf(out, "  ]\n");
    fprintf(out, "};\n");

    fclose(out);
    sqlite3_close(db);

    printf("[Staff Logic] Successfully updated frontend state.\n");
}
