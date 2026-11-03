#include "academia_export.h"

void update_parent_frontend_state(int id)
{
    sqlite3 *db;
    FILE *out = begin_state(&db,
        "Academia/BackEnd/Academia Parents/parents_states.js", "parentState");
    if (!out) return;

    int ok = 1;

    ok &= export_query(out, db, "parent",
        "SELECT user.id AS userId, username, first_name AS firstName, "
        "last_name AS lastName, phone FROM user "
        "JOIN parent ON user.id = parent.id "
        "WHERE user.id = ? AND role = 'parent'", id, 1) > 0;

    ok &= export_query(out, db, "students",
        "SELECT student.id AS studentId, first_name AS firstName, "
        "last_name AS lastName, grade_level AS gradeLevel, gpa "
        "FROM student JOIN user ON student.id = user.id "
        "WHERE parent_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "grades",
        "SELECT grade.student_id AS studentId, course_name AS course, score "
        "FROM grade JOIN student ON grade.student_id = student.id "
        "JOIN course ON grade.course_id = course.id "
        "WHERE parent_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "attendance",
        "SELECT attendance.student_id AS studentId, course_name AS course, "
        "present FROM attendance "
        "JOIN student ON attendance.student_id = student.id "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE parent_id = ?", id, 0) >= 0;

    ok &= export_query(out, db, "announcements",
        "SELECT title, message, announcement_date AS date "
        "FROM announcement ORDER BY id DESC", 0, 0) >= 0;

    end_state(out, db, ok, "Parent data could not be loaded.");
}
