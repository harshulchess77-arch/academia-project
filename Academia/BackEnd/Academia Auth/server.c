#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "SQLite3/sqlite3.h"

#pragma comment(lib, "Ws2_32.lib")

int main(void)
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    printf("Server running on port 8080\n");

    while (1) {
        SOCKET client = accept(server, NULL, NULL);

        char request[4096] = {0};
        recv(client, request, sizeof(request) - 1, 0);

        const char *body = "student";
        char response[512];

        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n%s",
            strlen(body), body);

        send(client, response, (int)strlen(response), 0);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}