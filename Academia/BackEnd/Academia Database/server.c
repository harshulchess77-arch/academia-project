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

#include "SQLite3 Source Code/sqlite3.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 3000
#define BUFFER_SIZE 8192

#define DATABASE_PATH \
    "Academia/BackEnd/Academia Database/academia.db"


// State functions located in the Parent, Student, and Staff modules.
void update_parent_frontend_state(int parent_user_id);
void update_student_frontend_state(int student_user_id);
void update_staff_frontend_state(int staff_user_id);


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

    // Creates the JSON key pattern.
    snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\"",
        key
    );

    // Finds the requested key in the JSON body.
    start = strstr(json, pattern);

    if (start == NULL)
    {
        return;
    }

    // Finds the colon located after the key.
    start = strchr(
        start + strlen(pattern),
        ':'
    );

    if (start == NULL)
    {
        return;
    }

    start++;

    // Skips spaces between the colon and value.
    while (
        *start != '\0' &&
        isspace((unsigned char) *start)
    )
    {
        start++;
    }

    // The requested value must be a JSON string.
    if (*start != '"')
    {
        return;
    }

    start++;

    // Finds the closing quotation mark.
    end = strchr(start, '"');

    if (end == NULL)
    {
        return;
    }

    length = (int) (end - start);

    // Prevents the output array from overflowing.
    if (length >= output_size)
    {
        length = output_size - 1;
    }

    strncpy(
        output,
        start,
        length
    );

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


    // Opens the main Academia database.
    // The function returns -1 if the connection fails.
    if (sqlite3_open(
            DATABASE_PATH,
            &db
        ) != SQLITE_OK)
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


    // Searches for an account matching all three login values.
    const char *sql =
        "SELECT id "
        "FROM user "
        "WHERE username = ? "
        "AND password = ? "
        "AND role = ?;";


    // Prepares the login query for execution.
    // The login values are bound after preparation.
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


    // Inserts the username into the first question mark.
    sqlite3_bind_text(
        stmt,
        1,
        username,
        -1,
        SQLITE_TRANSIENT
    );

    // Inserts the password into the second question mark.
    sqlite3_bind_text(
        stmt,
        2,
        password,
        -1,
        SQLITE_TRANSIENT
    );

    // Inserts the role into the third question mark.
    sqlite3_bind_text(
        stmt,
        3,
        role,
        -1,
        SQLITE_TRANSIENT
    );


    // Reads the user ID when a matching account exists.
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        user_id = sqlite3_column_int(
            stmt,
            0
        );
    }


    // Releases the query and closes the database.
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return user_id;
}


/*
Sends a plain-text HTTP response to the browser.
The response includes the CORS headers required by Live Server.
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

    // Creates the complete HTTP response.
    snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code,
        status_text,
        body_length,
        body
    );

    // Sends the response to the browser.
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
void send_options_response(
    SOCKET client_socket
)
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
Processes a POST request sent to the /login endpoint.
Successful login creates the state file for the verified role.
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


    // Finds where the HTTP headers end and the JSON body begins.
    body = strstr(
        request,
        "\r\n\r\n"
    );

    // Rejects the request if an HTTP body was not found.
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


    // Reads the three login values from the JSON body.
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


    // Rejects the request when any required value is missing.
    if (
        username[0] == '\0' ||
        password[0] == '\0' ||
        role[0] == '\0'
    )
    {
        printf(
            "[Login Result] Missing login information.\n"
        );

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


    // Checks the login values against the SQLite database.
    user_id = authenticate_user(
        username,
        password,
        role
    );


    // Rejects invalid credentials before opening a portal.
    if (user_id == -1)
    {
        printf(
            "[Login Result] Invalid credentials.\n"
        );

        send_text_response(
            client_socket,
            401,
            "Unauthorized",
            "invalid"
        );

        return;
    }


    // Creates the Student Portal state before returning the role.
    // The browser opens the student page after receiving this response.
    if (strcmp(role, "student") == 0)
    {
        update_student_frontend_state(
            user_id
        );

        printf(
            "[Login Result] Student login successful. "
            "User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "student"
        );
    }

    // Creates the Parent Portal state before returning the role.
    // The browser opens the parent page after receiving this response.
    else if (strcmp(role, "parent") == 0)
    {
        update_parent_frontend_state(
            user_id
        );

        printf(
            "[Login Result] Parent login successful. "
            "User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "parent"
        );
    }

    // Creates the Staff Portal state before returning the role.
    // The browser opens the staff page after receiving this response.
    else if (strcmp(role, "staff") == 0)
    {
        update_staff_frontend_state(
            user_id
        );

        printf(
            "[Login Result] Staff login successful. "
            "User ID: %d\n",
            user_id
        );

        send_text_response(
            client_socket,
            200,
            "OK",
            "staff"
        );
    }

    // Rejects unknown role values.
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


    // Starts the Windows socket system.
    // The program stops if Winsock cannot be initialized.
    if (WSAStartup(
            MAKEWORD(2, 2),
            &wsa
        ) != 0)
    {
        printf(
            "[Error] WSAStartup failed: %d\n",
            WSAGetLastError()
        );

        return 1;
    }


    // Creates the TCP server socket.
    // The socket receives login requests from the browser.
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


    // Clears and configures the server address.
    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);


    // Connects the socket to port 3000.
    // A bind error may mean another server is using the port.
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


    // Begins listening for browser requests.
    // The server can queue up to five pending connections.
    if (listen(
            server_socket,
            5
        ) == SOCKET_ERROR)
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
    printf(
        "Login endpoint: http://localhost:%d/login\n",
        PORT
    );
    printf("Press Ctrl+C to stop the server.\n\n");


    // Accepts and processes browser requests continuously.
    // Each connection is closed after its response is sent.
    while (1)
    {
        SOCKET client_socket;
        char request[BUFFER_SIZE];

        int received;


        // Waits for the next browser connection.
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


        // Clears the request buffer before receiving data.
        memset(
            request,
            0,
            sizeof(request)
        );


        // Reads the HTTP request and leaves space for '\0'.
        received = recv(
            client_socket,
            request,
            sizeof(request) - 1,
            0
        );

        // Closes the connection if no request data was received.
        if (received <= 0)
        {
            closesocket(client_socket);
            continue;
        }

        request[received] = '\0';


        // Handles the browser's CORS preflight request.
        if (
            strstr(
                request,
                "OPTIONS /login"
            ) != NULL
        )
        {
            send_options_response(
                client_socket
            );
        }

        // Handles login information sent by the Auth page.
        else if (
            strstr(
                request,
                "POST /login"
            ) != NULL
        )
        {
            handle_login(
                client_socket,
                request
            );
        }

        // Rejects all routes that are not supported.
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


    // These lines run only if the server loop ends.
    closesocket(server_socket);
    WSACleanup();

    return 0;
}