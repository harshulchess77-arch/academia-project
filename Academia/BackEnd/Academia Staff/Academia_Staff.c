/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
#include "academia_export.h"

void update_staff_frontend_state(int id)
{
    sqlite3 *db;
    FILE *out = begin_state(&db,
        "Academia/BackEnd/Academia Staff/staff_states.js", "staffState");
    if (!out) return;

    int ok = 1;

    ok &= export_query(out, db, "staff",
        "SELECT user.id AS userId, username, first_name AS firstName, "
        "last_name AS lastName, phone FROM user "
        "JOIN staff ON user.id = staff.id "
        "WHERE user.id = ? AND role = 'staff'", id, 1) > 0;

    ok &= export_query(out, db, "courses",
        "SELECT id AS courseId, course_name AS course "
        "FROM course WHERE staff_id = ?", id, 0) >= 0;

    // Students enrolled in this staff member's courses (for the dropdowns).
    ok &= export_query(out, db, "enrollments",
        "SELECT user.id AS studentId, first_name || ' ' || last_name AS student, "
        "course.id AS courseId, course_name AS course "
        "FROM enrollment JOIN user ON enrollment.student_id = user.id "
        "JOIN course ON enrollment.course_id = course.id "
        "WHERE course.staff_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "grades",
        "SELECT grade.id AS gradeId, grade.student_id AS studentId, "
        "first_name || ' ' || last_name AS student, "
        "course_name AS course, score FROM grade "
        "JOIN user ON grade.student_id = user.id "
        "JOIN course ON grade.course_id = course.id "
        "WHERE course.staff_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "attendance",
        "SELECT attendance.id AS attendanceId, "
        "attendance.student_id AS studentId, "
        "first_name || ' ' || last_name AS student, "
        "course_name AS course, present FROM attendance "
        "JOIN user ON attendance.student_id = user.id "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE course.staff_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "announcements",
        "SELECT announcement.id AS announcementId, course_name AS course, "
        "title, message, announcement_date AS date FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC", 0, 0) >= 0;

    end_state(out, db, ok, "Staff data could not be loaded.");
}
