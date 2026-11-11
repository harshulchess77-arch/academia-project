/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia - Local Web Server and API Controller
November 1, 2026
*/

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsqlite/winsqlite3.h>

#include "../Academia Auth/Academia_Auth.h"
#include "../Academia Database/Database.h"
#include "../Academia Student/Academia_Student.h"
#include "../Academia Staff/Academia_Staff.h"
#include "../Academia Parent/Academia_Parents.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winsqlite3.lib")

#define PORT 3000
#define BUFFER_SIZE 65536

// Active SQLite database connection
static sqlite3 *db = NULL;

// Helper to send HTTP header and string body
void send_response(SOCKET client, int status, const char *status_text,
                   const char *content_type, const char *body,
                   const char *extra_headers) {
    char header[2048];
    int length = body != NULL ? (int)strlen(body) : 0;

    // Build the HTTP header using the response information.
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Headers: Content-Type, Cookie\r\n"
             "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             "Connection: close\r\n"
             "%s"
             "\r\n",
             status, status_text, content_type, length,
             extra_headers != NULL ? extra_headers : "");

    send(client, header, (int)strlen(header), 0);

    if (length > 0) {
        send(client, body, length, 0);
    }
}

// Helper to send binary files (HTML, CSS, JS, PNG, etc.)
void send_file(SOCKET client, const char *filepath, const char *content_type) {
    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL) {
        send_response(client, 404, "Not Found", "text/html",
                      "<h1>404 - File Not Found</h1><p><a href='/'>Back to Home</a></p>",
                      NULL);
        return;
    }

    // Find the total file size for the HTTP Content-Length header.
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[1024];

    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Connection: close\r\n"
             "\r\n",
             content_type, file_size);

    send(client, header, (int)strlen(header), 0);

    // Send large files in smaller chunks.
    char chunk[8192];
    size_t bytes_read;

    while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        send(client, chunk, (int)bytes_read, 0);
    }

    fclose(fp);
}

// Simple JSON string value extractor
void get_json_value(const char *body, const char *key,
                    char *value, size_t value_size) {
    value[0] = '\0';

    if (body == NULL || key == NULL) {
        return;
    }

    // Search for the requested key inside the JSON body.
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *pos = strstr(body, search);

    if (pos == NULL) {
        return;
    }

    pos = strchr(pos, ':');

    if (pos == NULL) {
        return;
    }

    pos++;

    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }

    if (*pos == '"') {
        pos++;
        size_t i = 0;

        while (*pos != '\0' && *pos != '"' && i < value_size - 1) {
            value[i++] = *pos++;
        }

        value[i] = '\0';
    } else {
        // numeric or boolean
        size_t i = 0;

        while (*pos != '\0' &&
               *pos != ',' &&
               *pos != '}' &&
               *pos != '\r' &&
               *pos != '\n' &&
               i < value_size - 1) {
            value[i++] = *pos++;
        }

        value[i] = '\0';
    }
}

// Get cookie value
void get_cookie(const char *request, char *token, size_t token_size) {
    token[0] = '\0';

    const char *cookie = strstr(request, "Cookie:");

    if (cookie == NULL) {
        return;
    }

    // Find the Academia session value inside the Cookie header.
    const char *pos = strstr(cookie, "academia_session=");

    if (pos == NULL) {
        return;
    }

    pos += strlen("academia_session=");

    size_t i = 0;

    while (*pos != '\0' &&
           *pos != ';' &&
           *pos != '\r' &&
           *pos != '\n' &&
           i < token_size - 1) {
        token[i++] = *pos++;
    }

    token[i] = '\0';
}

