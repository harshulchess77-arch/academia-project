/*
================================================================================
 Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
 CS50 C Project
 Academia - Staff Module  (Academia_Staff.c)
================================================================================

 FULL DOCUMENTATION
 ------------------

 1. PURPOSE
    Everything the Faculty Portal needs, in two halves:
      READ  update_staff_frontend_state()  writes staff_states.js for
            GET /api/staff-data.
      WRITE staff_save_grade() / staff_save_attendance()  store what the
            teacher enters, for POST /api/staff/grade and
            POST /api/staff/attendance.
    The WRITE half is NEW. The repository's server has no routes for it (see
    section 6), so saving only works after you add the route block.

 2. REQUEST FLOW
    Read:
      Browser (Academia.js, initStaff / refreshStaff)
        -> GET /api/staff-data  with  X-Auth-Token
      Academia_Server.c: get_token, auth_check (401 / 403 on failure)
        -> update_staff_frontend_state(user_id)          <-- THIS FILE
        -> sends Academia/FrontEnd/JS/staff_states.js
    Write:
      Browser (saveGrade / saveAttendance) -> POST JSON with X-Auth-Token
      Server: auth_check, role must be "staff"
        -> staff_save_grade(...) or staff_save_attendance(...)  <-- THIS FILE
        -> {"success":true} or an error
      Browser then reloads /api/staff-data so the tables show the database.

    SECURITY NOTE: the staff id always comes from the session. Every query is
    scoped by course.staff_id, and every write checks that the teacher owns the
    course and the student is enrolled in it.

 3. DATABASE TABLES USED (see Academia.sql)
      user        id, username, first_name, last_name
      staff       id (= user.id), phone
      course      id, staff_id, course_name          staff_id = the teacher
      enrollment  student_id, course_id, status      (1 = active)
      student     id, gpa                            gpa is updated on grade save
      grade       student_id, course_id, score       (0..100)
      attendance  student_id, course_id, present     (1 / 0, no date column)
      announcement course_id, title, message, announcement_date

 4. OUTPUT FILE SHAPE (staff_states.js)
      const staffState = {
        staff: { userId: 4, username: "staff1", firstName: "Dr. Eleanor",
                 lastName: "Vance", phone: "(555) 987-6543" },
        courses:  [ { courseId: 1, course: "Complete HTML Tutorial", students: 2 } ],
        students: [ { studentId: 1, name: "Alex Rivera", username: "student1",
                      course: "Complete HTML Tutorial", score: 97.0 } ],
        attendance: [ { course: "...", student: "Alex Rivera", present: 1 } ],
        announcements: [ { course: "...", title: "...", message: "...", date: "..." } ],
        success: true,
        staffId: 4
      };
    students[].score is "" when the student has no grade yet.

 5. FUNCTIONS
      static void write_list(...)           shared "SELECT -> JS array" writer
      int update_staff_frontend_state()     READ: build staff_states.js
      static int staff_owns_course()        guard: teacher owns the course
      static int student_enrolled()         guard: student is in the course
      int staff_save_grade()                WRITE: save grade, recompute GPA
      int staff_save_attendance()           WRITE: save attendance

 6. WIRING THE WRITE HALF INTO THE SERVER (not in the repository yet)
    a) Add to Academia_Staff.h:
         int staff_save_grade(int staff_id, int student_id, int course_id, double score);
         int staff_save_attendance(int staff_id, int student_id, int course_id, int present);
    b) Paste this inside handle_client_request() in Academia_Server.c, after the
       "Data pages" block and before the "/" redirect:

       if (strcmp(path, "/api/staff/grade") == 0 || strcmp(path, "/api/staff/attendance") == 0) {
           char token[128], role[16], name[128], sid[16], cid[16], val[16];
           char *body = strstr(buffer, "\r\n\r\n");
           int user_id, ok = 0;

           get_token(buffer, token, sizeof(token));
           if (!auth_check(token, &user_id, role, sizeof(role), name, sizeof(name))) {
               send_response(client, 401, "Unauthorized", "application/json",
                             "{\"success\":false,\"message\":\"Sign in first.\"}");
               return;
           }
           if (strcmp(role, "staff") != 0 || !body) {
               send_response(client, 403, "Forbidden", "application/json",
                             "{\"success\":false,\"message\":\"Staff only.\"}");
               return;
           }
           get_value(body, "studentId", sid, sizeof(sid));
           get_value(body, "courseId", cid, sizeof(cid));
           if (strcmp(path, "/api/staff/grade") == 0) {
               get_value(body, "score", val, sizeof(val));
               ok = staff_save_grade(user_id, atoi(sid), atoi(cid), atof(val));
           } else {
               get_value(body, "present", val, sizeof(val));
               ok = staff_save_attendance(user_id, atoi(sid), atoi(cid), atoi(val));
           }
           if (ok) send_response(client, 200, "OK", "application/json", "{\"success\":true}");
           else    send_response(client, 400, "Bad Request", "application/json",
                       "{\"success\":false,\"message\":\"Not saved: check the student, course and score.\"}");
           return;
       }

    get_value() only reads string values ("key":"value"), so the frontend sends
    every field as a string.

 7. KNOWN LIMITS
      - Attendance has no date column, so each save replaces the previous value
        for that student and course.
      - GPA = AVG(score) / 25 is a choice made here (100 average = 4.0). Change
        the formula in staff_save_grade() if your school uses another scale.
      - No announcement-posting function exists yet.
      - Strings are not escaped in the generated file.
      - Relative output path; start the server from the repository root.
      - Plain text passwords (see Academia_Auth.c).

 8. CHANGES FROM THE VERSION IN THE REPOSITORY
      write_list() is now `static` (link collision with the other modules).
      The two guards and the two save functions are new. The read logic is
      unchanged.
================================================================================
*/

