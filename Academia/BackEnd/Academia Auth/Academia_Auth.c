/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
*/
#include "Academia_Auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char active_token[128] = "";
static int active_user_id = 0;
static char active_role[32] = "";
static char active_name[128] = "";

int auth_login(sqlite3 *database, const char *username, const char *password,
               char *response, size_t response_size, char *token,
               size_t token_size, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    sqlite3_stmt *statement = NULL;
    const char *sql = "SELECT id, first_name, last_name, role FROM user WHERE username = ? AND password = ?;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, NULL) != SQLITE_OK) {
        snprintf(response, response_size, "{\"success\":false,\"message\":\"Database error.\"}");
        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(statement) != SQLITE_ROW) {
        sqlite3_finalize(statement);
        snprintf(response, response_size, "{\"success\":false,\"message\":\"Invalid username or password.\"}");
        return 0;
    }

    int found_id = sqlite3_column_int(statement, 0);
    const unsigned char *first = sqlite3_column_text(statement, 1);
    const unsigned char *last = sqlite3_column_text(statement, 2);
    const unsigned char *stored_role = sqlite3_column_text(statement, 3);

    char full_name[128];
    snprintf(full_name, sizeof(full_name), "%s %s", first ? (const char*)first : "", last ? (const char*)last : "");
    snprintf(role, role_size, "%s", stored_role ? (const char*)stored_role : "");
    sqlite3_finalize(statement);

    srand((unsigned int)time(NULL));
    snprintf(active_token, sizeof(active_token), "session-%ld-%d-%d", (long)time(NULL), rand() % 10000, found_id);
    snprintf(token, token_size, "%s", active_token);

    active_user_id = found_id;
    snprintf(active_role, sizeof(active_role), "%s", role);
    snprintf(active_name, sizeof(active_name), "%s", full_name);
    snprintf(name, name_size, "%s", full_name);

    snprintf(response, response_size,
             "{\"success\":true,\"message\":\"Login successful.\",\"token\":\"%s\",\"user\":{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
             active_token, found_id, full_name, role);
    return 1;
}

int auth_check(const char *token, int *user_id, char *role, size_t role_size, char *name, size_t name_size) {
    if (active_token[0] == '\0' || token == NULL || strcmp(token, active_token) != 0) return 0;
    *user_id = active_user_id;
    snprintf(role, role_size, "%s", active_role);
    snprintf(name, name_size, "%s", active_name);
    return 1;
}

void auth_logout(const char *token) {
    if (token && strcmp(token, active_token) == 0) {
        active_token[0] = '\0';
        active_user_id = 0;
    }
}