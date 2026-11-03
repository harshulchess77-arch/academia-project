/*
Dmitrii Buianov
Seungho Kim
CS50 C Project
Academia Login Server
11.02.26
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>

#include "../Academia Database/SQLite3 Source Code/sqlite3.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 3000
#define BUFFER_SIZE 8192
#define DATABASE_PATH \
    "Academia/BackEnd/Academia Database/academia.db"


// Parent function located in Academia_Parents.c
void update_parent_frontend_state(int parent_user_id);


/*
Reads one string value from the JSON request body.
The function supports spaces around the colon in the JSON.
*/
void get_json_value(
    const char *json,
    const char *key,
    char *output,
    int output_size
)
{
    char pattern[100];
    const char *start;
    const char *end;
    int length;

    output[0] = '\0';

    // Finds the requested JSON key
    snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\"",
        key
    );

    start = strstr(json, pattern);

    if (start == NULL)
    {
        return;
    }

    // Finds the colon after the JSON key
    start = strchr(start + strlen(pattern), ':');

    if (start == NULL)
    {
        return;
    }

    start++;

    // Skips spaces between the colon and value
    while (*start != '\0' && isspace((unsigned char) *start))
    {
        start++;
    }

    if (*start != '"')
    {
        return;
    }

    start++;

    // Finds the closing quotation mark
    end = strchr(start, '"');

    if (end == NULL)
    {
        return;
    }

    length = (int) (end - start);

    // Prevents the output array from overflowing
    if (length >= output_size)
    {
        length = output_size - 1;
    }

    strncpy(output, start, length);
    output[length] = '\0';
}


/*
Checks the username, password, and role in the SQLite database.
It returns the user ID when valid and returns -1 when invalid.
*/
int authenticate_user(
    const char *username,
    const char *password,
    const char *role
)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int user_id = -1;

    // Opens the main Academia database
    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK)
    {
        printf(
            "[Error] Cannot open database: %s\n",
            db != NULL
                ? sqlite3_errmsg(db)
                : "Unknown SQLite error"
        );

        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return -1;
    }

    // Searches for an account matching all login values
    const char *sql =
        "SELECT id "
        "FROM user "
        "WHERE username = ? "
        "AND password = ? "
        "AND role = ?;";

    // Prepares the login query for execution
    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        printf(
            "[Error] Login query failed: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);
        return -1;
    }

    // Inserts the login values into the SQL query
    sqlite3_bind_text(
        stmt,
        1,
        username,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        2,
        password,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        3,
        role,
        -1,
        SQLITE_TRANSIENT
    );

    // Reads the user ID when a matching account exists
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        user_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return user_id;
}


/*
Sends a plain-text HTTP response to the browser.
The response includes headers required for Live Server requests.
*/
void send_text_response(
    SOCKET client_socket,
    int status_code,
    const char *status_text,
    const char *body
)
{
    char response[1024];
    int body_length;

    body_length = (int) strlen(body);

    snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code,
        status_text,
        body_length,
        body
    );

    send(
        client_socket,
        response,
        (int) strlen(response),
        0
    );
}


/*
Handles the browser's CORS preflight request.
This allows the Auth page to send JSON from Live Server.
*/
void send_options_response(SOCKET client_socket)
{
    const char *response =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(
        client_socket,
        response,
        (int) strlen(response),
        0
    );
}