#include "Academia_Staff.h"
#include "../Database.h"

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sqlite3.h>
#endif

/*
 write_list
 ----------
 Runs ONE SELECT and writes the rows as a property holding a JS array.

 Parameters
   out       the already-open staff_states.js
   db        an open SQLite connection
   name      property name to write ("courses", "students", ...)
   sql       a SELECT with EXACTLY ONE `?` placeholder
   staff_id  the integer bound to that placeholder

 How it works
   1. Writes "  <name>: [".
   2. If the statement cannot be prepared, closes the array and returns (empty
      list instead of broken JavaScript).
   3. Binds staff_id with sqlite3_bind_int (no SQL injection possible).
   4. For each row writes { alias: value, ... }; the SQL `AS` names become the
      JS property names. FLOAT -> "%.1f", INTEGER -> "%d", else quoted string
      ("" for NULL).
   5. Finalizes and writes "],".
*/
static void write_list(FILE *out, sqlite3 *db, const char *name, const char *sql, int staff_id) {
    sqlite3_stmt *row = NULL;

    fprintf(out, "  %s: [\n", name);

    if (sqlite3_prepare_v2(db, sql, -1, &row, NULL) != SQLITE_OK) {
        fputs("  ],\n", out);
        return;
    }
    sqlite3_bind_int(row, 1, staff_id);

    while (sqlite3_step(row) == SQLITE_ROW) {
        int i;
        fputs("    {", out);
        for (i = 0; i < sqlite3_column_count(row); i++) {
            const char *text = (const char *)sqlite3_column_text(row, i);

            fprintf(out, "%s%s: ", i ? ", " : "", sqlite3_column_name(row, i));

            if (sqlite3_column_type(row, i) == SQLITE_FLOAT) {
                fprintf(out, "%.1f", sqlite3_column_double(row, i));
            } else if (sqlite3_column_type(row, i) == SQLITE_INTEGER) {
                fprintf(out, "%d", sqlite3_column_int(row, i));
            } else {
                fprintf(out, "\"%s\"", text == NULL ? "" : text);
            }
        }
        fputs(" },\n", out);
    }

    sqlite3_finalize(row);
    fputs("  ],\n", out);
}

