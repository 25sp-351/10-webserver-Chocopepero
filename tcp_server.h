#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 1024
#define LONGER_BUFFER_SIZE 2048
#define MAX_HEADER_SIZE 8192
#define KEEPALIVE_LIMIT 100

int createSocket(int port);
void acceptConnections(int listening_socket);
void handleConnection(int a_client);
int detectEOL(const char *buffer, int buffer_len, const char **eol);
char *findHeaderEnd(const char *buffer, int buffer_len, const char **eol,
                    int eol_len);

#endif  // TCP_SERVER_H
