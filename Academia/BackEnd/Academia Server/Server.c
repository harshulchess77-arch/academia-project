/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
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

