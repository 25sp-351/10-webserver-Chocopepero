#ifndef ROUTER_H
#define ROUTER_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "response.h"

typedef struct {
    char* method;
    char* path;
    bool full_match;
    Response* (*handler)(Request* req);
} Route;

Response* routeRequest(Request *req);

#endif  // ROUTER_H