/*
 update_staff_frontend_state
 ---------------------------
 Called by the server for GET /api/staff-data (role "staff" only).

 Parameter
   staff_user_id   the logged in user's id (user.id, which equals staff.id)

 Returns
   1  staff member found and file written
   0  database/file could not be opened, or no such staff member

 Steps
   1. Open the database.
   2. Open staff_states.js for writing.
   3. Write "const staffState = {".
   4. "staff" block: user JOIN staff for this id. Sets `found`.
   5. Lists, ALL scoped by course.staff_id = ? (a teacher only sees their own):
        courses       course + sub-select counting active enrollments
                      -> courseId, course, students
        students      enrollment JOIN student JOIN user JOIN course, with a
                      sub-select for the current grade, ordered by last name
                      -> studentId, name, username, course, score
        attendance    attendance JOIN course JOIN student JOIN user
                      -> course, student, present
        announcements announcement JOIN course, newest first
                      -> course, title, message, date
   6. Write "success" and "staffId", close the object.
   7. Close the file and database, log a line, return `found`.
*/
int update_staff_frontend_state(int staff_user_id) {
    sqlite3 *db = open_database();
    sqlite3_stmt *row = NULL;
    FILE *out = NULL;
    int found = 0;

    if (!db) {
        printf("[Staff] Cannot open the database.\n");
        return 0;
    }

    out = fopen("Academia/FrontEnd/JS/staff_states.js", "w");
    if (!out) {
        printf("[Staff] Cannot write staff_states.js\n");
        close_database(db);
        return 0;
    }

    fputs("// Written by the Academia C backend.\nconst staffState = {\n", out);

    /* ---- staff profile ---- */
    fputs("  staff: {\n", out);
    if (sqlite3_prepare_v2(db,
            "SELECT user.username AS username, user.first_name AS firstName, "
            "user.last_name AS lastName, staff.phone AS phone FROM user "
            "INNER JOIN staff ON user.id = staff.id WHERE user.id = ?;",
            -1, &row, NULL) == SQLITE_OK) {
        sqlite3_bind_int(row, 1, staff_user_id);
        if (sqlite3_step(row) == SQLITE_ROW) {
            int i, n = sqlite3_column_count(row);
            found = 1;
            fprintf(out, "    userId: %d,\n", staff_user_id);
            for (i = 0; i < n; i++) {
                const char *text = (const char *)sqlite3_column_text(row, i);
                fprintf(out, "    %s: \"%s\"%s\n", sqlite3_column_name(row, i),
                        text == NULL ? "" : text, i == n - 1 ? "" : ",");
            }
        }
        sqlite3_finalize(row);
    }
    fputs("  },\n", out);

    /* ---- the teacher's courses, with active student counts ---- */
    write_list(out, db, "courses",
        "SELECT course.id AS courseId, course.course_name AS course, "
        "(SELECT COUNT(*) FROM enrollment WHERE enrollment.course_id = course.id "
        "AND enrollment.status = 1) AS students FROM course "
        "WHERE course.staff_id = ? ORDER BY course.id;", staff_user_id);

    /* ---- roster with the current grade ---- */
    write_list(out, db, "students",
        "SELECT user.id AS studentId, user.first_name || ' ' || user.last_name AS name, "
        "user.username AS username, course.course_name AS course, "
        "(SELECT score FROM grade WHERE grade.student_id = user.id "
        "AND grade.course_id = course.id) AS score FROM enrollment "
        "INNER JOIN student ON enrollment.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "INNER JOIN course ON enrollment.course_id = course.id "
        "WHERE course.staff_id = ? ORDER BY user.last_name;", staff_user_id);

    /* ---- attendance for the teacher's courses ---- */
    write_list(out, db, "attendance",
        "SELECT course.course_name AS course, "
        "user.first_name || ' ' || user.last_name AS student, "
        "attendance.present AS present FROM attendance "
        "INNER JOIN course ON attendance.course_id = course.id "
        "INNER JOIN student ON attendance.student_id = student.id "
        "INNER JOIN user ON student.id = user.id "
        "WHERE course.staff_id = ? ORDER BY course.id;", staff_user_id);

    /* ---- announcements for the teacher's courses ---- */
    write_list(out, db, "announcements",
        "SELECT course.course_name AS course, announcement.title AS title, "
        "announcement.message AS message, announcement.announcement_date AS date "
        "FROM announcement INNER JOIN course ON announcement.course_id = course.id "
        "WHERE course.staff_id = ? ORDER BY announcement.id DESC;", staff_user_id);

    fprintf(out, "  success: %s,\n  staffId: %d\n};\n",
            found ? "true" : "false", staff_user_id);

    fclose(out);
    close_database(db);
    printf("[Staff] staff_states.js written for staff %d\n", staff_user_id);
    return found;
}

/* ========================= WRITE HALF (new) ============================== */

