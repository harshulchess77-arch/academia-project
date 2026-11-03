/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
Server Setup
November 1, 2026
*/

// Main Method
// 
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <sqlite3.h>

#include "../Academia Database/Database.h"
#include "../Academia Auth/Academia_Auth.h"

#define PORT 3000
#define BUFFER_SIZE 16000

// Current database connection.
sqlite3 *database = NULL;

// Send an HTTP response.
void send_response(
    SOCKET client,
    int status,
    const char *status_text,
    const char *content_type,
    const char *body,
    const char *extra_headers
)
{
    char response[20000];

    int length = (int)strlen(body);

    snprintf(
        response,
        sizeof(response),

        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: http://localhost:3000\r\n"
        "Access-Control-Allow-Credentials: true\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Connection: close\r\n"
        "%s"
        "\r\n"
        "%s",

        status,
        status_text,
        content_type,
        length,
        extra_headers != NULL ? extra_headers : "",
        body
    );

    send(client, response, (int)strlen(response), 0);
}

// Get a value from a simple JSON request body.
// This is intended for basic fields such as username and password.
void get_json_value(
    const char *body,
    const char *key,
    char *value,
    size_t value_size
)
{
    char search[100];

    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *position = strstr(body, search);

    value[0] = '\0';

    if (position == NULL)
    {
        return;
    }

    position = strchr(position, ':');

    if (position == NULL)
    {
        return;
    }

    position++;

    while (*position == ' ' || *position == '\t')
    {
        position++;
    }

    if (*position != '"')
    {
        return;
    }

    position++;

    size_t i = 0;

    while (
        *position != '\0' &&
        *position != '"' &&
        i < value_size - 1
    )
    {
        value[i] = *position;

        i++;
        position++;
    }

    value[i] = '\0';
}

// Read the session token from a Cookie header.
void get_cookie(
    const char *request,
    char *token,
    size_t token_size
)
{
    token[0] = '\0';

    const char *cookie = strstr(request, "Cookie:");

    if (cookie == NULL)
    {
        return;
    }

    const char *position = strstr(cookie, "academia_session=");

    if (position == NULL)
    {
        return;
    }

    position += strlen("academia_session=");

    size_t i = 0;

    while (
        *position != '\0' &&
        *position != ';' &&
        *position != '\r' &&
        *position != '\n' &&
        i < token_size - 1
    )
    {
        token[i] = *position;

        i++;
        position++;
    }

    token[i] = '\0';
}

// Send a simple JSON error.
void send_error(SOCKET client, const char *message)
{
    char body[500];

    snprintf(
        body,
        sizeof(body),
        "{\"success\":false,\"message\":\"%s\"}",
        message
    );

    send_response(
        client,
        400,
        "Bad Request",
        "application/json",
        body,
        NULL
    );
}