// Determine MIME type
const char *get_mime_type(const char *path) {
    const char *dot = strrchr(path, '.');

    if (dot == NULL) return "text/html";
    if (_stricmp(dot, ".html") == 0 ||
        _stricmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    if (_stricmp(dot, ".css") == 0) return "text/css; charset=utf-8";
    if (_stricmp(dot, ".js") == 0) return "application/javascript; charset=utf-8";
    if (_stricmp(dot, ".png") == 0) return "image/png";
    if (_stricmp(dot, ".jpg") == 0 ||
        _stricmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (_stricmp(dot, ".svg") == 0) return "image/svg+xml";
    if (_stricmp(dot, ".json") == 0) return "application/json; charset=utf-8";
    if (_stricmp(dot, ".mp4") == 0) return "video/mp4";

    return "application/octet-stream";
}

// Check if a file exists on disk
static int file_exists(const char *path) {
    FILE *fp = fopen(path, "rb");

    if (fp) {
        fclose(fp);
        return 1;
    }

    return 0;
}

// API: Handle Login
void handle_api_login(SOCKET client, const char *body) {
    char username[64] = "";
    char password[64] = "";

    get_json_value(body, "username", username, sizeof(username));
    get_json_value(body, "password", password, sizeof(password));

    char response[2048];
    char token[128];
    int user_id = 0;
    char role[32];
    char name[128];

    int success = auth_login(db, username, password, response, sizeof(response),
                             token, sizeof(token), &user_id, role, sizeof(role),
                             name, sizeof(name));

    char cookie_header[256] = "";

    if (success) {
        // Store the authenticated session token in the browser.
        snprintf(cookie_header, sizeof(cookie_header),
                 "Set-Cookie: academia_session=%s; Path=/; HttpOnly\r\n",
                 token);

        // Refresh states
        if (strcmp(role, "student") == 0) {
            update_student_frontend_state(user_id);
        } else if (strcmp(role, "staff") == 0) {
            update_staff_frontend_state(user_id);
        } else if (strcmp(role, "parent") == 0) {
            update_parent_frontend_state(user_id);
        }
    }

    send_response(client, 200, "OK", "application/json",
                  response, cookie_header);
}

// API: Check current session
void handle_api_session(SOCKET client, const char *request) {
    char token[128] = "";
    get_cookie(request, token, sizeof(token));

    int user_id = 0;
    char role[32] = "";
    char name[128] = "";

    if (auth_check(token, &user_id, role, sizeof(role),
                   name, sizeof(name))) {
        char json[512];

        snprintf(json, sizeof(json),
                 "{\"authenticated\":true,\"user\":"
                 "{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
                 user_id, name, role);

        send_response(client, 200, "OK",
                      "application/json", json, NULL);
    } else {
        send_response(client, 200, "OK", "application/json",
                      "{\"authenticated\":false,\"user\":null}", NULL);
    }
}

// API: Return Student Data JSON
void handle_api_student_data(SOCKET client, int student_id) {
    if (student_id <= 0) {
        student_id = 1; // Default to student 1
    }

    char json[16384];

    // Offset tracks where the next JSON section should be added.
    size_t offset = 0;
    sqlite3_stmt *stmt = NULL;

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "{\"success\":true,\"student\":{");

    // Student profile
    const char *sql_profile =
        "SELECT user.id, user.username, user.first_name, user.last_name, "
        "student.grade_level, student.gpa "
        "FROM user JOIN student ON user.id = student.id "
        "WHERE user.id = ?;";

    if (sqlite3_prepare_v2(db, sql_profile, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "\"userId\":%d,\"username\":\"%s\","
                               "\"firstName\":\"%s\",\"lastName\":\"%s\","
                               "\"gradeLevel\":%d,\"gpa\":%.2f",
                               sqlite3_column_int(stmt, 0),
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3),
                               sqlite3_column_int(stmt, 4),
                               sqlite3_column_double(stmt, 5));
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "},\"grades\":[");

    // Grades
    const char *sql_grades =
        "SELECT course.course_name, grade.score "
        "FROM grade JOIN course ON grade.course_id = course.id "
        "WHERE student_id = ?;";

    if (sqlite3_prepare_v2(db, sql_grades, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);

        // Prevent a comma from appearing before the first array item.
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"score\":%.1f}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_double(stmt, 1));
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"attendance\":[");

    // Attendance
    const char *sql_att =
        "SELECT course.course_name, attendance.present "
        "FROM attendance "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE student_id = ?;";

    if (sqlite3_prepare_v2(db, sql_att, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"status\":\"%s\"}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_int(stmt, 1)
                                   ? "Present" : "Absent");
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"assignments\":[");

    // Assignments
    const char *sql_ass =
        "SELECT course.course_name, assignment.assignment_name, "
        "assignment.due_date "
        "FROM assignment "
        "JOIN course ON assignment.course_id = course.id "
        "JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? "
        "AND enrollment.status = 1 "
        "ORDER BY assignment.due_date;";

    if (sqlite3_prepare_v2(db, sql_ass, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"course\":\"%s\",\"assignmentName\":\"%s\","
                "\"dueDate\":\"%s\"}",
                first ? "" : ",",
                sqlite3_column_text(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"announcements\":[");

    // Announcements
    const char *sql_ann =
        "SELECT course.course_name, announcement.title, "
        "announcement.message, announcement.announcement_date "
        "FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC;";

    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *c = sqlite3_column_text(stmt, 0);

            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"course\":\"%s\",\"title\":\"%s\","
                "\"message\":\"%s\",\"date\":\"%s\"}",
                first ? "" : ",",
                c ? (const char *)c : "General Announcement",
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset, "]}");

    send_response(client, 200, "OK", "application/json", json, NULL);
}

// API: Return Parent Data JSON
void handle_api_parent_data(SOCKET client, int parent_id) {
    if (parent_id <= 0) {
        parent_id = 3; // Default to parent1 (Marcus Rivera)
    }

    char json[16384];
    size_t offset = 0;
    sqlite3_stmt *stmt = NULL;

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "{\"success\":true,\"parent\":{");

    // Parent profile
    const char *sql_p =
        "SELECT user.id, user.username, user.first_name, "
        "user.last_name, parent.phone "
        "FROM user JOIN parent ON user.id = parent.id "
        "WHERE user.id = ?;";

    if (sqlite3_prepare_v2(db, sql_p, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, parent_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "\"userId\":%d,\"username\":\"%s\","
                "\"firstName\":\"%s\",\"lastName\":\"%s\","
                "\"phone\":\"%s\"",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 4)
            );
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "},\"student\":{");

    // Child student
    int child_id = 1;

    const char *sql_child =
        "SELECT user.id, user.first_name, user.last_name, "
        "student.grade_level, student.gpa "
        "FROM student JOIN user ON student.id = user.id "
        "WHERE student.parent_id = ?;";

    if (sqlite3_prepare_v2(db, sql_child, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, parent_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Save the child ID for the grade and attendance queries.
            child_id = sqlite3_column_int(stmt, 0);

            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "\"studentId\":%d,\"firstName\":\"%s\","
                "\"lastName\":\"%s\",\"gradeLevel\":%d,\"gpa\":%.2f",
                child_id,
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_int(stmt, 3),
                sqlite3_column_double(stmt, 4)
            );
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "},\"grades\":[");

    // Child grades
    const char *sql_grades =
        "SELECT course.course_name, grade.score "
        "FROM grade JOIN course ON grade.course_id = course.id "
        "WHERE student_id = ?;";

    if (sqlite3_prepare_v2(db, sql_grades, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, child_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"score\":%.1f}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_double(stmt, 1));
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"attendance\":[");

    // Child attendance
    const char *sql_att =
        "SELECT course.course_name, attendance.present "
        "FROM attendance "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE student_id = ?;";

    if (sqlite3_prepare_v2(db, sql_att, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, child_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"status\":\"%s\"}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_int(stmt, 1)
                                   ? "Present" : "Absent");
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"announcements\":[");

    // Announcements
    const char *sql_ann =
        "SELECT course.course_name, announcement.title, "
        "announcement.message, announcement.announcement_date "
        "FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC;";

    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *c = sqlite3_column_text(stmt, 0);

            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"course\":\"%s\",\"title\":\"%s\","
                "\"message\":\"%s\",\"date\":\"%s\"}",
                first ? "" : ",",
                c ? (const char *)c : "General Announcement",
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset, "]}");

    send_response(client, 200, "OK", "application/json", json, NULL);
}

