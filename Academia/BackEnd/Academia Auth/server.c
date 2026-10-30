/*
    Academia Login Server
    CS50 Project

    Compile:
    gcc server.c -lsqlite3 -o server

    Run:
    ./server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sqlite3.h>

#define PORT 8080
#define BUFFER_SIZE 8192

//----------------------------------------------------
// JSON PARSER
//----------------------------------------------------

void getJsonValue(const char *json,
                  const char *key,
                  char *output)
{
    char pattern[100];

    sprintf(pattern, "\"%s\":\"", key);

    char *start = strstr(json, pattern);

    if(start == NULL)
    {
        output[0] = '\0';
        return;
    }

    start += strlen(pattern);

    char *end = strchr(start, '"');

    if(end == NULL)
    {
        output[0] = '\0';
        return;
    }

    strncpy(output, start, end - start);
    output[end - start] = '\0';
}

//----------------------------------------------------
// LOGIN AUTHENTICATION
//----------------------------------------------------

const char *authenticateUser(
        const char *username,
        const char *password,
        const char *role)
{
    static char result[20];

    strcpy(result, "invalid");

    sqlite3 *db;
    sqlite3_stmt *stmt;

    if(sqlite3_open("academia.db", &db) != SQLITE_OK)
    {
        printf("Database connection failed\n");
        return result;
    }

    const char *sql =
        "SELECT role "
        "FROM user "
        "WHERE username=? AND password=?;";

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
        const char *dbRole =
            (const char *)
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

//----------------------------------------------------
// MAIN SERVER
//----------------------------------------------------

int main()
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    socklen_t addr_len =
        sizeof(server_addr);

    server_fd =
        socket(AF_INET,
               SOCK_STREAM,
               0);

    if(server_fd < 0)
    {
        perror("Socket Failed");
        return 1;
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(bind(server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("Bind Failed");
        return 1;
    }

    if(listen(server_fd, 10) < 0)
    {
        perror("Listen Failed");
        return 1;
    }

    printf("Academia Server Running\n");
    printf("http://localhost:%d\n\n", PORT);

    while(1)
    {
        client_fd =
            accept(server_fd,
                   (struct sockaddr *)&server_addr,
                   &addr_len);

        if(client_fd < 0)
        {
            continue;
        }

        char request[BUFFER_SIZE];

        memset(request, 0, BUFFER_SIZE);

        read(
            client_fd,
            request,
            BUFFER_SIZE);

        printf("Request:\n%s\n", request);

        if(strstr(request,
                  "POST /login") != NULL)
        {
            char username[100];
            char password[100];
            char role[100];

            char *body =
                strstr(request,
                       "\r\n\r\n");

            if(body != NULL)
            {
                body += 4;

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

                printf("Username: %s\n",
                       username);

                printf("Password: %s\n",
                       password);

                printf("Role: %s\n\n",
                       role);

                const char *result =
                    authenticateUser(
                        username,
                        password,
                        role);

                char response[512];

                sprintf(
                    response,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "\r\n"
                    "%s",
                    result);

                write(
                    client_fd,
                    response,
                    strlen(response));
            }
        }
        else
        {
            char response[] =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Not Found";

            write(
                client_fd,
                response,
                strlen(response));
        }

        close(client_fd);
    }

    close(server_fd);

    return 0;
}