// Handle an HTTP request.
void handle_request(SOCKET client, char *request)
{
    char method[20] = "";
    char path[300] = "";

    sscanf(request, "%19s %299s", method, path);

    // Find request body.
    char *body = strstr(request, "\r\n\r\n");

    if (body != NULL)
    {
        body += 4;
    }
    else
    {
        body = "";
    }

    // Read cookie.
    char token[100];

    get_cookie(request, token, sizeof(token));

    // Browser preflight request.
    if (strcmp(method, "OPTIONS") == 0)
    {
        send_response(
            client,
            204,
            "No Content",
            "text/plain",
            "",
            NULL
        );

        return;
    }

    // Home route.
    if (
        strcmp(method, "GET") == 0 &&
        strcmp(path, "/") == 0
    )
    {
        const char *html =
            "<!DOCTYPE html>"
            "<html><head><title>Academia Server</title></head>"
            "<body>"
            "<h1>Academia Server is Running</h1>"
            "<p>HTTP server connected on port 3000.</p>"
            "<p>Visit /api/health to check the API.</p>"
            "</body></html>";

        send_response(
            client,
            200,
            "OK",
            "text/html",
            html,
            NULL
        );

        return;
    }

    // Health check.
    if (
        strcmp(method, "GET") == 0 &&
        strcmp(path, "/api/health") == 0
    )
    {
        const char *json =
            "{\"success\":true,"
            "\"message\":\"Academia server is running.\"}";

        send_response(
            client,
            200,
            "OK",
            "application/json",
            json,
            NULL
        );

        return;
    }

    // LOGIN.
    if (
        strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/auth/login") == 0
    )
    {
        char username[100];
        char password[100];

        char response[1000];

        char new_token[100];

        char role[30];
        char name[100];

        int user_id = 0;

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

        if (username[0] == '\0' || password[0] == '\0')
        {
            send_error(client, "Enter your username and password.");
            return;
        }

        int success = auth_login(
            database,
            username,
            password,
            response,
            sizeof(response),
            new_token,
            sizeof(new_token),
            &user_id,
            role,
            sizeof(role),
            name,
            sizeof(name)
        );

        if (!success)
        {
            send_response(
                client,
                401,
                "Unauthorized",
                "application/json",
                response,
                NULL
            );

            return;
        }

        char headers[300];

        snprintf(
            headers,
            sizeof(headers),
            "Set-Cookie: academia_session=%s; HttpOnly; SameSite=Strict; Path=/\r\n",
            new_token
        );

        send_response(
            client,
            200,
            "OK",
            "application/json",
            response,
            headers
        );

        return;
    }

    // SESSION CHECK.
    if (
        strcmp(method, "GET") == 0 &&
        strcmp(path, "/api/auth/session") == 0
    )
    {
        int user_id = 0;

        char role[30];
        char name[100];

        if (
            auth_check(
                token,
                &user_id,
                role,
                sizeof(role),
                name,
                sizeof(name)
            )
        )
        {
            char response[500];

            snprintf(
                response,
                sizeof(response),
                "{\"success\":true,\"user\":{\"id\":%d,"
                "\"name\":\"%s\",\"role\":\"%s\"}}",
                user_id,
                name,
                role
            );

            send_response(
                client,
                200,
                "OK",
                "application/json",
                response,
                NULL
            );
        }
        else
        {
            send_response(
                client,
                401,
                "Unauthorized",
                "application/json",
                "{\"success\":false,\"message\":\"Not logged in.\"}",
                NULL
            );
        }

        return;
    }

    // LOGOUT.
    if (
        strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/auth/logout") == 0
    )
    {
        auth_logout(token);

        send_response(
            client,
            200,
            "OK",
            "application/json",
            "{\"success\":true,\"message\":\"Logged out.\"}",
            "Set-Cookie: academia_session=; Max-Age=0; Path=/; HttpOnly\r\n"
        );

        return;
    }

    // Unknown API route.
    send_response(
        client,
        404,
        "Not Found",
        "application/json",
        "{\"success\":false,\"message\":\"Route not found.\"}",
        NULL
    );
}

// Main server function.
int main(void)
{
    WSADATA data;

    SOCKET server_socket;
    SOCKET client_socket;

    struct sockaddr_in server_address;

    // Start Windows networking.
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        printf("Winsock could not start.\n");

        return 1;
    }

    // Connect to SQLite.
    database = open_database();

    if (database == NULL)
    {
        printf("Could not connect to Academia database.\n");

        WSACleanup();

        return 1;
    }

    // Create tables if they do not exist.
    if (!create_tables(database))
    {
        close_database(database);

        WSACleanup();

        return 1;
    }

    // Create the server socket.
    server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_socket == INVALID_SOCKET)
    {
        printf("Could not create server socket.\n");

        close_database(database);
        WSACleanup();

        return 1;
    }

    // Server address.
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind server to port 3000.
    if (
        bind(
            server_socket,
            (struct sockaddr *)&server_address,
            sizeof(server_address)
        ) == SOCKET_ERROR
    )
    {
        printf("Could not bind port %d.\n", PORT);

        closesocket(server_socket);
        close_database(database);
        WSACleanup();

        return 1;
    }

    // Start listening.
    if (listen(server_socket, 10) == SOCKET_ERROR)
    {
        printf("Server could not start listening.\n");

        closesocket(server_socket);
        close_database(database);
        WSACleanup();

        return 1;
    }

    printf("\n");
    printf("       ACADEMIA SERVER\n");
    printf("Server: http://localhost:%d\n", PORT);
    printf("API:    http://localhost:%d/api/health\n", PORT);
    printf("Press Ctrl + C to stop.\n");

    // Accept incoming requests.
    while (1)
    {
        client_socket = accept(
            server_socket,
            NULL,
            NULL
        );

        if (client_socket == INVALID_SOCKET)
        {
            printf("Could not accept connection.\n");

            continue;
        }

        char request[BUFFER_SIZE];

        int received = recv(
            client_socket,
            request,
            sizeof(request) - 1,
            0
        );

        if (received > 0)
        {
            request[received] = '\0';

            handle_request(client_socket, request);
        }

        closesocket(client_socket);
    }

    // These lines run if the server loop is later changed to stop.
    closesocket(server_socket);

    close_database(database);

    WSACleanup();

    return 0;
}
