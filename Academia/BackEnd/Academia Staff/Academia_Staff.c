/*
Lawrence Mao
CS50 c Project
School-Staff Functions
10.22.26
*/

#include <stdio.h>
#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

void write_js_text(FILE *out, const unsigned char *text)
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

void update_staff_frontend_state(int staff_user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    FILE *out;

    int staff_found = 0;

    sqlite3_open(
        "Academia/BackEnd/Academia Database/academia.db",
        &db
    );

    out = fopen(
        "Academia/FrontEnd/Staff/staff_state.js",
        "w"
    );

    fprintf(out, "const staffState = {\n");

    /*
        STAFF INFO
    */

    fprintf(out, " staff:{\n");

    const char *sql_staff =
        "SELECT "
        "user.id,"
        "user.first_name,"
        "user.last_name,"
        "staff.phone "
        "FROM user "
        "INNER JOIN staff "
        "ON user.id = staff.id "
        "WHERE user.id=?;";

    sqlite3_prepare_v2(
        db,
        sql_staff,
        -1,
        &stmt,
        NULL
    );

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
            "id:%d,\n",
            sqlite3_column_int(stmt,0)
        );

        fprintf(out,"firstName:\"");
        write_js_text(
            out,
            sqlite3_column_text(stmt,1)
        );

        fprintf(out,"\",\n");

        fprintf(out,"lastName:\"");
        write_js_text(
            out,
            sqlite3_column_text(stmt,2)
        );

        fprintf(out,"\",\n");

        fprintf(out,"phone:\"");
        write_js_text(
            out,
            sqlite3_column_text(stmt,3)
        );

        fprintf(out,"\"\n");
    }

    sqlite3_finalize(stmt);

    fprintf(out,"},\n");

    /*
        COURSES
    */

    fprintf(out,"courses:[\n");

    const char *sql_courses =
        "SELECT "
        "id,"
        "course_name "
        "FROM course "
        "WHERE staff_id=?;";

    sqlite3_prepare_v2(
        db,
        sql_courses,
        -1,
        &stmt,
        NULL
    );

    sqlite3_bind_int(stmt,1,staff_user_id);

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        fprintf(
            out,
            "{courseId:%d,course:\"",
            sqlite3_column_int(stmt,0)
        );

        write_js_text(
            out,
            sqlite3_column_text(stmt,1)
        );

        fprintf(out,"\"},\n");
    }

    sqlite3_finalize(stmt);

    fprintf(out,"],\n");

    /*
        GRADES
    */

    fprintf(out,"grades:[\n");

    const char *sql_grades =
        "SELECT "
        "user.first_name,"
        "user.last_name,"
        "course.course_name,"
        "grade.score "
        "FROM grade "
        "INNER JOIN student "
        "ON grade.student_id=student.id "
        "INNER JOIN user "
        "ON student.id=user.id "
        "INNER JOIN course "
        "ON grade.course_id=course.id "
        "WHERE course.staff_id=?;";

    sqlite3_prepare_v2(
        db,
        sql_grades,
        -1,
        &stmt,
        NULL
    );

    sqlite3_bind_int(stmt,1,staff_user_id);

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        fprintf(out,"{student:\"");

        write_js_text(
            out,
            sqlite3_column_text(stmt,0)
        );

        fprintf(out," ");

        write_js_text(
            out,
            sqlite3_column_text(stmt,1)
        );

        fprintf(out,"\",course:\"");

        write_js_text(
            out,
            sqlite3_column_text(stmt,2)
        );

        fprintf(
            out,
            "\",grade:%.2f},\n",
            sqlite3_column_double(stmt,3)
        );
    }

    sqlite3_finalize(stmt);

    fprintf(out,"],\n");

    /*
        ATTENDANCE
    */

    fprintf(out,"attendance:[\n");

    const char *sql_attendance =
        "SELECT "
        "user.first_name,"
        "user.last_name,"
        "attendance.present "
        "FROM attendance "
        "INNER JOIN student "
        "ON attendance.student_id = student.id "
        "INNER JOIN user "
        "ON student.id=user.id "
        "INNER JOIN course "
        "ON attendance.course_id=course.id "
        "WHERE course.staff_id=?;";

    sqlite3_prepare_v2(
        db,
        sql_attendance,
        -1,
        &stmt,
        NULL
    );

    sqlite3_bind_int(stmt,1,staff_user_id);

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        fprintf(
            out,
            "{student:\"%s %s\",status:\"%s\"},\n",
            sqlite3_column_text(stmt,0),
            sqlite3_column_text(stmt,1),
            sqlite3_column_int(stmt,2)
                ? "Present"
                : "Absent"
        );
    }

    sqlite3_finalize(stmt);

    fprintf(out,"],\n");

    /*
        ANNOUNCEMENTS
    */

    fprintf(out,"announcements:[\n");

    const char *sql_announcement =
        "SELECT "
        "title,"
        "message,"
        "announcement_date "
        "FROM announcement "
        "ORDER BY id DESC;";

    sqlite3_prepare_v2(
        db,
        sql_announcement,
        -1,
        &stmt,
        NULL
    );

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        fprintf(out,"{title:\"");

        write_js_text(
            out,
            sqlite3_column_text(stmt,0)
        );

        fprintf(out,"\",message:\"");

        write_js_text(
            out,
            sqlite3_column_text(stmt,1)
        );

        fprintf(out,"\",date:\"");

        write_js_text(
            out,
            sqlite3_column_text(stmt,2)
        );

        fprintf(out,"\"},\n");
    }

    sqlite3_finalize(stmt);

    fprintf(out,"],\n");

    fprintf(
        out,
        "success:%s\n",
        staff_found ? "true" : "false"
    );

    fprintf(out,"};");

    fclose(out);

    sqlite3_close(db);

    printf(
        "[Staff Logic] staff_state.js updated.\n"
    );
}