#include <stdio.h>
#include <string.h>
#include "sdp2xxx.h"

void print_help(void)
{
        printf("sdptool [-s <serial port>] CMD\n");
        printf("        CMD := { ... }\n");
}

int main(int argc, char **argv)
{
        const char *port = NULL;
        int fd_in, fd_out;
        int arg_idx = 1;

        if (argc < 2) {
                print_help();
                return -1;
        }

        if (!strcmp(argv[1], "-s")) {
                if (argc < 4) {
                        print_help();
                        return -1;
                }
                port = argv[2];
                arg_idx = 3;
        }

        if (port != NULL) {
                // TODO:
                // open
                // set parameters if serial port
                // get fd for io device (port or stdio)
        } 
        else {
                fd_in = 1;
                fd_out = 2;
        }

        // TODO parse command and write it into output
        if (strcmp(argv[arg_idx], "")) {
        } 
        else if (strcmp(argv[arg_idx], "")) {
        }
        else if (strcmp(argv[arg_idx], "")) {
        }
        else if (strcmp(argv[arg_idx], "")) {
        }
        else if (strcmp(argv[arg_idx], "")) {
        }
        else {
                print_help();
                return -1;
        }

        // TODO read response with proer timeout
        return 0;
}

