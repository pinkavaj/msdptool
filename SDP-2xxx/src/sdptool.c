#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sdp2xxx.h"

void print_help(void)
{
        printf("sdptool [-h] [-a <addr>] <io port> <CMD>\n"
        "        -h - print help\n"
        "        -a - set device address\n"
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

void print_help_short(void)
{
        printf("sdptool - commandline utility for remote control of SDP series power supplies\n"
               "        Use -h or --help for help\n");
                        
}

int main(int argc, char **argv)
{
        int fd_dev_in, fd_dev_out, fd_std_out;
        int arg_idx = 1;
        int addr = 1;

        /* no args, print short help */
        if (argc < 2) {
                print_help_short();
                return -1;
        }

        if (argc == 2) {
                print_help();
                if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
                        return 0;
                return -1;
        }

        if (!strcmp(argv[1], "-a")) {
                char *endptr;

                if (argc < 5) {
                        print_help();
                        return -1;
                }
                addr = strtol(argv[2], &endptr, 0);
                if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX ||
                                *endptr) {
                        print_help();
                        return -1;
                }
                arg_idx = 3;
        }

        if (!strcmp(argv[arg_idx], "-")) {
                fd_dev_in = 1;
                fd_dev_out = 2;
                fd_std_out = 3;
        } 
        else {
                fd_dev_in = fd_dev_out = open(argv[arg_idx], O_RDWR);
                if (fd_dev_in == -1) {
                        perror("Failed to open port");
                        return -1;
                }
                fd_std_out = 2;
                // TODO set parameters of serial port to 9600 8n1
        }

        // TODO parse command and write it into output
        if (strcmp(argv[arg_idx], "ccom")) {
        }
        else if (strcmp(argv[arg_idx], "gcom")) {
        }
        else if (strcmp(argv[arg_idx], "gmax")) {
        }
        else if (strcmp(argv[arg_idx], "govp")) {
        }
        else if (strcmp(argv[arg_idx], "getd")) {
        }
        else if (strcmp(argv[arg_idx], "gets")) {
        }
        else if (strcmp(argv[arg_idx], "getm")) {
        }
        else if (strcmp(argv[arg_idx], "getp")) {
        }
        else if (strcmp(argv[arg_idx], "gpal")) {
        }
        else if (strcmp(argv[arg_idx], "volt")) {
        }
        else if (strcmp(argv[arg_idx], "curr")) {
        }
        else if (strcmp(argv[arg_idx], "sovp")) {
        }
        else if (strcmp(argv[arg_idx], "sout")) {
        }
        else if (strcmp(argv[arg_idx], "poww")) {
        }
        else if (strcmp(argv[arg_idx], "prom")) {
        }
        else if (strcmp(argv[arg_idx], "prop")) {
        }
        else if (strcmp(argv[arg_idx], "runm")) {
        }
        else if (strcmp(argv[arg_idx], "runp")) {
        }
        else if (strcmp(argv[arg_idx], "stop")) {
        }
        else {
                print_help();
                return -1;
        }

        // TODO read response with proper timeout
        return 0;
}

