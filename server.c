// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void send_response(int client_fd, char *filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        char *not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "404 File Not Found";
        write(client_fd, not_found, strlen(not_found));
        return;
    }

    char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n";

    write(client_fd, header, strlen(header));

    char buffer[BUFFER_SIZE];
    int bytes;

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        write(client_fd, buffer, bytes);
    }

    fclose(file);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Mini HTTP Server running on http://localhost:%d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&server_addr, &addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE);

        printf("Request:\n%s\n", buffer);

        // parse GET request
        char method[16], path[256];
        sscanf(buffer, "%s %s", method, path);

        if (strcmp(path, "/") == 0) {
            strcpy(path, "/index.html");
        }

        // remove leading '/'
        char *filename = path + 1;

        send_response(client_fd, filename);

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
