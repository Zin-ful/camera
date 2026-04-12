#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

int main() {
    int server_fd;
    struct sockaddr_in addr = {AF_INET, htons(8080), INADDR_ANY};
    int addrlen = sizeof(addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed to create");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Socket failed to bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Socket failed to listen");
        exit(EXIT_FAILURE);
    }
    int new_socket = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Socket failed to accept:");
        exit(EXIT_FAILURE);
    }
    char buffer[30000] = {0};
    read(new_socket, buffer, 30000);
    printf("Request Received\n\n%s\n\n###################\n", buffer);
    send(socket, header, strlen(header), 0);
    char html[BUFFER_SIZE] = {0};
    if (!get_html_file("index.html", html)) {
        send_404(socket);
    }
    send(socket, html, strlen(html), 0);
    
}

void send_html_header(int socket) {
    char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(socket, header, strlen(header), 0);
}

void send_simple_response(int socket, char *status, char *content_type, char *body) {
    printf("(send_simple_response) Sending response of type: %s\n", content_type);
    char response[BUFFER_SIZE];
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n"
        "%s", status, content_type, strlen(body), body);
    send(socket, response, len, 0);
}


void send_404(int socket) {
    printf("(send_404) Sending 404\n");
    char *body = "<html><body style='background-color:black;color:white;'>"
                 "<h1>404 - File Not Found</h1></body></html>";
    send_simple_response(socket, "404 Not Found", "text/html", body);

}

int get_html_file(char *path, char *newfile) {
    newfile = fopen(path, "r");
    if (!newfile) {
        printf("(get_html_file) File not found\n");
        return 0;
    }
    return 1;
}

