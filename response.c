#include "response.h"

void addHeader(Response *rsp, char *key, char *value) {
    char header[256];
    snprintf(header, sizeof(header), "%s: %s", key, value);
    vec_add(rsp->headers, strdup(header)); 
}

void printResponse(Response *rsp) {
    printf("HTTP/%s %d\n", rsp->version, rsp->status_code);
    for (size_t i = 0; i < vec_length(rsp->headers); i++) 
        printf("%s\n", (char *)vec_at(rsp->headers, i)); 
    printf("\n%s\n", rsp->body ? rsp->body : "");
}

void freeResponse(Response *rsp) {
    if (rsp) {
        free(rsp->version);
        for (size_t vi = 0; vi < vec_length(rsp->headers); vi++) 
            free(vec_at(rsp->headers, vi)); 
        vec_free(rsp->headers); 
        free(rsp->body);
        free(rsp);
    }
}

void setBody(Response *rsp, char *body) {
    rsp->body = strdup(body);
    rsp->body_len = strlen(body);
}

bool sendToFd(Response *rsp, int fd) {
    char buffer[1024];
    int offset = snprintf(buffer, sizeof(buffer), "HTTP/%s %d OK\r\n",
                          rsp->version, rsp->status_code);

    for (size_t i = 0; i < vec_length(rsp->headers); i++) { 
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s\r\n",
                           (char *)vec_at(rsp->headers, i)); 
    }

    snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    write(fd, buffer, strlen(buffer));

    if (rsp->body)
        write(fd, rsp->body, rsp->body_len);

    return true;
}

Response *createResponse(void) {
    Response *rsp = malloc(sizeof(Response));
    if (!rsp) {
        perror("Failed to allocate memory for Response");
        return NULL;
    }

    rsp->status_code = 200;
    rsp->version = strdup("1.1");
    rsp->headers = new_vec(sizeof(char *)); 
    rsp->body = NULL;
    rsp->body_len = 0;

    rsp->addHeader = addHeader;
    rsp->print = printResponse;
    rsp->free = freeResponse;
    rsp->sendToFd = sendToFd;
    rsp->setBody = setBody;

    return rsp;
}
