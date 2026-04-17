#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "strops.h"
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 4096
#define FILE_BUFFER_SIZE 4096
int safe_mode = 1;

char *reset_index_file = "<!DOCTYPE html>\n<html>\n<body>\n\n\n<h1>Security Server</h1>\n\n\n</body>\n</html>";

char *index_file = "<!DOCTYPE html>\n<html>\n<body>\n\n\n<h1>Security Server</h1>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n<p><a href='camera_%d'>%s %d</a></p>\n\n\n</body>\n</html>";

struct CameraInfo {
    int active;
    int index;
    int socket;
    char name[32];
    char addr[16];
    FILE *file;
};

struct CameraInfo Cameras[10];

int cameras_len() {
    int j = 0;
    for (int i = 0; i < 10; i++) {
        if (Cameras[i].active) {
            printf("Camera found at %d\n", i);
            j++;
        }
    }
    return j;
}

int find_camera_slot() {
    int i = 0;    
    for (i = 0; Cameras[i].active != 0; i++) {
        printf("Searching...\n");
    }
    printf("Camera slot found at %d\n", i);
    return i;
}

int find_active_camera(int socket) {
    for (int i = 0; i < 10; i++) {
        if (Cameras[i].socket == socket) {
            return Cameras[i].index;
        }
    }
    return 0;
}

void init_cameras() {
    printf("Init cameras to 0\n");
    for (int i = 0; i < 9; i++) {
        Cameras[i].active = 0;
        Cameras[i].index = 0;
        strcpy(Cameras[i].name, "");
    }
}

void generate_html_page() {
    char formatted_html[1024];
    snprintf(formatted_html, sizeof(formatted_html), index_file,
    Cameras[0].index, Cameras[0].name, Cameras[0].index,
    Cameras[1].index, Cameras[1].name, Cameras[1].index,
    Cameras[2].index, Cameras[2].name, Cameras[2].index,
    Cameras[3].index, Cameras[3].name, Cameras[3].index,
    Cameras[4].index, Cameras[4].name, Cameras[4].index,
    Cameras[5].index, Cameras[5].name, Cameras[5].index,
    Cameras[6].index, Cameras[6].name, Cameras[6].index,
    Cameras[7].index, Cameras[7].name, Cameras[7].index,
    Cameras[8].index, Cameras[8].name, Cameras[8].index,
    Cameras[9].index, Cameras[9].name, Cameras[9].index);
    FILE *file = fopen("index.html", "w");
    fwrite(formatted_html, sizeof(char), strlen(formatted_html), file);
    fclose(file);
}

void add_camera(int socket, char *request) {
    printf("Adding a camera\n");

    int index = find_camera_slot();
    char host[16];
    char filename[16];
    snprintf(filename, sizeof(filename), "camera_%d.ts", index);
    FILE *file = fopen(filename, "wb");

    sscanf(request, "CAMERA_CONNECTION_REQUESTED\nHost: %15s", host);    
    struct CameraInfo camera = {1, index, socket, "", "Camera", file};
    strcpy(camera.addr, host);

    Cameras[index] = camera;

    printf("Camera initalized\n");
    printf("Camera assigned: Active: %d\nIndex: %d\nSocket: %d\nHost: %s\n", Cameras[index].active, Cameras[index].index, Cameras[index].socket, Cameras[index].addr);
    generate_html_page();
}

void remove_camera(int index) {
    printf("Removing camera\n");
    Cameras[index].active = 0;
    Cameras[index].index = 0;
    Cameras[index].socket = 0;
    strcpy(Cameras[index].addr, "");
    fclose(Cameras[index].file);
    
}

void set_video(int socket) {
    int camera = find_active_camera(socket);
    char buffer[4096] = {0};
    int bytes;

    printf("Starting video capture\n");
    
}

void reset_html_page() {
    FILE *file = fopen("index.html", "w");
    fwrite(reset_index_file, 1, strlen(reset_index_file), file);
    printf("Resetting HTML page.\n");
    fclose(file);
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

int send_html_file(char *path, int socket) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("(get_html_file) File not found\n");
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    send_html_header(socket);
    char filedata[size];
    while(fgets(filedata, sizeof(filedata), file)) {
        send(socket, filedata, strlen(filedata), 0);
    }
    fclose(file);
    close(socket);
    return 1;
}

int get_html_file(char *path, char *filedata) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("(get_html_file) File not found\n");
        return 0;
    }
    fgets(filedata, sizeof(filedata), file);
    fclose(file);
    return 1;
}

