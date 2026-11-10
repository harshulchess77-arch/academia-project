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

static sqlite3 *db = NULL;

/* -------------------------------------------------------------------------
   send_response - HTTP header plus a string body.
   ------------------------------------------------------------------------- */
void send_response(SOCKET client, int status, const char *status_text,
                   const char *content_type, const char *body,
                   const char *extra_headers) {
    char header[2048];
    int length = body != NULL ? (int)strlen(body) : 0;

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

/* -------------------------------------------------------------------------
   send_file - serves HTML, CSS, JS, PNG and the rest.
   ------------------------------------------------------------------------- */
void send_file(SOCKET client, const char *filepath, const char *content_type) {
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        send_response(client, 404, "Not Found", "text/html",
                      "<h1>404 - File Not Found</h1><p><a href='/'>Back to Home</a></p>", NULL);
        return;
    }

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

    char chunk[8192];
    size_t bytes_read;
    while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        send(client, chunk, (int)bytes_read, 0);
    }
    fclose(fp);
}

/* -------------------------------------------------------------------------
   get_json_value - pulls one string or number out of a JSON body.
   ------------------------------------------------------------------------- */
void get_json_value(const char *body, const char *key, char *value, size_t value_size) {
    value[0] = '\0';
    if (body == NULL || key == NULL) return;

    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *pos = strstr(body, search);
    if (pos == NULL) return;

    pos = strchr(pos, ':');
    if (pos == NULL) return;
    pos++;

    while (*pos == ' ' || *pos == '\t') pos++;

    if (*pos == '"') {
        pos++;
        size_t i = 0;
        while (*pos != '\0' && *pos != '"' && i < value_size - 1) {
            value[i++] = *pos++;
        }
        value[i] = '\0';
    } else {
        size_t i = 0;
        while (*pos != '\0' && *pos != ',' && *pos != '}' && *pos != '\r' && *pos != '\n' &&
               i < value_size - 1) {
            value[i++] = *pos++;
        }
        value[i] = '\0';
    }
}

/* -------------------------------------------------------------------------
   get_cookie - reads the academia_session cookie out of the request.
   ------------------------------------------------------------------------- */
void get_cookie(const char *request, char *token, size_t token_size) {
    token[0] = '\0';
    const char *cookie = strstr(request, "Cookie:");
    if (cookie == NULL) return;

    const char *pos = strstr(cookie, "academia_session=");
    if (pos == NULL) return;
    pos += strlen("academia_session=");

    size_t i = 0;
    while (*pos != '\0' && *pos != ';' && *pos != '\r' && *pos != '\n' && i < token_size - 1) {
        token[i++] = *pos++;
    }
    token[i] = '\0';
}

/* -------------------------------------------------------------------------
   get_mime_type
   ------------------------------------------------------------------------- */
