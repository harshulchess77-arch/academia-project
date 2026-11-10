/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia - Local Web Server
*/
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sqlite3.h>

#include "Database.h"
#include "Academia Auth/Academia_Auth.h"
#include "Academia Student/Academia_Student.h"
#include "Academia Staff/Academia_Staff.h"
#include "Academia Parent/Academia_Parents.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winsqlite3.lib")

#define PORT 3000
#define BUFFER_SIZE 65536

static sqlite3 *db = NULL;

void send_response(SOCKET client, int status, const char *status_text, const char *content_type, const char *body) {
    char header[2048];
    int length = body ? (int)strlen(body) : 0;
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n",
             status, status_text, content_type, length);
    send(client, header, (int)strlen(header), 0);
    if (length > 0) send(client, body, length, 0);
}

void send_file(SOCKET client, const char *filepath, const char *content_type) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        send_response(client, 404, "Not Found", "text/html", "<h1>404 - File Not Found</h1>");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[1024];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n", content_type, size);
    send(client, header, (int)strlen(header), 0);

    char chunk[8192];
    size_t read;
    while ((read = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        send(client, chunk, (int)read, 0);
    }
    fclose(fp);
}

void handle_client_request(SOCKET client) {
    char buffer[BUFFER_SIZE];
    int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) return;
    buffer[received] = '\0';

    char method[16], path[512];
    sscanf(buffer, "%15s %511s", method, path);

    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        send_file(client, "FrontEnd/HTML/Academia_Landing.html", "text/html; charset=utf-8");
        return;
    }

    char target[512];
    snprintf(target, sizeof(target), "FrontEnd%s", path);
    FILE *fp = fopen(target, "rb");
    if (fp) {
        fclose(fp);
        const char *type = "text/html";
        if (strstr(target, ".css")) type = "text/css";
        else if (strstr(target, ".js")) type = "application/javascript";
        else if (strstr(target, ".png")) type = "image/png";
        else if (strstr(target, ".mp4")) type = "video/mp4";
        send_file(client, target, type);
    } else {
        send_response(client, 404, "Not Found", "text/html", "<h2>404 Not Found</h2>");
    }
}

int main(void) {
    printf("====================================================\n");
    printf("     ACADEMIA SERVER STARTING ON PORT %d\n", PORT);
    printf("====================================================\n");

    db = open_database();
    if (!db) return 1;

    update_student_frontend_state(1);
    update_staff_frontend_state(4);
    update_parent_frontend_state(3);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, SOMAXCONN);

    printf("Server running at: http://localhost:%d\n", PORT);

    while (1) {
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        handle_client_request(client);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    close_database(db);
    return 0;
}