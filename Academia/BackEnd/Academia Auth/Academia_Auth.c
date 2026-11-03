/*
Dmitrii Buianov
CS50 C Project
SQL Database & Login Method Connection
Academia
October 22, 2026
*/

// Main Method
// We include the libraries such as 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>

#include "Academia_Auth.h"

// Current active session.
// This first version supports one active user.

char active_token[100] = "";
int active_user_id = 0;
char active_role[30] = "";
char active_name[100] = "";

// Convert text into safe JSON text.
void auth_json_escape(
    const char *input,
    char *output,
    size_t output_size
)
{
    size_t j = 0;

    for (size_t i = 0; input[i] != '\0'; i++)
    {
        if (j + 2 >= output_size)
        {
            break;
        }

        if (input[i] == '"' || input[i] == '\\')
        {
            output[j++] = '\\';
        }

        if (input[i] == '\n')
        {
            output[j++] = '\\';
            output[j++] = 'n';
        }
        else
        {
            output[j++] = input[i];
        }
    }

    output[j] = '\0';
}

// Login function.
int auth_login(
    sqlite3 *database,
    const char *username,
    const char *password,
    char *response,
    size_t response_size,
    char *token,
    size_t token_size,
    int *user_id,
    char *role,
    size_t role_size,
    char *name,
    size_t name_size
)
{
    sqlite3_stmt *statement;

    const char *sql =
        "SELECT id, first_name, last_name, role "
        "FROM user "
        "WHERE username = ? AND password = ?";

    int result = sqlite3_prepare_v2(
        database,
        sql,
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        snprintf(
            response,
            response_size,
            "{\"success\":false,\"message\":\"Database error.\"}"
        );

        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);

        snprintf(
            response,
            response_size,
            "{\"success\":false,\"message\":\"Invalid username or password.\"}"
        );

        return 0;
    }

    *user_id = sqlite3_column_int(statement, 0);

    const unsigned char *first =
        sqlite3_column_text(statement, 1);

    const unsigned char *last =
        sqlite3_column_text(statement, 2);

    const unsigned char *user_role =
        sqlite3_column_text(statement, 3);

    char first_name[100] = "";
    char last_name[100] = "";

    if (first != NULL)
    {
        snprintf(first_name, sizeof(first_name), "%s", first);
    }

    if (last != NULL)
    {
        snprintf(last_name, sizeof(last_name), "%s", last);
    }

    snprintf(
        role,
        role_size,
        "%s",
        user_role != NULL ? (const char *)user_role : ""
    );

    snprintf(
        name,
        name_size,
        "%s %s",
        first_name,
        last_name
    );

    sqlite3_finalize(statement);

    // Create a simple local development token.
    // Replace this with a cryptographically secure token
    // before using the application beyond localhost.

    snprintf(
        active_token,
        sizeof(active_token),
        "%ld-%d-%d",
        (long)time(NULL),
        rand(),
        *user_id
    );

    snprintf(token, token_size, "%s", active_token);

    active_user_id = *user_id;

    snprintf(
        active_role,
        sizeof(active_role),
        "%s",
        role
    );

    snprintf(
        active_name,
        sizeof(active_name),
        "%s",
        name
    );

    char safe_name[200];
    char safe_role[60];

    auth_json_escape(name, safe_name, sizeof(safe_name));
    auth_json_escape(role, safe_role, sizeof(safe_role));

    snprintf(
        response,
        response_size,
        "{\"success\":true,\"message\":\"Login successful.\","
        "\"user\":{\"id\":%d,\"name\":\"%s\",\"role\":\"%s\"}}",
        *user_id,
        safe_name,
        safe_role
    );

    return 1;
}

// Check current session.
int auth_check(
    const char *token,
    int *user_id,
    char *role,
    size_t role_size,
    char *name,
    size_t name_size
)
{
    if (
        token == NULL ||
        active_token[0] == '\0' ||
        strcmp(token, active_token) != 0
    )
    {
        return 0;
    }

    *user_id = active_user_id;

    snprintf(role, role_size, "%s", active_role);
    snprintf(name, name_size, "%s", active_name);

    return 1;
}

// Logout.
void auth_logout(const char *token)
{
    if (
        token != NULL &&
        strcmp(token, active_token) == 0
    )
    {
        active_token[0] = '\0';
        active_user_id = 0;
        active_role[0] = '\0';
        active_name[0] = '\0';
    }
}

/*#include <stdio.h>
#include "../Academia Common/Academia_Common.h"
#include "Academia_Auth.h"

Checks the username, password, and role against the user table.

Returns the verified user's ID when login succeeds.
Returns -1 when login fails or a database error occurs.

int checkUserLogin(
    const char *username,
    const char *password,
    const char *role
)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int query_result;
    int user_id = -1;

    const char *sql =
        "SELECT id, role "
        "FROM user "
        "WHERE username = ? "
        "AND password = ? "
        "AND role = ?;";


    if (
        username == NULL ||
        password == NULL ||
        role == NULL
    )
    {
        printf(
            "[Login Error] Username, password, and role are required.\n"
        );

        return -1;
    }


    // Uses the common database connection function.
    db = open_academia_database();

    if (db == NULL)
    {
        return -1;
    }


    if (
        sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK
    )
    {
        printf(
            "[Login Error] Cannot prepare login query: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);

        return -1;
    }

    Binds the login information safely instead of inserting
    user input directly into the SQL string.
    
    if (
        sqlite3_bind_text(
            stmt,
            1,
            username,
            -1,
            SQLITE_TRANSIENT
        ) != SQLITE_OK ||
        sqlite3_bind_text(
            stmt,
            2,
            password,
            -1,
            SQLITE_TRANSIENT
        ) != SQLITE_OK ||
        sqlite3_bind_text(
            stmt,
            3,
            role,
            -1,
            SQLITE_TRANSIENT
        ) != SQLITE_OK
    )
    {
        printf(
            "[Login Error] Cannot bind login information: %s\n",
            sqlite3_errmsg(db)
        );

        finalize_statement(&stmt);
        sqlite3_close(db);

        return -1;
    }


    query_result = sqlite3_step(stmt);

    if (query_result == SQLITE_ROW)
    {
        const unsigned char *database_role;

        user_id = sqlite3_column_int(
            stmt,
            0
        );

        database_role = sqlite3_column_text(
            stmt,
            1
        );

        printf(
            "[Login Result] %s login successful. User ID: %d\n",
            database_role != NULL
                ? (const char *)database_role
                : role,
            user_id
        );
    }
    else if (query_result == SQLITE_DONE)
    {
        printf(
            "[Login Result] Invalid username, password, or role.\n"
        );
    }
    else
    {
        printf(
            "[Login Error] Login query failed: %s\n",
            sqlite3_errmsg(db)
        );
    }


    finalize_statement(&stmt);
    sqlite3_close(db);

    return user_id;
}*/