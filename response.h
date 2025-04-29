#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "vec.h"
typedef struct rsp{
    int status_code;
    char *version;
    Vec headers;
    char *body;
    int body_len;

    void (*addHeader)(struct rsp* rsp, char* key, char* value);
    void (*print) (struct rsp* rsp);
    void (*free) (struct rsp* rsp);
    bool (*sendToFd)(struct rsp* rsp, int fd);
    void (*setBody)(struct rsp* rsp, char* body);
} Response;

Response* createResponse(void);

#endif  // RESPONSE_H
