#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "request.h"

extern int print_flag;
void *handleConnectionThread(void *arg);

int createSocket(int port) {
    int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in socket_address;
    socket_address.sin_family      = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port        = htons(port);

    printf("Binding to port %d\n", port);

    int returnval;
    returnval = bind(listening_socket, (struct sockaddr *)&socket_address,
                     sizeof(socket_address));
    if (returnval == -1) {
        perror("bind");
        close(listening_socket);
        return -1;
    }
    returnval = listen(listening_socket, LISTEN_BACKLOG);
    return listening_socket;
}

void acceptConnections(int listening_socket) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    while (1) {
        pthread_t thread_id;
        int *client_fd_buf = malloc(sizeof(int));

        *client_fd_buf =
            accept(listening_socket, (struct sockaddr *)&client_address,
                   &client_address_len);
        if (*client_fd_buf < 0) {
            perror("accept");
            free(client_fd_buf);
            continue;
        }
        pthread_create(&thread_id, NULL,
                       (void *(*)(void *))handleConnectionThread,
                       client_fd_buf);
        pthread_detach(thread_id);
    }
}

int detectEOL(const char *buffer, int buffer_len, const char **eol) {
    for (int buf_in = 0; buf_in < buffer_len; buf_in++) {
        if (buffer[buf_in] == '\n') {
            if (buf_in + 1 < buffer_len && buffer[buf_in + 1] == '\r') {
                *eol = "\n\r";
                return 2;
            } else {
                *eol = "\n";
                return 1;
            }
        } else if (buffer[buf_in] == '\r') {
            if (buf_in + 1 < buffer_len && buffer[buf_in + 1] == '\n') {
                *eol = "\r\n";
                return 2;
            } else {
                *eol = "\r";
                return 1;
            }
        }
    }
    *eol = NULL;
    return 0;
}

char *findHeaderEnd(const char *buffer, int buffer_len, const char **eol,
                    int eol_len) {
    if (eol == NULL || buffer_len < eol_len * 2)
        return NULL;

    for (int buf_in = 0; buf_in < buffer_len - (eol_len * 2); buf_in++) {
        if (memcmp(buffer + buf_in, *eol, eol_len) == 0 &&
            memcmp(buffer + buf_in + eol_len, *eol, eol_len) == 0) {
            return (char *)&buffer[buf_in + eol_len * 2];
        }
    }
    return NULL;
}

void handleConnection(int a_client) {
    char read_buffer[BUFFER_SIZE];
    char write_buffer[LONGER_BUFFER_SIZE];
    char *end_of_line = NULL;
    int write_len     = 0;
    int eol_len       = 0;

    while (1) {
        int bytes_read = read(a_client, read_buffer, sizeof(read_buffer));
        if (bytes_read <= 0) {
            if (bytes_read < 0)
                perror("read");
            else
                printf("Client disconnected.\n");
            break;
        }

        if (print_flag)
            printf("Read from client: %.*s\n", bytes_read, read_buffer);

        if (write_len + bytes_read >= LONGER_BUFFER_SIZE) {
            fprintf(stderr, "Line too long, dropping connection.\n");
            break;
        }

        memcpy(write_buffer + write_len, read_buffer, bytes_read);
        write_len += bytes_read;

        // Determine what EOL is
        if (end_of_line == NULL)
            eol_len = detectEOL(write_buffer, write_len, &end_of_line);

        if (end_of_line != NULL) {
            char *header_end =
                findHeaderEnd(write_buffer, write_len, &end_of_line, eol_len);
            if (header_end != NULL) {
                int header_len = header_end - write_buffer;
                int tail_len   = write_len - header_len;
                if (print_flag)
                    printf("Header end found: %.*s\n",
                           header_end - write_buffer, write_buffer);

                // Create HTTP header then send response
                char *request_header = malloc(header_len + 1);
                memcpy(request_header, write_buffer, header_len);
                request_header[header_len] = '\0';
                int keep_alive =
                    handleRequest(request_header, a_client, end_of_line);
                free(request_header);

                // Shift remaining data to the beginning of the buffer
                write_len = tail_len;
                memmove(write_buffer, header_end, tail_len);
                memset(write_buffer + tail_len, 0,
                       LONGER_BUFFER_SIZE - tail_len);
                if (print_flag)
                    printf("Remaining buffer: %.*s\n", tail_len, write_buffer);

                if (!keep_alive) {
                    if (print_flag)
                        printf("Connection will be closed after response.\n");
                    break;
                } else {
                    if (print_flag)
                        printf("Keeping connection alive.\n");
                }
            }
        }
    }
    close(a_client);
}

void *handleConnectionThread(void *arg) {
    int client_fd = *((int *)arg);
    free(arg);
    handleConnection(client_fd);
    return NULL;
}