const char *get_mime_type(const char *path) {
    const char *dot = strrchr(path, '.');
    if (dot == NULL) return "text/html";
    if (_stricmp(dot, ".html") == 0 || _stricmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    if (_stricmp(dot, ".css") == 0) return "text/css; charset=utf-8";
    if (_stricmp(dot, ".js") == 0) return "application/javascript; charset=utf-8";
    if (_stricmp(dot, ".png") == 0) return "image/png";
    if (_stricmp(dot, ".jpg") == 0 || _stricmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (_stricmp(dot, ".svg") == 0) return "image/svg+xml";
    if (_stricmp(dot, ".json") == 0) return "application/json; charset=utf-8";
    if (_stricmp(dot, ".mp4") == 0) return "video/mp4";
    return "application/octet-stream";
}

static int file_exists(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------
   API: Login
   On success the session cookie is set, and the signed-in user's state file
   is refreshed so their portal shows live database records.
   ------------------------------------------------------------------------- */
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
        snprintf(cookie_header, sizeof(cookie_header),
                 "Set-Cookie: academia_session=%s; Path=/; HttpOnly; SameSite=Lax; Max-Age=28800\r\n",
                 token);

        if (strcmp(role, "student") == 0)      update_student_frontend_state(user_id);
        else if (strcmp(role, "staff") == 0)   update_staff_frontend_state(user_id);
        else if (strcmp(role, "parent") == 0)  update_parent_frontend_state(user_id);
    }

    send_response(client, 200, "OK", "application/json", response, cookie_header);
}

/* -------------------------------------------------------------------------
   API: Session check
   ------------------------------------------------------------------------- */
void handle_api_session(SOCKET client, const char *request) {
    char token[128] = "";
    get_cookie(request, token, sizeof(token));

    int user_id = 0;
    char role[32] = "";
    char name[128] = "";
    char safe_name[256];
    char safe_role[64];

    if (auth_check(token, &user_id, role, sizeof(role), name, sizeof(name))) {
        char json[512];
        escape_json_text_public(name, safe_name, sizeof(safe_name));
        escape_json_text_public(role, safe_role, sizeof(safe_role));
        snprintf(json, sizeof(json),
                 "{\"authenticated\":true,\"user\":{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
                 user_id, safe_name, safe_role);
        send_response(client, 200, "OK", "application/json", json, NULL);
    } else {
        send_response(client, 200, "OK", "application/json",
                      "{\"authenticated\":false,\"user\":null}", NULL);
    }
}

/* -------------------------------------------------------------------------
   API: Logout
   Ends the session and clears the cookie, so the portals stop seeing a user.
   ------------------------------------------------------------------------- */
void handle_api_logout(SOCKET client, const char *request) {
    char token[128] = "";
    get_cookie(request, token, sizeof(token));
    auth_logout(token);

    send_response(client, 200, "OK", "application/json",
                  "{\"success\":true,\"message\":\"Signed out.\"}",
                  "Set-Cookie: academia_session=; Path=/; HttpOnly; Max-Age=0\r\n");
}

/* -------------------------------------------------------------------------
   API: Student data (JSON)
   The id comes from the session cookie, never from the URL, so one student
   cannot read another's record.
   ------------------------------------------------------------------------- */
void handle_api_student_data_for(SOCKET client, int student_id) {
    char json[16384];
    size_t offset = 0;
    sqlite3_stmt *stmt = NULL;

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "{\"success\":true,\"student\":{");

    const char *sql_profile =
        "SELECT user.id, user.username, user.first_name, user.last_name, student.grade_level, student.gpa "
        "FROM user JOIN student ON user.id = student.id WHERE user.id = ?;";
    if (sqlite3_prepare_v2(db, sql_profile, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "\"userId\":%d,\"username\":\"%s\",\"firstName\":\"%s\",\"lastName\":\"%s\",\"gradeLevel\":%d,\"gpa\":%.2f",
                               sqlite3_column_int(stmt, 0),
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3),
                               sqlite3_column_int(stmt, 4),
                               sqlite3_column_double(stmt, 5));
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "},\"grades\":[");

    const char *sql_grades =
        "SELECT course.course_name, grade.score FROM grade "
        "JOIN course ON grade.course_id = course.id WHERE student_id = ?;";
    if (sqlite3_prepare_v2(db, sql_grades, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
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
    offset += snprintf(json + offset, sizeof(json) - offset, "],\"attendance\":[");

    const char *sql_att =
        "SELECT course.course_name, attendance.present FROM attendance "
        "JOIN course ON attendance.course_id = course.id WHERE student_id = ?;";
    if (sqlite3_prepare_v2(db, sql_att, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"status\":\"%s\"}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_int(stmt, 1) ? "Present" : "Absent");
            first = 0;
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "],\"assignments\":[");

    const char *sql_ass =
        "SELECT course.course_name, assignment.assignment_name, assignment.due_date "
        "FROM assignment JOIN course ON assignment.course_id = course.id "
        "JOIN enrollment ON enrollment.course_id = course.id "
        "WHERE enrollment.student_id = ? AND enrollment.status = 1 ORDER BY assignment.due_date;";
    if (sqlite3_prepare_v2(db, sql_ass, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"assignmentName\":\"%s\",\"dueDate\":\"%s\"}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2));
            first = 0;
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "],\"announcements\":[");

    const char *sql_ann =
        "SELECT course.course_name, announcement.title, announcement.message, announcement.announcement_date "
        "FROM announcement LEFT JOIN course ON announcement.course_id = course.id ORDER BY announcement.id DESC;";
    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *c = sqlite3_column_text(stmt, 0);
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"title\":\"%s\",\"message\":\"%s\",\"date\":\"%s\"}",
                               first ? "" : ",",
                               c ? (const char *)c : "General Announcement",
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3));
            first = 0;
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "]}");

    send_response(client, 200, "OK", "application/json", json, NULL);
}

