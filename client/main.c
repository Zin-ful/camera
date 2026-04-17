#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 4096



void send_video(int socket_fd) {
    FILE *camout;
    char start[1];
    printf("Waiting on server..");
    
    //Idk what im doing right now
    //read(socket_fd, start, 1);
    
    unsigned char buffer[BUFFER_SIZE];
    camout = popen("rpicam-vid -t 0 --codec h264 --inline --mode 1280:720 --framerate 25 "
               "--shutter 30000 --gain 2 --flicker 60 --awb indoor "
               "--brightness 0.2 -o - 2>/dev/null", "r");
    /* ALTERNATIVE

    camout = popen(
    "rpicam-vid -t 0 --codec h264 --inline --mode 1280:720 --framerate 25 "
    "--shutter 30000 --gain 2 --flicker 60 --awb indoor "
    "--brightness 0.2 -o - 2>/dev/null | "
    "ffmpeg -fflags nobuffer -f h264 -i pipe:0 "
    "-c:v copy -f mpegts pipe:1 2>/dev/null",
    "r");
    */
    if (!camout) {
        perror("Failed to open Pi-Camera:");
    }
    setvbuf(camout, NULL, _IONBF, 0); //disable buffering
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, camout)) > 0) {
        if (send(socket_fd, buffer, bytes_read, MSG_NOSIGNAL) <= 0) {
            break;
        }
    }
    
    printf("Camera stopped\n");
    
}

int main(int argc, char* argv[]) {
    int socket_fd;
    struct sockaddr_in server_addr = {AF_INET, htons(8080)};

    if (argc > 2) {
        printf("Youve entered too many arguments. Enter the IP Address youd like to connect to\n");
        exit(EXIT_FAILURE);
    } else if (argc < 2) {
        printf("Youve entered too few arguments. Enter the IP Address youd like to connect to\n");
        exit(EXIT_FAILURE);
    }
    char *IP = argv[1];
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed to create:");
        exit(EXIT_FAILURE);
    }
    
    printf("socket created\n");
    printf("Attempting to connect to %s\n", IP);
    
    if (inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        exit(EXIT_FAILURE);
    }
    
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        perror("Failed to connect:");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    getsockname(socket_fd, (struct sockaddr*)&local, &len);
    printf("Local IP: %s\n", inet_ntoa(local.sin_addr));

    char hello[128] = {0};
    snprintf(hello, sizeof(hello), "CAMERA_CONNECTION_REQUESTED\nHost: %s\nAccept-Encoding: gzip, zstd", inet_ntoa(local.sin_addr));
    send(socket_fd, hello, strlen(hello), 0);
    send_video(socket_fd);
}
