#include "router.h"
#include "response.h"
#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "response.h"

int parseRequest(const char *request, char *eol, Request *req) {
    char *request_copy = strdup(request);
    if (!request_copy) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    req->method = strtok(request_copy, " ");
    req->path = strtok(NULL, " ");
    req->version = strtok(NULL, eol);

    if (!req->method || !req->path || !req->version) {
        fprintf(stderr, "Malformed HTTP request\n");
        free(request_copy);
        return 0;
    }

    printf("Method: %s\n", req->method);
    printf("Path: %s\n", req->path);
    printf("Version: %s\n", req->version);

    free(request_copy);
    return 1;
}

int shouldKeepAlive(const char *request, const char *version) {
    if (strcmp(version, "HTTP/1.1") != 0) {
        return 0;
    }

    char *connection_header = strstr(request, "Connection:");
    if (connection_header) {
        if (strstr(connection_header, "keep-alive")) {
            return 1;
        } else if (strstr(connection_header, "close")) {
            return 0;
        }
    }

    return 1;
}

int handleRequest(const char *request, int client_fd, char *eol) {
    Request req;
    Response *res = createResponse();
    int keep_alive = 1;

    // Parse the HTTP request
    if (!parseRequest(request, eol, &req)) {
        res->status_code = 400; // Bad Request
        res->setBody(res, "Invalid Request");
        res->sendToFd(res, client_fd);
        res->free(res);
        return 0;
    }

    // Determine if the connection should be kept alive
    keep_alive = shouldKeepAlive(request, req.version);

    // Route the request to the appropriate handler
    if (!routeRequest(&req)) {
        res->status_code = 404; // Not Found
        res->setBody(res, "Not Found");
    }

    // Send the response
    res->addHeader(res, "Connection", keep_alive ? "keep-alive" : "close");
    res->sendToFd(res, client_fd);
    res->free(res);

    return keep_alive;
}

void printRequest(Request *req) {
    printf("Request Method: %s\n", req->method);
    printf("Request Path: %s\n", req->path);
    printf("Request Version: %s\n", req->version);
    printf("Request Headers: %s\n", req->headers);
    printf("Request Body: %s\n", req->body);
}

void freeRequest(Request *req) {
    free(req->method);
    free(req->path);
    free(req->version);
    free(req->headers);
    free(req->body);
}