void handle_api_student_data(SOCKET client, const char *request) {
    char token[128] = "";
    int user_id = 0;
    char role[32] = "";
    char name[128] = "";
    get_cookie(request, token, sizeof(token));

    if (!auth_check(token, &user_id, role, sizeof(role), name, sizeof(name))) {
        send_response(client, 401, "Unauthorized", "application/json",
                      "{\"success\":false,\"message\":\"Not signed in.\"}", NULL);
        return;
    }
    handle_api_student_data_for(client, user_id);
}

/* -------------------------------------------------------------------------
   API: Parent data (JSON) - the parent's own id from the cookie.
   ------------------------------------------------------------------------- */
void handle_api_parent_data_for(SOCKET client, int parent_id) {
    char json[16384];
    size_t offset = 0;
    sqlite3_stmt *stmt = NULL;

    offset += snprintf(json + offset, sizeof(json) - offset,
                       "{\"success\":true,\"parent\":{");

    const char *sql_p =
        "SELECT user.id, user.username, user.first_name, user.last_name, parent.phone "
        "FROM user JOIN parent ON user.id = parent.id WHERE user.id = ?;";
    if (sqlite3_prepare_v2(db, sql_p, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, parent_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "\"userId\":%d,\"username\":\"%s\",\"firstName\":\"%s\",\"lastName\":\"%s\",\"phone\":\"%s\"",
                               sqlite3_column_int(stmt, 0),
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3),
                               sqlite3_column_text(stmt, 4));
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "},\"student\":{");

    int child_id = 0;
    const char *sql_child =
        "SELECT user.id, user.first_name, user.last_name, student.grade_level, student.gpa "
        "FROM student JOIN user ON student.id = user.id WHERE student.parent_id = ?;";
    if (sqlite3_prepare_v2(db, sql_child, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, parent_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            child_id = sqlite3_column_int(stmt, 0);
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "\"studentId\":%d,\"firstName\":\"%s\",\"lastName\":\"%s\",\"gradeLevel\":%d,\"gpa\":%.2f",
                               child_id,
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_int(stmt, 3),
                               sqlite3_column_double(stmt, 4));
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "},\"grades\":[");

    const char *sql_grades =
        "SELECT course.course_name, grade.score FROM grade "
        "JOIN course ON grade.course_id = course.id WHERE student_id = ?;";
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
    offset += snprintf(json + offset, sizeof(json) - offset, "],\"attendance\":[");

    const char *sql_att =
        "SELECT course.course_name, attendance.present FROM attendance "
        "JOIN course ON attendance.course_id = course.id WHERE student_id = ?;";
    if (sqlite3_prepare_v2(db, sql_att, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, child_id);
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"status\":\"%s\"}",
                               first ? "" : ",",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_int(stmt, 1) ? "Present" : "Absent");
            first = 0;
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "],\"announcements\":[");

    const char *sql_ann =
        "SELECT course.course_name, announcement.title, announcement.message, announcement.announcement_date "
        "FROM announcement LEFT JOIN course ON announcement.course_id = course.id ORDER BY announcement.id DESC;";
    if (sqlite3_prepare_v2(db, sql_ann, -1, &stmt, NULL) == SQLITE_OK) {
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *c = sqlite3_column_text(stmt, 0);
            offset += snprintf(json + offset, sizeof(json) - offset,
                               "%s{\"course\":\"%s\",\"title\":\"%s\",\"message\":\"%s\",\"date\":\"%s\"}",
                               first ? "" : ",",
                               c ? (const char *)c : "General Announcement",
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3));
            first = 0;
        }
        sqlite3_finalize(stmt);
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "]}");

    send_response(client, 200, "OK", "application/json", json, NULL);
}

void handle_api_parent_data(SOCKET client, const char *request) {
    char token[128] = "";
    int user_id = 0;
    char role[32] = "";
    char name[128] = "";
    get_cookie(request, token, sizeof(token));

    if (!auth_check(token, &user_id, role, sizeof(role), name, sizeof(name))) {
        send_response(client, 401, "Unauthorized", "application/json",
                      "{\"success\":false,\"message\":\"Not signed in.\"}", NULL);
        return;
    }
    handle_api_parent_data_for(client, user_id);
}

/* -------------------------------------------------------------------------
   API: Staff write actions - grade, attendance, announcement.
   ------------------------------------------------------------------------- */