/*
 staff_owns_course
 -----------------
 Guard. Returns 1 only if course.staff_id equals this teacher, so one teacher
 cannot change another teacher's class. Returns 0 on any error.
*/
static int staff_owns_course(sqlite3 *db, int staff_id, int course_id) {
    sqlite3_stmt *s = NULL;
    int ok = 0;
    if (sqlite3_prepare_v2(db, "SELECT 1 FROM course WHERE id = ? AND staff_id = ?;",
                           -1, &s, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(s, 1, course_id);
    sqlite3_bind_int(s, 2, staff_id);
    ok = (sqlite3_step(s) == SQLITE_ROW);
    sqlite3_finalize(s);
    return ok;
}

/*
 student_enrolled
 ----------------
 Guard. Returns 1 only if the student has an ACTIVE (status 1) enrollment in
 the course, so grades cannot be written for students who are not in the class.
*/
static int student_enrolled(sqlite3 *db, int student_id, int course_id) {
    sqlite3_stmt *s = NULL;
    int ok = 0;
    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM enrollment WHERE student_id = ? AND course_id = ? AND status = 1;",
            -1, &s, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(s, 1, student_id);
    sqlite3_bind_int(s, 2, course_id);
    ok = (sqlite3_step(s) == SQLITE_ROW);
    sqlite3_finalize(s);
    return ok;
}

/*
 staff_save_grade
 ----------------
 Saves one grade and refreshes the student's GPA.

 Parameters: staff_id (from the session), student_id, course_id, score (0..100)
 Returns:    1 saved, 0 rejected or failed

 Steps
   1. Reject if score is outside 0..100, the teacher does not own the course, or
      the student is not enrolled.
   2. UPDATE the existing grade row; if sqlite3_changes() is 0 there was none,
      so INSERT a new row.
   3. On success set student.gpa = AVG(score) / 25.0 over all of that student's
      grades (an average of 100 gives 4.0).
   4. Close the database.
*/
int staff_save_grade(int staff_id, int student_id, int course_id, double score) {
    sqlite3 *db = open_database();
    sqlite3_stmt *s = NULL;
    int ok = 0;

    if (!db) return 0;
    if (score < 0 || score > 100 ||
        !staff_owns_course(db, staff_id, course_id) ||
        !student_enrolled(db, student_id, course_id)) {
        close_database(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db, "UPDATE grade SET score = ? WHERE student_id = ? AND course_id = ?;",
                           -1, &s, NULL) == SQLITE_OK) {
        sqlite3_bind_double(s, 1, score);
        sqlite3_bind_int(s, 2, student_id);
        sqlite3_bind_int(s, 3, course_id);
        sqlite3_step(s);
        sqlite3_finalize(s);
    }
    if (sqlite3_changes(db) == 0) {
        if (sqlite3_prepare_v2(db, "INSERT INTO grade (student_id, course_id, score) VALUES (?, ?, ?);",
                               -1, &s, NULL) == SQLITE_OK) {
            sqlite3_bind_int(s, 1, student_id);
            sqlite3_bind_int(s, 2, course_id);
            sqlite3_bind_double(s, 3, score);
            ok = (sqlite3_step(s) == SQLITE_DONE);
            sqlite3_finalize(s);
        }
    } else {
        ok = 1;
    }

    if (ok && sqlite3_prepare_v2(db,
            "UPDATE student SET gpa = (SELECT AVG(score) / 25.0 FROM grade WHERE student_id = ?) "
            "WHERE id = ?;", -1, &s, NULL) == SQLITE_OK) {
        sqlite3_bind_int(s, 1, student_id);
        sqlite3_bind_int(s, 2, student_id);
        sqlite3_step(s);
        sqlite3_finalize(s);
    }

    close_database(db);
    return ok;
}

/*
 staff_save_attendance
 ---------------------
 Saves present/absent for one student in one course.

 Parameters: staff_id (from the session), student_id, course_id,
             present (non-zero = present, 0 = absent)
 Returns:    1 saved, 0 rejected or failed

 Same two guards as staff_save_grade. Then UPDATE the existing row, or INSERT
 when none exists. There is no date column, so only the latest value per
 student and course is kept.
*/
int staff_save_attendance(int staff_id, int student_id, int course_id, int present) {
    sqlite3 *db = open_database();
    sqlite3_stmt *s = NULL;
    int ok = 0;

    if (!db) return 0;
    if (!staff_owns_course(db, staff_id, course_id) ||
        !student_enrolled(db, student_id, course_id)) {
        close_database(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db, "UPDATE attendance SET present = ? WHERE student_id = ? AND course_id = ?;",
                           -1, &s, NULL) == SQLITE_OK) {
        sqlite3_bind_int(s, 1, present ? 1 : 0);
        sqlite3_bind_int(s, 2, student_id);
        sqlite3_bind_int(s, 3, course_id);
        sqlite3_step(s);
        sqlite3_finalize(s);
    }
    if (sqlite3_changes(db) == 0) {
        if (sqlite3_prepare_v2(db, "INSERT INTO attendance (course_id, student_id, present) VALUES (?, ?, ?);",
                               -1, &s, NULL) == SQLITE_OK) {
            sqlite3_bind_int(s, 1, course_id);
            sqlite3_bind_int(s, 2, student_id);
            sqlite3_bind_int(s, 3, present ? 1 : 0);
            ok = (sqlite3_step(s) == SQLITE_DONE);
            sqlite3_finalize(s);
        }
    } else {
        ok = 1;
    }

    close_database(db);
    return ok;
}