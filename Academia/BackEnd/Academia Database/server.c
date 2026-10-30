#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <sqlite3.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 8192

void getJsonValue(const char* json,
                  const char* key,
                  char* output)
{
    char pattern[100];

    sprintf(pattern, "\"%s\":\"", key);

    char* start = strstr(json, pattern);

    if(start == NULL)
    {
        output[0] = '\0';
        return;
    }

    start += strlen(pattern);

    char* end = strchr(start, '"');

    if(end == NULL)
    {
        output[0] = '\0';
        return;
    }

    strncpy(output, start, end - start);
    output[end - start] = '\0';
}

const char* authenticateUser(
    const char* username,
    const char* password,
    const char* role)
{
    static char result[20];

    strcpy(result, "invalid");

    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        return result;
    }

    const char* sql =
        "SELECT role "
        "FROM user "
        "WHERE username=? "
        "AND password=?;";

    if(sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL) != SQLITE_OK)
    {
        sqlite3_close(db);
        return result;
    }

    sqlite3_bind_text(
        stmt,
        1,
        username,
        -1,
        SQLITE_STATIC);

    sqlite3_bind_text(
        stmt,
        2,
        password,
        -1,
        SQLITE_STATIC);

    if(sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* dbRole =
            (const char*)
            sqlite3_column_text(stmt, 0);

        if(dbRole != NULL &&
           strcmp(dbRole, role) == 0)
        {
            if(strcmp(role, "Student") == 0)
            {
                strcpy(result, "student");
            }
            else if(strcmp(role, "Parent") == 0)
            {
                strcpy(result, "parent");
            }
            else if(strcmp(role, "Staff") == 0)
            {
                strcpy(result, "staff");
            }
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result;
}

int main()
{
    WSADATA wsa;

    if(WSAStartup(
            MAKEWORD(2,2),
            &wsa) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET serverSocket;

    serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0);

    if(serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if(bind(
            serverSocket,
            (struct sockaddr*)&server,
            sizeof(server))
            == SOCKET_ERROR)
    {
        printf("Bind failed\n");
        return 1;
    }

    listen(serverSocket, 5);

    printf("Academia Server Running\n");
    printf("http://localhost:8080\n\n");

    while(1)
    {
        SOCKET clientSocket;

        clientSocket =
            accept(
                serverSocket,
                NULL,
                NULL);

        char request[BUFFER_SIZE];

        memset(
            request,
            0,
            sizeof(request));

        recv(
            clientSocket,
            request,
            sizeof(request),
            0);

        if(strstr(request,
                  "POST /login"))
        {
            char* body =
                strstr(
                    request,
                    "\r\n\r\n");

            if(body != NULL)
            {
                body += 4;

                char username[100];
                char password[100];
                char role[100];

                getJsonValue(
                    body,
                    "username",
                    username);

                getJsonValue(
                    body,
                    "password",
                    password);

                getJsonValue(
                    body,
                    "role",
                    role);

                printf(
                    "LOGIN: %s %s %s\n",
                    username,
                    password,
                    role);

                const char* result =
                    authenticateUser(
                        username,
                        password,
                        role);

                char response[512];

                sprintf(
                    response,
                    "HTTP/1.1 200 OK\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "%s",
                    result);

                send(
                    clientSocket,
                    response,
                    strlen(response),
                    0);
            }
        }
        else
        {
            char response[] =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type:text/plain\r\n\r\n"
                "Not Found";

            send(
                clientSocket,
                response,
                strlen(response),
                0);
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}