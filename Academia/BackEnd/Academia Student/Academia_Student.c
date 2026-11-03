#include "academia_export.h"

void update_student_frontend_state(int id)
{
    sqlite3 *db;
    FILE *out = begin_state(&db,
        "Academia/BackEnd/Academia Student/student_states.js", "studentState");
    if (!out) return;

    int ok = 1;

    ok &= export_query(out, db, "student",
        "SELECT user.id AS userId, username, first_name AS firstName, "
        "last_name AS lastName, grade_level AS gradeLevel, gpa "
        "FROM user JOIN student ON user.id = student.id "
        "WHERE user.id = ? AND role = 'student'", id, 1) > 0;

    ok &= export_query(out, db, "grades",
        "SELECT course_name AS course, score FROM grade "
        "JOIN course ON grade.course_id = course.id "
        "WHERE student_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "attendance",
        "SELECT course_name AS course, present FROM attendance "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE student_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "assignments",
        "SELECT course_name AS course, assignment_name AS name, "
        "due_date AS due FROM assignment "
        "JOIN course ON assignment.course_id = course.id "
        "JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? AND enrollment.status = 1 "
        "ORDER BY due_date", id, 0) >= 0;

    ok &= export_query(out, db, "announcements",
        "SELECT title, message, announcement_date AS date "
        "FROM announcement ORDER BY id DESC", 0, 0) >= 0;

    end_state(out, db, ok, "Student data could not be loaded.");
}