void handle_api_staff_grade(SOCKET client, const char *body) {
    char student_id_text[32] = "";
    char course_id_text[32] = "";
    char score_text[32] = "";
    get_json_value(body, "studentId", student_id_text, sizeof(student_id_text));
    get_json_value(body, "courseId", course_id_text, sizeof(course_id_text));
    get_json_value(body, "score", score_text, sizeof(score_text));

    int student_id = atoi(student_id_text);
    int course_id = atoi(course_id_text);
    double score = atof(score_text);
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (student_id > 0 && course_id > 0 && score >= 0.0 && score <= 100.0) {
        /* Replace the score if a row already exists, otherwise insert one. */
        const char *sql =
            "INSERT INTO grade (student_id, course_id, score) VALUES (?, ?, ?) "
            "ON CONFLICT DO UPDATE SET score = excluded.score;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, student_id);
            sqlite3_bind_int(stmt, 2, course_id);
            sqlite3_bind_double(stmt, 3, score);
            ok = sqlite3_step(stmt) == SQLITE_DONE;
            sqlite3_finalize(stmt);
        }

        /* Recalculate the student's 4.0 GPA from their grade average. */
        if (ok) {
            const char *gpa_sql =
                "UPDATE student SET gpa = "
                "(SELECT MIN(4.0, MAX(0.0, (AVG(score) - 50.0) / 12.5)) FROM grade WHERE student_id = ?) "
                "WHERE id = ?;";
            if (sqlite3_prepare_v2(db, gpa_sql, -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, student_id);
                sqlite3_bind_int(stmt, 2, student_id);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            update_student_frontend_state(student_id);
        }
    }

    send_response(client, 200, "OK", "application/json",
                  ok ? "{\"success\":true,\"message\":\"Grade recorded.\"}"
                     : "{\"success\":false,\"message\":\"Grade not recorded.\"}", NULL);
}

void handle_api_staff_attendance(SOCKET client, const char *body) {
    char student_id_text[32] = "";
    char course_id_text[32] = "";
    char present_text[32] = "";
    get_json_value(body, "studentId", student_id_text, sizeof(student_id_text));
    get_json_value(body, "courseId", course_id_text, sizeof(course_id_text));
    get_json_value(body, "present", present_text, sizeof(present_text));

    int student_id = atoi(student_id_text);
    int course_id = atoi(course_id_text);
    int present = atoi(present_text);
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (student_id > 0 && course_id > 0) {
        const char *sql =
            "INSERT INTO attendance (student_id, course_id, present) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, student_id);
            sqlite3_bind_int(stmt, 2, course_id);
            sqlite3_bind_int(stmt, 3, present ? 1 : 0);
            ok = sqlite3_step(stmt) == SQLITE_DONE;
            sqlite3_finalize(stmt);
        }
        if (ok) update_student_frontend_state(student_id);
    }

    send_response(client, 200, "OK", "application/json",
                  ok ? "{\"success\":true,\"message\":\"Attendance recorded.\"}"
                     : "{\"success\":false,\"message\":\"Attendance not recorded.\"}", NULL);
}

void handle_api_staff_announcement(SOCKET client, const char *body) {
    char course_id_text[32] = "";
    char title[128] = "";
    char message[512] = "";
    get_json_value(body, "courseId", course_id_text, sizeof(course_id_text));
    get_json_value(body, "title", title, sizeof(title));
    get_json_value(body, "message", message, sizeof(message));

    int course_id = atoi(course_id_text);
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (title[0] != '\0' && message[0] != '\0') {
        const char *sql =
            "INSERT INTO announcement (course_id, title, message, announcement_date) "
            "VALUES (?, ?, ?, date('now'));";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            if (course_id > 0) sqlite3_bind_int(stmt, 1, course_id);
            else               sqlite3_bind_null(stmt, 1);
            sqlite3_bind_text(stmt, 2, title, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, message, -1, SQLITE_TRANSIENT);
            ok = sqlite3_step(stmt) == SQLITE_DONE;
            sqlite3_finalize(stmt);
        }
    }

    send_response(client, 200, "OK", "application/json",
                  ok ? "{\"success\":true,\"message\":\"Announcement published.\"}"
                     : "{\"success\":false,\"message\":\"Announcement not published.\"}", NULL);
}

/* -------------------------------------------------------------------------
   main - set up Winsock, open the database, serve requests.
   ------------------------------------------------------------------------- */
