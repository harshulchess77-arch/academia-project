/*
Harshul Gupta
Cs50 c project
Student Module
10.20.26
*/

#include <stdio.h>
#include <string.h>
#include "../Academia Database/SQLite3/sqlite3.h"

char find_letter(int grade)
{
    if (grade >= 90) return 'A';
    if (grade >= 80) return 'B';
    if (grade >= 70) return 'C';
    if (grade >= 60) return 'D';
    return 'F';
}

void run_student_module(int student_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char sql[200];

    if (sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Connection Failed\n");
        return;
    }

    FILE *js = fopen("Academia/FrontEnd/Academia.js", "w");
    if (js == NULL)
    {
        printf("Could not open Academia.js\n");
        sqlite3_close(db);
        return;
    }

    // 1. Grades and GPA Logic
    int sum = 0;
    int count = 0;

    fprintf(js, "document.getElementById('grades-tbody').innerHTML = '';\n");

    sprintf(sql, "SELECT course, grade FROM grades WHERE student_id = %d;", student_id);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *course = (const char *)sqlite3_column_text(stmt, 0);
            int grade = sqlite3_column_int(stmt, 1);
            char letter = find_letter(grade);

            sum = sum + grade;
            count = count + 1;

            fprintf(js, "document.getElementById('grades-tbody').innerHTML += '<tr><td>%s</td><td>%d%%</td><td>%c</td></tr>';\n",
                    course, grade, letter);
        }
    }
    sqlite3_finalize(stmt);

    if (count > 0)
    {
        float avg = (float)sum / count;
        float gpa = (avg / 100.0) * 4.0;

        fprintf(js, "document.getElementById('avg-val').innerHTML = '%.1f%%';\n", avg);
        fprintf(js, "document.getElementById('gpa-val').innerHTML = '%.2f';\n", gpa);

        // Update the student's GPA in the students table
        sprintf(sql, "UPDATE students SET gpa = %.2f WHERE id = %d;", gpa, student_id);
        sqlite3_exec(db, sql, 0, 0, 0);
    }

    // 2. Attendance Logic
    int present = 0;
    int total = 0;

    fprintf(js, "document.getElementById('att-tbody').innerHTML = '';\n");

    sprintf(sql, "SELECT date, status FROM attendance WHERE student_id = %d;", student_id);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *date = (const char *)sqlite3_column_text(stmt, 0);
            const char *status = (const char *)sqlite3_column_text(stmt, 1);

            total = total + 1;
            if (strcmp(status, "Present") == 0)
            {
                present = present + 1;
            }

            fprintf(js, "document.getElementById('att-tbody').innerHTML += '<tr><td>%s</td><td>%s</td></tr>';\n",
                    date, status);
        }
    }
    sqlite3_finalize(stmt);

    if (total > 0)
    {
        float rate = (present * 100.0) / total;
        fprintf(js, "document.getElementById('att-val').innerHTML = '%.0f%%';\n", rate);
    }

    // 3. Announcements Logic
    fprintf(js, "document.getElementById('ann-div').innerHTML = '';\n");

    if (sqlite3_prepare_v2(db, "SELECT title, message FROM announcements;", -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *title = (const char *)sqlite3_column_text(stmt, 0);
            const char *msg = (const char *)sqlite3_column_text(stmt, 1);

            fprintf(js, "document.getElementById('ann-div').innerHTML += '<p><b>%s:</b> %s</p>';\n",
                    title, msg);
        }
    }
    sqlite3_finalize(stmt);

    fclose(js);
    sqlite3_close(db);
    printf("Student C logic finished and updated Academia.js\n");
}