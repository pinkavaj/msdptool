/*
 * The sdp2xxx project.
 * Copyright (C) 2011  Jiří Pinkava
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msdp2xxx.h"
#ifdef __linux__
#else
#ifdef _WIN32
#include <windows.h>
#else
#error "Unsupported OS"
#endif
#endif

/**
 * Print message to standard error
 *
 * fmt:         format message, see man printf for details
 * returns:     -1 always
 **/
int printe(const char *fmt)
{
        fprintf(stderr, "%s\n", fmt);
        return -1;
}

/**
 * Same as perror, but returns -1
 *
 * str:         Message to print with error
 * returns:     -1 always
 */
int perror_(const char *str)
{
        perror(str);

        return -1;
}

/**
 * Print program help to standard output
 */
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
        sdp_t sdp;
#ifdef __linux__
        int fd_std_out;
#elif _WIN32
        HANDLE fd_std_out;
#endif
        int arg_idx = 1;
        int addr = 1;

        /* no args, print short help */
        if (argc < 2) {
                print_help_short();
                return -1;
        }

        if (argc == 2) {
                if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
                        print_help();
                        return 0;
                }
                return printe("Invalid or missing argument.");
        }

        if (!strcmp(argv[1], "-a")) {
                char *endptr;

                if (argc < 5)
                        return printe("Invalid or missing argument.");
                addr = strtol(argv[2], &endptr, 0);
                if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX ||
                                *endptr)
                        return printe("Device address out of range.");
                arg_idx = 3;
        }

#ifdef __linux__
        if (!strcmp(argv[arg_idx], "-")) {
                sdp.f_in = STDIN_FILENO;
                sdp.f_out = STDOUT_FILENO;
                fd_std_out = STDERR_FILENO;
        }
        else {
                int ret;

                ret = sdp_open(&sdp, argv[arg_idx], addr);
                if (ret == -1)
                        return -1;
                fd_std_out = STDOUT_FILENO;
        }
#elif _WIN32
        if (!strcmp(argv[arg_idx], "-")) {
                sdp.f_in = GetStdHandle(STD_INPUT_HANDLE);
                sdp.f_out = GetStdHandle(STD_OUTPUT_HANDLE);
                fd_std_out = GetStdHandle(STD_ERROR_HANDLE);
        } 
        else {
                int ret;

                ret = sdp_open(&sdp, argv[arg_idx], addr);
                // TODO
                if (ret == -1)
                        return -1;
                fd_std_out = GetStdHandle(STD_OUTPUT_HANDLE);

                //char buf[1024];

                // TODO
                /*FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, GetLastError(),
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                buf, sizeof(buf), NULL);
                fprintf(stderr, buf);*/
        }
#endif

        char *cmd = argv[arg_idx];
        // Drop already processed arguments
        argv += arg_idx;
        argc -= arg_idx;
        arg_idx = 0;

        // TODO parse command and write it into output
        if (!strcmp(cmd, "ccom")) {
                sdp_ifce_t ifce;
                int ret;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                if (!strcmp(argv[0], "rs232"))
                        ifce = sdp_ifce_rs232;
                else if (!strcmp(argv[0], "rs485"))
                        ifce = sdp_ifce_rs485;
                else
                        return printe("Invalid argument");

                // TODO
                ret = sdp_select_ifce(&sdp, ifce);
                if (ret == -1)
                        return printe("bug: sdp_select_ifce");
                // TODO
        }
        else if (!strcmp(cmd, "gcom")) {
        }
        else if (!strcmp(cmd, "gmax")) {
        }
        else if (!strcmp(cmd, "govp")) {
        }
        else if (!strcmp(cmd, "getd")) {
        }
        else if (!strcmp(cmd, "gets")) {
        }
        else if (!strcmp(cmd, "getm")) {
        }
        else if (!strcmp(cmd, "getp")) {
        }
        else if (!strcmp(cmd, "gpal")) {
        }
        else if (!strcmp(cmd, "volt")) {
        }
        else if (!strcmp(cmd, "curr")) {
        }
        else if (!strcmp(cmd, "sovp")) {
        }
        else if (!strcmp(cmd, "sout")) {
        }
        else if (!strcmp(cmd, "poww")) {
        }
        else if (!strcmp(cmd, "prom")) {
        }
        else if (!strcmp(cmd, "prop")) {
        }
        else if (!strcmp(cmd, "runm")) {
        }
        else if (!strcmp(cmd, "runp")) {
        }
        else if (!strcmp(cmd, "stop")) {
                int ret;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                ret = sdp_stop(&sdp);
                if (ret == -1)
                        return printe("bug: sdp_stop");
        }
        else {
                print_help();
                return -1;
        }

        // TODO read response with proper timeout
        return 0;
}

