/*
   ==========================================================================
   ACADEMIA - AUTHENTICATION MODULE  (Academia_Auth.c)
   Checks a username and password, and remembers who is signed in
   CS50 C Project
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   ==========================================================================

      WHAT THIS FILE DOES
   -------------------
   Two jobs:

     1. Check a sign-in. auth_login() looks the username and password up in
        the user table. If they match, it builds a JSON reply for the browser
        and remembers the person who signed in.

     2. Check a session. Every later request carries a cookie. auth_check()
        compares that cookie with the session held here, so a page can tell
        whether the visitor is still signed in.

   This is a small local server, so only one person is signed in at a time.
   The four variables below hold that one session. A real website would use a
   table of sessions and hashed passwords; that is future work.

   HOW IT IS ORGANISED
   -------------------
   1. escape_json_text() - makes text safe to put inside a JSON string
   2. auth_login()       - check the username and password, start a session
   3. auth_check()       - is this cookie the current session?
   4. auth_logout()      - forget the current session

*/

#include "Academia_Auth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsqlite/winsqlite3.h>

/* -------------------------------------------------------------------------
   The one active session.

   active_token  is the cookie value given to the signed-in browser.
   A blank token means nobody is signed in.

   The other three hold who is signed in, so a page can show the right name
   without asking the database again on every request.
   ------------------------------------------------------------------------- */
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
   Note down who has just signed in and make a session token for them. The
   token is what the browser is given in its cookie, so later requests can be
   matched to this person.
   ------------------------------------------------------------------------- */
static void start_session(int user_id, const char *role, const char *name, char *token,
                          size_t token_size) {
    srand((unsigned int)time(NULL));

    /* The token combines the time, a random number and the user id, which is
       enough to keep one visit distinct from the next on a local server. */
    snprintf(active_token, sizeof(active_token), "session-%ld-%d-%d",
             (long)time(NULL), rand() % 10000, user_id);
    snprintf(token, token_size, "%s", active_token);

    active_user_id = user_id;
    snprintf(active_role, sizeof(active_role), "%s", role);
    snprintf(active_name, sizeof(active_name), "%s", name);
}

/* -------------------------------------------------------------------------
   auth_login
   The main sign-in check.

   Inputs :  the database, the username and password typed into the form
   Outputs:  a JSON reply in `response`, and the session details in `token`,
             `user_id`, `role` and `name`
   Returns:  1 when the password was right, 0 when it was not.

   The username and password are put into the SQL with ? marks rather than
   pasted into the text, which is what stops someone typing SQL into the form.
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

    /* Nothing can be checked without a database or a username. */
    if (database == NULL || username == NULL || password == NULL) {
        snprintf(response, response_size,
                 "{\"success\":false,\"message\":\"Database not connected.\"}");
        return 0;
    }

    /* Look for a row whose username and password both match. */
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

    /* No row means the pair is wrong. The same message is used whether the
       username or the password was at fault, so a stranger cannot work out
       which usernames exist. */
    if (step_result != SQLITE_ROW) {
        sqlite3_finalize(statement);
        snprintf(response, response_size,
                 "{\"success\":false,\"message\":\"Invalid username or password.\"}");
        return 0;
    }

    /* The row was found: read each column out of it. */
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

    /* Remember the sign-in and build the token. */
    start_session(found_id, role, full_name, token, token_size);
    snprintf(name, name_size, "%s", full_name);

    /* The reply travels inside JSON, so the two pieces of text taken from the
       database are escaped first. */
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
   Is this cookie the current session? Returns 1 when it is, 0 when it is not.
   ------------------------------------------------------------------------- */
int auth_check(const char *token, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size) {
    /* No session has been started yet, so nothing can match. */
    if (active_token[0] == '\0') {
        return 0;
    }

    /* No cookie was sent, or it belongs to an earlier visit. */
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
   Forget the session, but only if the caller holds its token. Blanking the
   token is enough to end it, because every later check compares against it.
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