int main(void) {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);
    char *request;
    char method[16], path[512];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[Server] WSAStartup failed.\n");
        return 1;
    }

    db = open_database();
    if (db == NULL) {
        printf("[Server] Database could not be opened.\n");
        WSACleanup();
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("[Server] Socket creation failed.\n");
        close_database(db);
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[Server] Bind failed on port %d.\n", PORT);
        closesocket(server_socket);
        close_database(db);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[Server] Listen failed.\n");
        closesocket(server_socket);
        close_database(db);
        WSACleanup();
        return 1;
    }

    printf("[Server] Academia running at http://localhost:%d\n", PORT);

    request = (char *)malloc(BUFFER_SIZE);
    if (request == NULL) {
        printf("[Server] Out of memory.\n");
        closesocket(server_socket);
        close_database(db);
        WSACleanup();
        return 1;
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }

        int received = recv(client_socket, request, BUFFER_SIZE - 1, 0);
        if (received <= 0) {
            closesocket(client_socket);
            continue;
        }
        request[received] = '\0';

        method[0] = '\0';
        path[0] = '\0';
        sscanf(request, "%15s %511s", method, path);

        /* --- API routes ---------------------------------------------------- */
        if (strcmp(method, "OPTIONS") == 0) {
            send_response(client_socket, 204, "No Content", "text/plain", NULL, NULL);
        }
        else if (strstr(request, "POST /api/login") != NULL) {
            char *body = strstr(request, "\r\n\r\n");
            handle_api_login(client_socket, body != NULL ? body + 4 : "");
        }
        else if (strstr(request, "POST /api/logout") != NULL) {
            handle_api_logout(client_socket, request);
        }
        else if (strstr(request, "GET /api/session") != NULL) {
            handle_api_session(client_socket, request);
        }
        else if (strstr(request, "GET /api/student-data") != NULL) {
            handle_api_student_data(client_socket, request);
        }
        else if (strstr(request, "GET /api/staff-data") != NULL) {
            /* Staff read their own state file, written at login. */
            char token[128] = "";
            int user_id = 0;
            char role[32] = "", name[128] = "";
            get_cookie(request, token, sizeof(token));
            if (auth_check(token, &user_id, role, sizeof(role), name, sizeof(name)) &&
                strcmp(role, "staff") == 0) {
                update_staff_frontend_state(user_id);
                send_response(client_socket, 200, "OK", "application/json",
                              "{\"success\":true,\"message\":\"Staff state refreshed.\"}", NULL);
            } else {
                send_response(client_socket, 401, "Unauthorized", "application/json",
                              "{\"success\":false,\"message\":\"Not signed in as staff.\"}", NULL);
            }
        }
        else if (strstr(request, "GET /api/parent-data") != NULL) {
            handle_api_parent_data(client_socket, request);
        }
        else if (strstr(request, "POST /api/staff/grade") != NULL) {
            char *body = strstr(request, "\r\n\r\n");
            handle_api_staff_grade(client_socket, body != NULL ? body + 4 : "");
        }
        else if (strstr(request, "POST /api/staff/attendance") != NULL) {
            char *body = strstr(request, "\r\n\r\n");
            handle_api_staff_attendance(client_socket, body != NULL ? body + 4 : "");
        }
        else if (strstr(request, "POST /api/staff/announcement") != NULL) {
            char *body = strstr(request, "\r\n\r\n");
            handle_api_staff_announcement(client_socket, body != NULL ? body + 4 : "");
        }
        /* --- Static files -------------------------------------------------- */
        else {
            char filepath[768];
            const char *relative = (strcmp(path, "/") == 0) ? "/HTML/Academia_Landing.html" : path;

            /* A request for /HTML/x.html maps to FrontEnd/HTML/x.html. Do not
               allow ../ out of the FrontEnd folder. */
            if (strstr(relative, "..") != NULL) {
                send_response(client_socket, 403, "Forbidden", "text/html",
                              "<h1>403 - Forbidden</h1>", NULL);
            } else {
                snprintf(filepath, sizeof(filepath), "FrontEnd%s", relative);
                if (!file_exists(filepath)) {
                    snprintf(filepath, sizeof(filepath), "Academia/FrontEnd%s", relative);
                }
                send_file(client_socket, filepath, get_mime_type(filepath));
            }
        }

        closesocket(client_socket);
    }

    free(request);
    closesocket(server_socket);
    close_database(db);
    WSACleanup();
    return 0;
}