void start_stream(char *request, int socket) {
    char *header = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: video/MP2T\r\n"
    "Transfer-Encoding: chunked\r\n\r\n";

    printf("Sending stream header\n");    
    if (send(socket, header, strlen(header), 0) < 0) {
        perror("Error sending stream header:");
        exit(EXIT_FAILURE);
    }
    char camera[10];
    int i;
    printf("Looking for camera...\n");
    for (i = 0; i < 10; i++) {
        snprintf(camera, sizeof(camera), "camera_%d", i);
        if (strstr(request, camera)) {
            printf("Camera stream file found!\n");
            break;   
        }
    }
    send(Cameras[i].socket, "s", 1, 0);
    if (i == 10 && safe_mode) {
        printf("Camera stream file not found - ending program to prevent file errors. (turn safemode off to disable)\n");
        exit(EXIT_FAILURE);
    }
    char filepath[16];
    snprintf(filepath, sizeof(filepath), "%s%s", camera, ".ts");
    
    int bytes_read;
    int file_descriptor = open(filepath, O_RDONLY | O_NONBLOCK);
    if (file_descriptor < 0) {
        perror("Error opening camera stream file:");
    }
    FILE *file = fdopen(file_descriptor, "rb");
    if (!file) {
        perror("Error opening and reading camera file:");
        exit(EXIT_FAILURE);
    }
    char chunk_header[128];
    char buffer[FILE_BUFFER_SIZE] = {0};
    int pos = 0;
    printf("File opened, init done: %s\n", filepath);
    while (1) {
        /*fseek(file, 0, SEEK_END);
        printf("Seeking\n");
        long end = ftell(file);
        long avaliable = end - pos;
        if (avaliable < FILE_BUFFER_SIZE) {
            printf("File not ready, waiting\n");
            usleep(10000);
            continue;
        }
        fseek(file, pos, SEEK_SET);
        printf("Starting read\n");
        bytes_read = fread(buffer, 1, FILE_BUFFER_SIZE, file);
        if (bytes_read <= 0) continue;
        pos += bytes_read;
        printf("Reading: %d bytes | position: %d | avaliable: %ld | end: %ld\n", bytes_read, pos, avaliable, end);*/
        
        bytes_read = read(socket, buffer, 4096);
        snprintf(chunk_header, sizeof(chunk_header), "%x\r\n", bytes_read);
        if (send(socket, chunk_header, strlen(chunk_header), 0) < 0) break;
        if (send(socket, buffer, bytes_read, 0) < 0) break;
        if (send(socket, "\r\n", 2, 0) < 0) break;
    }
    printf("bytes_read value: %d", bytes_read);
}

void get_request(char *request, int socket) {
	if (strstr(request, "CAMERA_CONNECTION_REQUESTED")) {
        add_camera(socket, request);
        set_video(socket);
        remove_camera(find_active_camera(socket));
        generate_html_page();
    } else if (strstr(request, "/camera")) {
        printf("Starting stream to browser\n");
		start_stream(request, socket);
	} else {
		send_html_header(socket);
	    if (send_html_file("index.html", socket) < 0) {
	    	printf("Failed to send HTML file\n");
	    } else {
	    	printf("Sent HTML file\n");
    	}
    	printf("Sending main page\n\n");
	}
}

void *client(void *new_socket) {
	int socket = *(int *)new_socket;
	free(new_socket);
    char buffer[30000] = {0};
    read(socket, buffer, 30000);
    
    printf("Request Received\n\n%s\n\n###################\n", buffer);    
	get_request(buffer, socket);
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN);
	int server_fd, new_socket;
   	pthread_t client_thread;
   	pthread_mutex_t mutex;
    
   	int opt = 1;
   	printf("serverfd, client_thread, new_socket, and mutex created\n");
   	
   	struct sockaddr_in addr = {AF_INET, htons(8080), INADDR_ANY};
   	int addrlen = sizeof(addr);
    	
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
       	perror("Socket failed to create");
       	exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Socket failed to set options:");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("socket created\n");
   	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
       	perror("Socket failed to bind");
       	exit(EXIT_FAILURE);
   	}
   	printf("socket bound to 8080 0.0.0.0\n");
   	if (pthread_mutex_init(&mutex, NULL) < 0) {
   		perror("Mutex failed to init: ");
   	}
    init_cameras();
    reset_html_page();
	while (1) {
    	if (listen(server_fd, 3) < 0) {
        	perror("Socket failed to listen");
        	exit(EXIT_FAILURE);
    	}
    	printf("listening for connections...\n");
    	
    	new_socket = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
    	int *pclient = malloc(sizeof(int)); //to prevent sharing var, malloc
		*pclient = new_socket;
		
		printf("client connected\n");
    	if (new_socket < 0) {
        	perror("Socket failed to accept:");
        	exit(EXIT_FAILURE);
    	}

    	pthread_create(&client_thread, NULL, &client, pclient);
        printf("thread created\n");
    }
}