// API: Return Staff Data JSON
void handle_api_staff_data(SOCKET client, int staff_id) {
    if (staff_id <= 0) {
        staff_id = 4; // Default to Dr. Vance
    }

    char json[20480];
    size_t offset = 0;
    sqlite3_stmt *stmt = NULL;

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "{\"success\":true,\"staff\":{");

    // Staff profile
    const char *sql_s =
        "SELECT user.id, user.username, user.first_name, "
        "user.last_name, staff.phone "
        "FROM user JOIN staff ON user.id = staff.id "
        "WHERE user.id = ?;";

    if (sqlite3_prepare_v2(db, sql_s, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, staff_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "\"userId\":%d,\"username\":\"%s\","
                "\"firstName\":\"%s\",\"lastName\":\"%s\","
                "\"phone\":\"%s\"",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 4)
            );
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "},\"courses\":[");

    // Courses taught
    const char *sql_c =
        "SELECT id, course_name FROM course WHERE staff_id = ?;";

    if (sqlite3_prepare_v2(db, sql_c, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, staff_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"courseId\":%d,\"course\":\"%s\"}",
                first ? "" : ",",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"enrollments\":[");

    // Students enrolled in staff's courses
    const char *sql_e =
        "SELECT DISTINCT user.id, "
        "user.first_name || ' ' || user.last_name, "
        "course.id, course.course_name "
        "FROM enrollment "
        "JOIN user ON enrollment.student_id = user.id "
        "JOIN course ON enrollment.course_id = course.id "
        "WHERE course.staff_id = ?;";

    if (sqlite3_prepare_v2(db, sql_e, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, staff_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"studentId\":%d,\"student\":\"%s\","
                "\"courseId\":%d,\"course\":\"%s\"}",
                first ? "" : ",",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_int(stmt, 2),
                sqlite3_column_text(stmt, 3)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"grades\":[");

    // All grades recorded for staff's courses
    const char *sql_g =
        "SELECT grade.id, "
        "user.first_name || ' ' || user.last_name, "
        "course.course_name, grade.score "
        "FROM grade "
        "JOIN user ON grade.student_id = user.id "
        "JOIN course ON grade.course_id = course.id "
        "WHERE course.staff_id = ?;";

    if (sqlite3_prepare_v2(db, sql_g, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, staff_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"gradeId\":%d,\"student\":\"%s\","
                "\"course\":\"%s\",\"score\":%.1f}",
                first ? "" : ",",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"attendance\":[");

    // All attendance records
    const char *sql_a =
        "SELECT attendance.id, "
        "user.first_name || ' ' || user.last_name, "
        "course.course_name, attendance.present "
        "FROM attendance "
        "JOIN user ON attendance.student_id = user.id "
        "JOIN course ON attendance.course_id = course.id "
        "WHERE course.staff_id = ?;";

    if (sqlite3_prepare_v2(db, sql_a, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, staff_id);
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"attendanceId\":%d,\"student\":\"%s\","
                "\"course\":\"%s\",\"present\":%d}",
                first ? "" : ",",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_int(stmt, 3)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "],\"announcements\":[");

    // Announcements
    const char *sql_ann =
        "SELECT announcement.id, course.course_name, "
        "announcement.title, announcement.message, "
        "announcement.announcement_date "
        "FROM announcement "
        "LEFT JOIN course ON announcement.course_id = course.id "
        "ORDER BY announcement.id DESC;";

    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        int first = 1;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *c = sqlite3_column_text(stmt, 1);

            offset += snprintf(
                json + offset,
                sizeof(json) - offset,
                "%s{\"announcementId\":%d,\"course\":\"%s\","
                "\"title\":\"%s\",\"message\":\"%s\",\"date\":\"%s\"}",
                first ? "" : ",",
                sqlite3_column_int(stmt, 0),
                c ? (const char *)c : "General Announcement",
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 4)
            );
            first = 0;
        }

        sqlite3_finalize(stmt);
    }

    offset += snprintf(json + offset, sizeof(json) - offset, "]}");

    send_response(client, 200, "OK", "application/json", json, NULL);
}

