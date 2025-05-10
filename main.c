#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcp_server.h"
#include "request.h"
#include "response.h"
#include "router.h"

#define DEFAULT_PORT 8080

int print_flag = 1;

void usage(char *program_name);

int main(int argc, char *argv[]) {
    int port_number = DEFAULT_PORT;

    // Parse command-line arguments
    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        if (!strcmp(argv[arg_index], "-p")) {
            if (arg_index + 1 >= argc || sscanf(argv[arg_index + 1], "%d", &port_number) != 1)
                usage(argv[0]);
            if (port_number < 1024 || port_number > 65535) {
                fprintf(stderr, "Port number must be between 1024 and 65535\n");
                usage(argv[0]);
            }
            arg_index++;
        } else if (!strcmp(argv[arg_index], "-v")) {
            print_flag = 1;
        } else {
            usage(argv[0]);
        }
    }

    // Create the listening socket
    int listening_socket = createSocket(port_number);
    if (listening_socket < 0) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }
    printf("Listening on port %d\n", port_number);

    // Use acceptConnections to handle incoming connections
    acceptConnections(listening_socket);

    // Close the listening socket (this will never be reached in the current implementation)
    close(listening_socket);
    return 0;
}

void usage(char *program_name) {
    fprintf(stderr,
            "Usage:\n"
            "  %s [-p port_number] [-v]\n"
            "  -p port_number: Specify the port number (1024-65535)\n"
            "  -v: Enable verbose output\n",
            program_name);
    exit(1);
}