/*
Processes the POST /login request from Academia.js.
A successful parent login also creates parents_states.js.
*/
void handle_login(
    SOCKET client_socket,
    const char *request
)
{
    const char *body;

    char username[100];
    char password[100];
    char role[100];

    int user_id;

    // Finds the beginning of the HTTP request body
    body = strstr(request, "\r\n\r\n");

    if (body == NULL)
    {
        send_text_response(
            client_socket,
            400,
            "Bad Request",
            "invalid"
        );

        return;
    }

    body += 4;

    // Reads the login values from the JSON body
    get_json_value(
        body,
        "username",
        username,
        sizeof(username)
    );

    get_json_value(
        body,
        "password",
        password,
        sizeof(password)
    );

    get_json_value(
        body,
        "role",
        role,
        sizeof(role)
    );

    // Rejects requests with missing login information
    if (
        username[0] == '\0' ||
        password[0] == '\0' ||
        role[0] == '\0'
    )
    {
        send_text_response(
            client_socket,
            400,
            "Bad Request",
            "invalid"
        );

        return;
    }

    printf(
        "[Login Request] Username: %s | Role: %s\n",
        username,
        role
    );

    // Checks the account against the SQLite database
    user_id = authenticate_user(
        username,
        password,
        role
    );

    if (user_id == -1)
    {
        printf("[Login Result] Invalid credentials.\n");

        send_text_response(
            client_socket,
            401,
            "Unauthorized",
            "invalid"
        );

        return;
    }

    // Creates the parent data file before opening the Parent Portal
    if (strcmp(role, "parent") == 0)
    {
        update_parent_frontend_state(user_id);

        printf(
            "[Login Result] Parent login successful. User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "parent"
        );
    }
    else if (strcmp(role, "student") == 0)
    {
        printf(
            "[Login Result] Student login successful. User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "student"
        );
    }
    else if (strcmp(role, "staff") == 0)
    {
        printf(
            "[Login Result] Staff login successful. User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "staff"
        );
    }
    else
    {
        send_text_response(
            client_socket,
            400,
            "Bad Request",
            "invalid"
        );
    }
}


/*
Starts the Academia login server on port 3000.
The program continues accepting requests until it is stopped.
*/
int main(void)
{
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in server;

    // Starts the Windows socket system
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf(
            "[Error] WSAStartup failed: %d\n",
            WSAGetLastError()
        );

        return 1;
    }

    // Creates the TCP server socket
    server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (server_socket == INVALID_SOCKET)
    {
        printf(
            "[Error] Socket creation failed: %d\n",
            WSAGetLastError()
        );

        WSACleanup();
        return 1;
    }

    memset(&server, 0, sizeof(server));

    // Allows the server to accept local requests on port 3000
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    // Connects the socket to the selected port
    if (bind(
            server_socket,
            (struct sockaddr *) &server,
            sizeof(server)
        ) == SOCKET_ERROR)
    {
        printf(
            "[Error] Bind failed: %d\n",
            WSAGetLastError()
        );

        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    // Begins listening for browser connections
    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        printf(
            "[Error] Listen failed: %d\n",
            WSAGetLastError()
        );

        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    printf("Academia Server Running\n");
    printf("Login endpoint: http://localhost:%d/login\n", PORT);
    printf("Press Ctrl+C to stop the server.\n\n");


    // Accepts and processes browser requests continuously
    while (1)
    {
        SOCKET client_socket;
        char request[BUFFER_SIZE];

        int received;

        client_socket = accept(
            server_socket,
            NULL,
            NULL
        );

        if (client_socket == INVALID_SOCKET)
        {
            printf(
                "[Error] Accept failed: %d\n",
                WSAGetLastError()
            );

            continue;
        }

        memset(request, 0, sizeof(request));

        // Reads the HTTP request and leaves space for the null character
        received = recv(
            client_socket,
            request,
            sizeof(request) - 1,
            0
        );

        if (received <= 0)
        {
            closesocket(client_socket);
            continue;
        }

        request[received] = '\0';

        // Handles the browser's CORS preflight request
        if (strstr(request, "OPTIONS /login") != NULL)
        {
            send_options_response(client_socket);
        }

        // Handles login information sent by Academia.js
        else if (strstr(request, "POST /login") != NULL)
        {
            handle_login(
                client_socket,
                request
            );
        }

        // Rejects all unsupported server routes
        else
        {
            send_text_response(
                client_socket,
                404,
                "Not Found",
                "Not Found"
            );
        }

        closesocket(client_socket);
    }


    closesocket(server_socket);
    WSACleanup();

    return 0;
}