// API: Save Grade
void handle_api_save_grade(SOCKET client, const char *body) {
    char student_str[32] = "";
    char course_str[32] = "";
    char grade_str[32] = "";

    get_json_value(body, "studentId", student_str, sizeof(student_str));
    get_json_value(body, "courseId", course_str, sizeof(course_str));
    get_json_value(body, "grade", grade_str, sizeof(grade_str));

    if (grade_str[0] == '\0') {
        get_json_value(body, "score", grade_str, sizeof(grade_str));
    }

    int student_id = atoi(student_str);
    int course_id = atoi(course_str);
    double score = atof(grade_str);

    if (student_id > 0 && course_id > 0) {
        sqlite3_stmt *stmt = NULL;

        // Check if grade already exists
        const char *check_sql =
            "SELECT id FROM grade "
            "WHERE student_id = ? AND course_id = ?;";

        int exists = 0;
        int grade_id = 0;

        if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, student_id);
            sqlite3_bind_int(stmt, 2, course_id);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                exists = 1;
                grade_id = sqlite3_column_int(stmt, 0);
            }

            sqlite3_finalize(stmt);
        }

        if (exists) {
            // Update the current grade instead of creating a duplicate.
            const char *update_sql =
                "UPDATE grade SET score = ? WHERE id = ?;";

            if (sqlite3_prepare_v2(db, update_sql, -1,
                                   &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_double(stmt, 1, score);
                sqlite3_bind_int(stmt, 2, grade_id);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        } else {
            // Create a new grade when no previous record exists.
            const char *ins_sql =
                "INSERT INTO grade "
                "(student_id, course_id, score) VALUES (?, ?, ?);";

            if (sqlite3_prepare_v2(db, ins_sql, -1,
                                   &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, student_id);
                sqlite3_bind_int(stmt, 2, course_id);
                sqlite3_bind_double(stmt, 3, score);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }

        // Recalculate student GPA
        const char *gpa_sql =
            "SELECT AVG(score) FROM grade WHERE student_id = ?;";

        double avg_score = 0;

        if (sqlite3_prepare_v2(db, gpa_sql, -1,
                               &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, student_id);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                avg_score = sqlite3_column_double(stmt, 0);
            }

            sqlite3_finalize(stmt);
        }

        double gpa = (avg_score / 100.0) * 4.0;

        if (gpa > 4.0) {
            gpa = 4.0;
        }

        const char *upd_gpa =
            "UPDATE student SET gpa = ? WHERE id = ?;";

        if (sqlite3_prepare_v2(db, upd_gpa, -1,
                               &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, gpa);
            sqlite3_bind_int(stmt, 2, student_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        // Keep state files synced
        update_student_frontend_state(student_id);
        update_staff_frontend_state(4);
        update_parent_frontend_state(3);

        send_response(
            client, 200, "OK", "application/json",
            "{\"success\":true,"
            "\"message\":\"Grade saved successfully.\"}",
            NULL
        );
    } else {
        send_response(
            client, 400, "Bad Request", "application/json",
            "{\"success\":false,"
            "\"message\":\"Invalid student or course ID.\"}",
            NULL
        );
    }
}

// API: Record Attendance
void handle_api_save_attendance(SOCKET client, const char *body) {
    char student_str[32] = "";
    char course_str[32] = "";
    char status_str[32] = "";

    get_json_value(body, "studentId", student_str, sizeof(student_str));
    get_json_value(body, "courseId", course_str, sizeof(course_str));
    get_json_value(body, "status", status_str, sizeof(status_str));

    if (status_str[0] == '\0') {
        get_json_value(body, "present", status_str, sizeof(status_str));
    }

    int student_id = atoi(student_str);
    int course_id = atoi(course_str);
    int present = atoi(status_str);

    if (student_id > 0 && course_id > 0) {
        sqlite3_stmt *stmt = NULL;

        const char *sql =
            "INSERT INTO attendance "
            "(course_id, student_id, present) VALUES (?, ?, ?);";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, course_id);
            sqlite3_bind_int(stmt, 2, student_id);
            sqlite3_bind_int(stmt, 3, present);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        update_student_frontend_state(student_id);
        update_staff_frontend_state(4);
        update_parent_frontend_state(3);

        send_response(
            client, 200, "OK", "application/json",
            "{\"success\":true,"
            "\"message\":\"Attendance recorded successfully.\"}",
            NULL
        );
    } else {
        send_response(
            client, 400, "Bad Request", "application/json",
            "{\"success\":false,"
            "\"message\":\"Invalid student or course ID.\"}",
            NULL
        );
    }
}

// API: Post Announcement
void handle_api_post_announcement(SOCKET client, const char *body) {
    char course_str[32] = "";
    char title[128] = "";
    char message[1024] = "";

    get_json_value(body, "courseId", course_str, sizeof(course_str));
    get_json_value(body, "title", title, sizeof(title));
    get_json_value(body, "message", message, sizeof(message));

    int course_id = atoi(course_str);

    if (strlen(title) > 0 && strlen(message) > 0) {
        sqlite3_stmt *stmt = NULL;

        const char *sql =
            "INSERT INTO announcement "
            "(course_id, title, message, announcement_date) "
            "VALUES (?, ?, ?, date('now'));";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            // A missing course ID creates a general announcement.
            if (course_id > 0) {
                sqlite3_bind_int(stmt, 1, course_id);
            } else {
                sqlite3_bind_null(stmt, 1);
            }

            sqlite3_bind_text(stmt, 2, title, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, message, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        update_student_frontend_state(1);
        update_staff_frontend_state(4);
        update_parent_frontend_state(3);

        send_response(
            client, 200, "OK", "application/json",
            "{\"success\":true,"
            "\"message\":\"Announcement published successfully.\"}",
            NULL
        );
    } else {
        send_response(
            client, 400, "Bad Request", "application/json",
            "{\"success\":false,"
            "\"message\":\"Title and message are required.\"}",
            NULL
        );
    }
}

// Route and Dispatch Request
void handle_client_request(SOCKET client) {
    char buffer[BUFFER_SIZE];

    int bytes_received =
        recv(client, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        return;
    }

    buffer[bytes_received] = '\0';

    // If POST, ensure full body is read according to Content-Length
    char *header_end = strstr(buffer, "\r\n\r\n");

    if (header_end != NULL) {
        int body_received =
            (int)(bytes_received - ((header_end + 4) - buffer));

        char *cl_header = strstr(buffer, "Content-Length:");

        if (cl_header == NULL) {
            cl_header = strstr(buffer, "content-length:");
        }

        int content_length = 0;

        if (cl_header != NULL) {
            content_length = atoi(cl_header + 15);
        }

        // Continue reading until the complete request body arrives.
        while (body_received < content_length &&
               bytes_received < (int)sizeof(buffer) - 1) {
            int more = recv(
                client,
                buffer + bytes_received,
                (int)sizeof(buffer) - 1 - bytes_received,
                0
            );

            if (more <= 0) {
                break;
            }

            bytes_received += more;
            buffer[bytes_received] = '\0';
            body_received += more;
        }
    }

    char method[16] = "";
    char path[512] = "";

    sscanf(buffer, "%15s %511s", method, path);

    // Find body of HTTP request if POST
    const char *body = strstr(buffer, "\r\n\r\n");

    if (body) {
        body += 4;
    } else {
        body = "";
    }

    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client, 204, "No Content",
                      "text/plain", "", NULL);
        return;
    }

    // --- API ROUTING ---
    if (strcmp(path, "/api/login") == 0 &&
        strcmp(method, "POST") == 0) {
        handle_api_login(client, body);
        return;
    }

    if (strcmp(path, "/api/session") == 0) {
        handle_api_session(client, buffer);
        return;
    }

    if (strcmp(path, "/api/logout") == 0) {
        char token[128] = "";

        get_cookie(buffer, token, sizeof(token));
        auth_logout(token);

        send_response(client, 200, "OK", "application/json",
                      "{\"success\":true}", NULL);
        return;
    }

    if (strncmp(path, "/api/student-data", 17) == 0) {
        int id = 1;
        const char *q = strstr(path, "id=");

        if (q) {
            id = atoi(q + 3);
        }

        handle_api_student_data(client, id);
        return;
    }

    if (strncmp(path, "/api/parent-data", 16) == 0) {
        int id = 3;
        const char *q = strstr(path, "id=");

        if (q) {
            id = atoi(q + 3);
        }

        handle_api_parent_data(client, id);
        return;
    }

    if (strncmp(path, "/api/staff-data", 15) == 0) {
        int id = 4;
        const char *q = strstr(path, "id=");

        if (q) {
            id = atoi(q + 3);
        }

        handle_api_staff_data(client, id);
        return;
    }

    if (strcmp(path, "/api/staff/grade") == 0 &&
        strcmp(method, "POST") == 0) {
        handle_api_save_grade(client, body);
        return;
    }

    if (strcmp(path, "/api/staff/attendance") == 0 &&
        strcmp(method, "POST") == 0) {
        handle_api_save_attendance(client, body);
        return;
    }

    if (strcmp(path, "/api/staff/announcement") == 0 &&
        strcmp(method, "POST") == 0) {
        handle_api_post_announcement(client, body);
        return;
    }

    // --- STATIC FILE SERVING ---
    // If root or index, serve Landing Page
    if (strcmp(path, "/") == 0 ||
        strcmp(path, "/index.html") == 0) {
        send_file(
            client,
            "FrontEnd/HTML/Academia_Landing.html",
            "text/html; charset=utf-8"
        );
        return;
    }

    // Strip leading slash
    const char *rel_path = path;

    while (*rel_path == '/') {
        rel_path++;
    }

    // Remove query params if present for static file lookup
    char clean_path[512];

    strncpy(clean_path, rel_path, sizeof(clean_path) - 1);
    clean_path[sizeof(clean_path) - 1] = '\0';

    char *question_mark = strchr(clean_path, '?');

    if (question_mark) {
        *question_mark = '\0';
    }

    // Decode URL %20 spaces
    char decoded_path[512];
    size_t dp = 0;

    for (size_t sp = 0;
         clean_path[sp] != '\0' &&
         dp < sizeof(decoded_path) - 1;
         sp++) {
        if (clean_path[sp] == '%' &&
            clean_path[sp + 1] == '2' &&
            clean_path[sp + 2] == '0') {
            decoded_path[dp++] = ' ';
            sp += 2;
        } else {
            decoded_path[dp++] = clean_path[sp];
        }
    }

    decoded_path[dp] = '\0';

    // Try several locations:
    // 1. Direct path
    // 2. FrontEnd/ + decoded_path
    // 3. FrontEnd/HTML/ + decoded_path
    // 4. FrontEnd/CSS/ + decoded_path
    // 5. FrontEnd/JS/ + decoded_path
    // 6. Images/ + decoded_path or FrontEnd/Images/ + decoded_path
    char target_file[512] = "";

    if (file_exists(decoded_path)) {
        strncpy(target_file, decoded_path, sizeof(target_file));
    } else {
        char try_path[512];

        snprintf(try_path, sizeof(try_path),
                 "FrontEnd/%s", decoded_path);

        if (file_exists(try_path)) {
            strncpy(target_file, try_path, sizeof(target_file));
        } else {
            snprintf(try_path, sizeof(try_path),
                     "FrontEnd/HTML/%s", decoded_path);

            if (file_exists(try_path)) {
                strncpy(target_file, try_path, sizeof(target_file));
            } else {
                snprintf(try_path, sizeof(try_path),
                         "FrontEnd/CSS/%s", decoded_path);

                if (file_exists(try_path)) {
                    strncpy(target_file, try_path, sizeof(target_file));
                } else {
                    snprintf(try_path, sizeof(try_path),
                             "FrontEnd/JS/%s", decoded_path);

                    if (file_exists(try_path)) {
                        strncpy(target_file, try_path,
                                sizeof(target_file));
                    } else {
                        snprintf(try_path, sizeof(try_path),
                                 "Images/%s", decoded_path);

                        if (file_exists(try_path)) {
                            strncpy(target_file, try_path,
                                    sizeof(target_file));
                        } else {
                            snprintf(
                                try_path,
                                sizeof(try_path),
                                "FrontEnd/Images/%s",
                                decoded_path
                            );

                            if (file_exists(try_path)) {
                                strncpy(
                                    target_file,
                                    try_path,
                                    sizeof(target_file)
                                );
                            }
                        }
                    }
                }
            }
        }
    }

    if (target_file[0] != '\0' && file_exists(target_file)) {
        send_file(client, target_file, get_mime_type(target_file));
    } else {
        // Send 404 or custom 404 page
        if (file_exists("FrontEnd/HTML/403 Error Page.html")) {
            send_file(
                client,
                "FrontEnd/HTML/403 Error Page.html",
                "text/html; charset=utf-8"
            );
        } else {
            send_response(
                client,
                404,
                "Not Found",
                "text/html",
                "<h2>404 Not Found</h2>"
                "<p><a href='/'>"
                "Go to Academia Landing Page"
                "</a></p>",
                NULL
            );
        }
    }
}

