#include "router.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "response.h"

#define UNUSED(x) (void*)(x)

Response* staticHandler(Request* req);
Response* calcHandler(Request* req);
Response* noResourceHandler(Request* req);
Response* badRequestHandler(Request* req);

static Route routes[] = {{"GET", "/static", true, staticHandler},
                         {"GET", "/calc", true, calcHandler},
                         {"GET", "", false, noResourceHandler},
                         {NULL, NULL, false, NULL}};

typedef struct {
    char* ext;
    char* filetype;
    bool is_binary;
} ContentType;

ContentType extension[] = {
    {"gif", "image/gif", true},    {"jpg", "image/jpg", true},
    {"jpeg", "image/jpeg", true},  {"png", "image/png", true},
    {"ico", "image/ico", true},    {"zip", "image/zip", true},
    {"gz", "image/gz", true},      {"tar", "image/tar", true},
    {"htm", "text/html", false},   {"html", "text/html", false},
    {"css", "text/css", false},    {"js", "application/javascript", false},
    {"md", "text/markdown", true}, {NULL, NULL, false}};

ContentType default_type = {"", "text/plain", false};

Response* routeRequest(Request* req) {
    for (Route* candidate = routes; candidate->method != NULL; candidate++) {
        if (strcmp(req->method, candidate->method) != 0)
            continue;
        if (candidate->full_match && strcmp(req->path, candidate->path) != 0)
            continue;
        if (!candidate->full_match &&
            strncmp(req->path, candidate->path, strlen(candidate->path)) != 0) {
            continue;
        }
        return (*candidate->handler)(req);
    }
    return noResourceHandler(req);
}

static void constructFilePath(const char* base_path, const char* req_path, char* file_path, size_t size) {
    snprintf(file_path, size, "%s%s", base_path, req_path + 7); // Skip "/static/"
}

static char* readFile(const char* file_path, int* file_size) {
    FILE* file = fopen(file_path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* body = malloc(*file_size);
    fread(body, 1, *file_size, file);
    fclose(file);

    return body;
}

ContentType* getContentType(const char* ext) {
    for (ContentType* type = extension; type->ext != NULL; type++) {
        if (strcmp(type->ext, ext) == 0) {
            return type;
        }
    }
    return &default_type;
}

Response* staticHandler(Request* req) {
    if (strncmp(req->path, "/static/", 8) != 0) {
        return badRequestHandler(req);
    }

    char file_path[512];
    constructFilePath("./static", req->path, file_path, sizeof(file_path));

    int file_size;
    char* body = readFile(file_path, &file_size);
    if (!body) {
        return noResourceHandler(req);
    }

    const char* ext = strrchr(file_path, '.');
    ContentType* content_type = getContentType(ext ? ext + 1 : "");

    Response* res = createResponse();
    res->status_code = 200;
    res->addHeader(res, "Content-Type", content_type->filetype);
    res->addHeader(res, "Content-Length", malloc(16));
    snprintf(vec_at(res->headers, vec_length(res->headers) - 1), 16, "%d", file_size);
    res->setBody(res, body);

    free(body);
    return res;
}

Response* calcHandler(Request* req) {
    if (strncmp(req->path, "/calc/", 6) != 0) {
        return badRequestHandler(req);
    }

    char operation[16];
    int num1, num2;
    if (sscanf(req->path, "/calc/%15[^/]/%d/%d", operation, &num1, &num2) != 3) {
        return badRequestHandler(req);
    }

    int result = 0;
    if (strcmp(operation, "add") == 0) {
        result = num1 + num2;
    } else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
    } else if (strcmp(operation, "div") == 0) {
        if (num2 == 0) {
            Response* res = createResponse();
            res->status_code = 400;
            res->setBody(res, "<html><body><h1>400 Division by Zero</h1></body></html>");
            return res;
        }
        result = num1 / num2;
    } else {
        return badRequestHandler(req);
    }

    char body[128];
    snprintf(body, sizeof(body), "<html><body><h1>Result: %d</h1></body></html>", result);

    Response* res = createResponse();
    res->status_code = 200;
    res->addHeader(res, "Content-Type", "text/html");
    res->setBody(res, body);

    return res;
}

Response* noResourceHandler(Request* req) {
    UNUSED(req);
    Response* res = createResponse();
    res->status_code = 404;
    res->setBody(res, "<html><body><h1>404 Not Found</h1></body></html>");
    return res;
}

Response* badRequestHandler(Request* req) {
    UNUSED(req);
    Response* res = createResponse();
    res->status_code = 400;
    res->setBody(res, "Bad Request");
    return res;
}
