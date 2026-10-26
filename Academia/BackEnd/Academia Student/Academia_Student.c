/* Harshul Gupta */

#include <stdio.h>
#include <string.h>
#include "../Academia Database/SQLite3/sqlite3.h"

void run_student_c_logic(sqlite3 *db) {
    sqlite3_stmt *stmt;
    FILE *js = fopen("Academia/FrontEnd/Academia.js", "w");
    if (js == NULL) {
        return;
    }

    float sum = 0.0;
    int count = 0;

    fprintf(js, "document.getElementById('student-grades').innerHTML = '';\n");

    // 1. Read Grades from SQLite and calculate GPA
    if (sqlite3_prepare_v2(db, "SELECT course, score FROM Grades WHERE student_id='STU01';", -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *course = (const char *)sqlite3_column_text(stmt, 0);
            float score = (float)sqlite3_column_double(stmt, 1);

            sum = sum + score;
            count = count + 1;

            fprintf(js, "document.getElementById('student-grades').innerHTML += '<tr><td>%s</td><td>%.0f%%</td></tr>';\n", course, score);
        }
    }
    sqlite3_finalize(stmt);

    if (count > 0) {
        float avg = sum / count;
        float gpa = (avg / 100.0) * 4.0;
        fprintf(js, "if(document.getElementById('avg-box')) document.getElementById('avg-box').innerHTML = '%.1f%%';\n", avg);
        fprintf(js, "if(document.getElementById('gpa-box')) document.getElementById('gpa-box').innerHTML = '%.2f';\n", gpa);
    }

    // 2. Read Attendance from SQLite and calculate rate
    int present = 0;
    int total = 0;
    fprintf(js, "document.getElementById('student-attendance').innerHTML = '';\n");

    if (sqlite3_prepare_v2(db, "SELECT date, status FROM Attendance WHERE student_id='STU01';", -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *date = (const char *)sqlite3_column_text(stmt, 0);
            const char *status = (const char *)sqlite3_column_text(stmt, 1);

            total = total + 1;
            if (strcmp(status, "Present") == 0) {
                present = present + 1;
            }

            fprintf(js, "document.getElementById('student-attendance').innerHTML += '<tr><td>%s</td><td>%s</td></tr>';\n", date, status);
        }
    }
    sqlite3_finalize(stmt);

    if (total > 0) {
        float rate = (present * 100.0) / total;
        fprintf(js, "if(document.getElementById('att-box')) document.getElementById('att-box').innerHTML = '%.0f%%';\n", rate);
    }

    fclose(js);
    printf("Student C logic finished and updated Academia.js\n");
}