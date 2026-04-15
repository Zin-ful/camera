#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "strops.h"

#define BUFFER_SIZE 4096

char *index_file = "<!DOCTYPE html>\n<html>\n<body>\n\n\n<h1>Security Server</h1>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n<p><a href='%s'>%s</a></p>\n\n\n</body>\n</html>";

struct CameraInfo {
    int active;
    int index;
    int socket;
    char addr[16];
    FILE *file;
};

struct CameraInfo Cameras[10];

int cameras_len() {
    int j = 0;
    for (int i = 0; i < 10; i++) {
        if (Cameras[i].socket) {
            printf("Camera found at %d", i);
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
    printf("Camera slot found at %d", i);
    return i;
}

void add_camera(int socket, char *request) {
    printf("Adding a camera\n");
    int index = find_camera_slot();
    char filename[10];
    char host[16];
    snprintf(filename, sizeof(filename), "camera_%d", index);
    FILE *file = fopen(filename, "wb");
    sscanf(request, "CAMERA_CONNECTION_REQUESTED\nHost: %15s", host);
    struct CameraInfo camera = {1, index, socket, "", file};
    strcpy(camera.addr, host);
    printf("Camera initalized");
    Cameras[index] = camera;
    printf("Camera assigned: Active: %d\nIndex: %d\nSocket: %d\nHost: %s\n", Cameras[index].active, Cameras[index].index, Cameras[index].socket, Cameras[index].addr);
}

void remove_camera(int index) {
    printf("Removing camera\n");
    Cameras[index].active = 0;
    Cameras[index].index = 0;
    Cameras[index].socket = 0;
    //Cameras[index].addr = {0};
    fclose(Cameras[index].file);
    
}

int find_active_camera(int socket) {
    for (int i = 0; i < 10; i++) {
        if (Cameras[i].socket == socket) {
            return Cameras[i].index;
        }
    }
    return 0;
}


void set_video(int socket) {
    printf("Creating new index.html page for video\n");
    char formatted_html[256];
    
    //snprintf(formatted_html, sizeof(formatted_html), index_file, );
    FILE *file = fopen("index.html", "w");
    
    fwrite(formatted_html, sizeof(char), strlen(formatted_html), file);
    char buffer[4096] = {0};
    int camera = find_active_camera(socket);
    while (read(socket, buffer, 4096)) {
        fwrite(buffer, sizeof(char), strlen(buffer), Cameras[camera].file);
    }
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

void get_request(char *request, int socket) {
	if (strstr(request, "CAMERA_CONNECTION_REQUESTED")) {
        add_camera(socket, request);
        set_video(socket);
    }

    if (strstr(request, "/camera")) {
		send_html_header(socket);
	    if (send_html_file("camera.html", socket) < 0) {
	    	printf("Failed to send HTML file\n");
	    } else {
	    	printf("Sent HTML file\n");
    	}
    	printf("Getting camera\n\n");
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
}

int main() {

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
