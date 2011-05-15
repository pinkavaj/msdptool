#include <stdio.h>
#include <string.h>
#include "sdp2xxx.h"

void print_help(void)
{
        printf("sdptool [-a <addr>] <io port> <CMD>\n"
        "        <addr>  - rs485 address of device (1 - 31), defaults to 1\n"
        "        <io port> - port for comunication with SDP power supply\n"
        "                win: { COM1 | COM2 | ... }\n"
        "                Linux: { /dev/ttyS0 | /dev/ttyS1 | ... }\n"
        "                use \"-\" for std. in and out, output will be send to stderr\n"
        "        CMD := {\n"
//        printf("                <sess> - disable front keypad, make PS to |
//        printf("                <ends> |\n");
        "                ccom { rs232 | rs485 } |\n"
        "                gcom |\n"
        "                gmax |\n"
        "                govp |\n"
        "                getd |\n"
        "                gets |\n"
        "                getm [{ 1 - 9 }] |\n"
        "                getp [{ 0 - 19}] |\n"
        "                gpal |\n"
        "                volt { 0 - 99.9 } |\n"
        "                curr { 0 - 9.99 } |\n"
        "                sovp { 0 - 99.9 } |\n"
        "                sout { on | off } |\n"
        "                poww { 1 - 9 } { on | off } |\n"
        "                prom { 1 - 9 } { 0 - 99.9 } { 0 - 9.99 } |\n"
        "                prop { 0 - 19 } { 0 - 99.9 } { 0 - 9.99 } { 00:00 - 99:59 } |\n"
        "                runm { 1 - 9 } |\n"
        "                runp { 1 - 999 | inf } |\n"
        "                stop }\n"
        "        See SDP power supply manual for detailed informations about commands\n");
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

