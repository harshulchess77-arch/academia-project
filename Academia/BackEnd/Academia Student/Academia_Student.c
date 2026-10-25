/*
Harshul Gupta
Cs50 c project
Student functions
10.22.26
*/

#include <stdio.h>
#include <string.h>
// #include <cs50.h> The reason of commiting this out is becuase I am not using this on cs50.dev yet
//so it will provide errors when we compile it on my local machine. I will uncomment it when I will use it on cs50.dev

//Main Method (FOR STRUCTRE AND DATABASE CONNECTION WITH BACKEND TO FRONTEND AND VICE VERSA)
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

// Harshul's Student Logic Engine:
// Queries SQLite, calculates GPA & Attendance %, and sends state to the HTML/JS Frontend
void update_student_frontend_state(const char *student_id) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    // 1. Open the single SQLite3 database
    if (sqlite3_open("Academia/BackEnd/Academia Database/academia.db", &db) != SQLITE_OK) {
        printf("[Error] Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // 2. Open the JS state file that Academia_Student.html will read
    FILE *out = fopen("Academia/FrontEnd/Academia Student/student_state.js", "w");
    if (out == NULL) {
        printf("[Error] Cannot create frontend state file.\n");
        sqlite3_close(db);
        return;
    }

    fprintf(out, "const studentState = {\n");
    fprintf(out, "  student_id: \"%s\",\n", student_id);

    // ---------------------------------------------------------
    // PART A: Query Grades & Calculate GPA in C
    // ---------------------------------------------------------
    fprintf(out, "  grades: [\n");
    char sql_grades[200];
    snprintf(sql_grades, sizeof(sql_grades),
             "SELECT course, score FROM Grades WHERE student_id = '%s';", student_id);

    float total_score = 0.0f;
    int grade_count = 0;

    if (sqlite3_prepare_v2(db, sql_grades, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *course = (const char *)sqlite3_column_text(stmt, 0);
            float score = (float)sqlite3_column_double(stmt, 1);

            fprintf(out, "    { course: \"%s\", score: %.2f },\n", course, score);
            total_score += score;
            grade_count++;
        }
    }
    sqlite3_finalize(stmt);
    fprintf(out, "  ],\n");

    // CS50 GPA Calculation Logic (4.0 Scale)
    float average_score = 0.0f;
    float gpa = 0.0f;
    if (grade_count > 0) {
        average_score = total_score / grade_count;
        gpa = (average_score / 100.0f) * 4.0f;
    }
    fprintf(out, "  average_score: %.2f,\n", average_score);
    fprintf(out, "  gpa: %.2f,\n", gpa);

    // Attendance & Calculate Attendance Rate in C
    fprintf(out, "  attendance: [\n");
    char sql_att[200];
    snprintf(sql_att, sizeof(sql_att),
            "SELECT date, status FROM Attendance WHERE student_id = '%s';", student_id);

    int total_days = 0;
    int present_days = 0;

    if (sqlite3_prepare_v2(db, sql_att, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *date = (const char *)sqlite3_column_text(stmt, 0);
            const char *status = (const char *)sqlite3_column_text(stmt, 1);

            fprintf(out, "    { date: \"%s\", status: \"%s\" },\n", date, status);
            total_days++;
            if (strcmp(status, "Present") == 0) {
                present_days++;
            }
        }
    }
    sqlite3_finalize(stmt);
    fprintf(out, "  ],\n");

    float attendance_rate = 0.0f;
    if (total_days > 0) {
        attendance_rate = (present_days * 100.0f) / total_days;
    }
    fprintf(out, "  attendance_rate: %.1f,\n", attendance_rate);

    // School Announcements (Static for now, but can be dynamic in future)
    fprintf(out, "  announcements: [\n");
    const char *sql_ann = "SELECT date, message FROM Announcements;";

    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *date = (const char *)sqlite3_column_text(stmt, 0);
            const char *msg = (const char *)sqlite3_column_text(stmt, 1);

            fprintf(out, "    { date: \"%s\", message: \"%s\" },\n", date, msg);
        }
    }
    sqlite3_finalize(stmt);
    fprintf(out, "  ]\n};\n");

    fclose(out);
    sqlite3_close(db);

    printf("[Student Logic] Successfully updated frontend state for %s!\n", student_id);
}

int main(void) {
    update_student_frontend_state("STU001");
    return 0;
}