// Server Main Method
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("====================================================================\n");
    printf("     ACADEMIA - Unified School Management System Server\n");
    printf("     CS50 Final Project | C Backend with SQLite3 & Web Frontend\n");
    printf("====================================================================\n");

    // Initialize Database
    db = open_database();

    if (db == NULL) {
        printf("[Fatal Error] Unable to open SQLite database. Exiting.\n");
        return 1;
    }

    // Initialize initial frontend state files
    update_student_frontend_state(1);
    update_staff_frontend_state(4);
    update_parent_frontend_state(3);

    // Initialize Winsock
    WSADATA wsaData;
    int wsa_err = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (wsa_err != 0) {
        printf("[Server Error] WSAStartup failed: %d\n", wsa_err);
        close_database(db);
        return 1;
    }

    // Create the TCP socket used to receive browser connections.
    SOCKET server_socket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_socket == INVALID_SOCKET) {
        printf("[Server Error] Failed to create server socket: %d\n",
               WSAGetLastError());
        WSACleanup();
        close_database(db);
        return 1;
    }

    // Set SO_REUSEADDR
    BOOL opt = TRUE;

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&opt, sizeof(opt));

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[Server Error] Socket bind failed on port %d: %d\n",
               PORT, WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        close_database(db);
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[Server Error] Socket listen failed: %d\n",
               WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        close_database(db);
        return 1;
    }

    printf("\n");
    printf("[Server Status] Local Server successfully running!\n");
    printf("[Server Link]   >>> http://localhost:%d <<<\n", PORT);
    printf("[Server Info]   Open this URL in your web browser to use Academia.\n");
    printf("[Server Info]   Press Ctrl+C in this terminal to shut down.\n");
    printf("--------------------------------------------------------------------\n");

    // Accept loop
    while (1) {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET client = accept(
            server_socket,
            (struct sockaddr *)&client_addr,
            &client_addr_len
        );

        if (client == INVALID_SOCKET) {
            printf("[Server Warning] Accept failed: %d\n",
                   WSAGetLastError());
            continue;
        }

        // Process one client request and then close its socket.
        handle_client_request(client);
        closesocket(client);
    }

    closesocket(server_socket);
    WSACleanup();
    close_database(db);

    return 0;
}