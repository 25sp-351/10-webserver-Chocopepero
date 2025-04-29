#ifndef REQUEST_H
#define REQUEST_H

typedef struct hreq {
    char *method;
    char *path;
    char *version;
    char *headers;
    char *body;
    int body_len;

    void (*print) (struct hreq *req);
    void (*free) (struct hreq *req);
} Request;

int handleRequest(const char *request, int client_fd, char *eol);


#endif  // REQUEST_H