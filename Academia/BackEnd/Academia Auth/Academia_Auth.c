/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
Authentication Module
November 1, 2026
*/

#include "Academia_Auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsqlite/winsqlite3.h>

// Current active session state for local server
static char active_token[128] = "";
static int active_user_id = 0;
static char active_role[32] = "";
static char active_name[128] = "";

// Helper to escape text for JSON strings
void auth_json_escape(const char *input, char *output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) return;
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j + 2 < output_size; i++) {
        if (input[i] == '"' || input[i] == '\\') {
            output[j++] = '\\';
            output[j++] = input[i];
        } else if (input[i] == '\n') {
            output[j++] = '\\';
            output[j++] = 'n';
        } else if (input[i] == '\r') {
            /* skip carriage return */
        } else if (input[i] == '\t') {
            output[j++] = '\\';
            output[j++] = 't';
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

// Login function: validates credentials from SQLite user table
int auth_login(sqlite3 *database, const char *username, const char *password,
               char *response, size_t response_size, char *token,
               size_t token_size, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    if (database == NULL || username == NULL || password == NULL) {
        snprintf(response, response_size, "{\"success\":false,\"message\":\"Database not connected.\"}");
        return 0;
    }

    sqlite3_stmt *statement = NULL;
    const char *sql = "SELECT id, first_name, last_name, role FROM user WHERE username = ? AND password = ?;";
    int result = sqlite3_prepare_v2(database, sql, -1, &statement, NULL);

    if (result != SQLITE_OK) {
        snprintf(response, response_size, "{\"success\":false,\"message\":\"Database error.\"}");
        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
        sqlite3_finalize(statement);
        snprintf(response, response_size, "{\"success\":false,\"message\":\"Invalid username or password.\"}");
        return 0;
    }

    *user_id = sqlite3_column_int(statement, 0);
    const unsigned char *first = sqlite3_column_text(statement, 1);
    const unsigned char *last = sqlite3_column_text(statement, 2);
    const unsigned char *user_role = sqlite3_column_text(statement, 3);

    char first_name[64] = "";
    char last_name[64] = "";
    if (first != NULL) snprintf(first_name, sizeof(first_name), "%s", first);
    if (last != NULL) snprintf(last_name, sizeof(last_name), "%s", last);

    snprintf(role, role_size, "%s", user_role != NULL ? (const char *)user_role : "");
    snprintf(name, name_size, "%s %s", first_name, last_name);

    sqlite3_finalize(statement);

    // Create session token
    srand((unsigned int)time(NULL));
    snprintf(active_token, sizeof(active_token), "session-%ld-%d-%d", (long)time(NULL), rand() % 10000, *user_id);
    snprintf(token, token_size, "%s", active_token);

    active_user_id = *user_id;
    snprintf(active_role, sizeof(active_role), "%s", role);
    snprintf(active_name, sizeof(active_name), "%s", name);

    char safe_name[128];
    char safe_role[64];
    auth_json_escape(name, safe_name, sizeof(safe_name));
    auth_json_escape(role, safe_role, sizeof(safe_role));

    snprintf(response, response_size,
             "{\"success\":true,\"message\":\"Login successful.\","
             "\"token\":\"%s\","
             "\"user\":{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
             active_token, *user_id, safe_name, safe_role);

    return 1;
}

// Check session
int auth_check(const char *token, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    if (token == NULL || active_token[0] == '\0' || strcmp(token, active_token) != 0) {
        return 0;
    }

    *user_id = active_user_id;
    snprintf(role, role_size, "%s", active_role);
    snprintf(name, name_size, "%s", active_name);
    return 1;
}

// Logout
void auth_logout(const char *token) {
    if (token != NULL && strcmp(token, active_token) == 0) {
        active_token[0] = '\0';
        active_user_id = 0;
        active_role[0] = '\0';
        active_name[0] = '\0';
    }
}
