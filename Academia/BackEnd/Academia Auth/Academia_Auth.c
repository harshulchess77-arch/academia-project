/*
   ==========================================================================
   ACADEMIA - AUTHENTICATION MODULE  (Academia_Auth.c)
   Checks a username and password, and remembers who is signed in
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================
*/

#include "Academia_Auth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsqlite/winsqlite3.h>

static char active_token[128] = "";
static int active_user_id = 0;
static char active_role[32] = "";
static char active_name[128] = "";

/* -------------------------------------------------------------------------
   escape_json_text
   Copies text into output, adding a backslash before any character that would
   break a JSON string.
   ------------------------------------------------------------------------- */
static void escape_json_text(const char *input, char *output, size_t output_size) {
    size_t i;
    size_t used = 0;

    if (input == NULL || output == NULL || output_size == 0) {
        return;
    }

    for (i = 0; input[i] != '\0' && used + 2 < output_size; i++) {
        if (input[i] == '"' || input[i] == '\\') {
            output[used++] = '\\';
            output[used++] = input[i];
        } else if (input[i] == '\n') {
            output[used++] = '\\';
            output[used++] = 'n';
        } else if (input[i] == '\t') {
            output[used++] = '\\';
            output[used++] = 't';
        } else if (input[i] == '\r') {
            /* Carriage returns are dropped. */
        } else {
            output[used++] = input[i];
        }
    }

    output[used] = '\0';
}

/* -------------------------------------------------------------------------
   start_session
   Notes who has signed in and makes a session token for them.
   The token uses seconds, nanoseconds and a counter, so two logins in the
   same second never collide.
   ------------------------------------------------------------------------- */
static void start_session(int user_id, const char *role, const char *name, char *token,
                          size_t token_size) {
    struct timespec ts;
    static unsigned int counter = 0;

    timespec_get(&ts, TIME_UTC);
    counter++;

    snprintf(active_token, sizeof(active_token), "session-%lld-%ld-%u-%d",
             (long long)ts.tv_sec, (long)ts.tv_nsec, counter, user_id);
    snprintf(token, token_size, "%s", active_token);

    active_user_id = user_id;
    snprintf(active_role, sizeof(active_role), "%s", role);
    snprintf(active_name, sizeof(active_name), "%s", name);
}

/* -------------------------------------------------------------------------
   auth_login
   Checks the username and password against the user table.

   Inputs :  the database, the username and password typed into the form
   Outputs:  a JSON reply in `response`, the session details in `token`,
             `user_id`, `role` and `name`
   Returns:  1 when the password was right, 0 when it was not.
   ------------------------------------------------------------------------- */
int auth_login(sqlite3 *database, const char *username, const char *password,
               char *response, size_t response_size, char *token,
               size_t token_size, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    sqlite3_stmt *statement = NULL;
    const char *sql;
    int step_result;
    int found_id;
    const unsigned char *first;
    const unsigned char *last;
    const unsigned char *stored_role;
    char full_name[128];
    char safe_name[256];
    char safe_role[64];

    if (database == NULL || username == NULL || password == NULL) {
        snprintf(response, response_size,
                 "{\"success\":false,\"message\":\"Database not connected.\"}");
        return 0;
    }

    /* The ? marks are what stop someone typing SQL into the form. */
    sql = "SELECT id, first_name, last_name, role "
          "FROM user WHERE username = ? AND password = ?;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, NULL) != SQLITE_OK) {
        snprintf(response, response_size,
                 "{\"success\":false,\"message\":\"Database error.\"}");
        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);

    step_result = sqlite3_step(statement);

    /* The same message is used whether the username or the password was at
       fault, so a stranger cannot work out which usernames exist. */
    if (step_result != SQLITE_ROW) {
        sqlite3_finalize(statement);
        snprintf(response, response_size,
                 "{\"success\":false,\"message\":\"Invalid username or password.\"}");
        return 0;
    }

    found_id = sqlite3_column_int(statement, 0);
    first = sqlite3_column_text(statement, 1);
    last = sqlite3_column_text(statement, 2);
    stored_role = sqlite3_column_text(statement, 3);

    snprintf(full_name, sizeof(full_name), "%s %s",
             first != NULL ? (const char *)first : "",
             last != NULL ? (const char *)last : "");

    snprintf(role, role_size, "%s",
             stored_role != NULL ? (const char *)stored_role : "");

    sqlite3_finalize(statement);

    start_session(found_id, role, full_name, token, token_size);
    snprintf(name, name_size, "%s", full_name);
    *user_id = found_id;

    escape_json_text(full_name, safe_name, sizeof(safe_name));
    escape_json_text(role, safe_role, sizeof(safe_role));

    snprintf(response, response_size,
             "{\"success\":true,\"message\":\"Login successful.\","
             "\"token\":\"%s\","
             "\"user\":{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
             active_token, found_id, safe_name, safe_role);

    return 1;
}

/* -------------------------------------------------------------------------
   auth_check
   Is this cookie the current session? 1 when it is, 0 when it is not.
   ------------------------------------------------------------------------- */
int auth_check(const char *token, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    if (active_token[0] == '\0') {
        return 0;
    }

    if (token == NULL || strcmp(token, active_token) != 0) {
        return 0;
    }

    *user_id = active_user_id;
    snprintf(role, role_size, "%s", active_role);
    snprintf(name, name_size, "%s", active_name);
    return 1;
}

/* -------------------------------------------------------------------------
   auth_logout
   Forgets the session, but only if the caller holds its token.
   ------------------------------------------------------------------------- */
void auth_logout(const char *token) {
    if (token == NULL || strcmp(token, active_token) != 0) {
        return;
    }

    active_token[0] = '\0';
    active_user_id = 0;
    active_role[0] = '\0';
    active_name[0] = '\0';
}