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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msdp2xxx.h"
#ifdef __linux__
#include <unistd.h>
#define TEXT(s) (s)
#define _TCHAR char
#else
#ifdef _WIN32
#include <wchar.h>
#define _TCHAR wchar_t

// FIXME this is hack for MinGW
#ifdef TEXT
#undef TEXT
#endif
#define TEXT(s) (L##s)

#define strcmp(a, b) wcscmp(a, b)
#define strtod(a, b) wcstod(a, b)
#define strtol(a, b, c) wcstol(a, b, c)
#else
#error "Unsupported OS"
#endif
#endif

/**
 * Same as perror, but return -1;
 *
 * fmt:         format message, see man printf for details
 * returns:     -1 always
 **/
static int perror_(const char *fmt)
{
        // TODO
        /*
		char buf[1024];

		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL, GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        buf, sizeof(buf), NULL);
        fprintf(stderr, buf);*/
        perror(fmt);

        return -1;
}

/**
 * Print string to standard error output and return -1
 *
 * str:         string to print
 * return:      -1 always
 */
static int printe(const char *str)
{
        fprintf(stderr, "%s", str);

        return -1;
}

/**
 * Print program help to standard output
 */
static void print_help(void)
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
        "                getm { 1 - 9 | all } |\n"
        "                getp { 0 - 19 | all } |\n"
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

static void print_help_short(void)
{
        printf("sdptool - commandline utility for remote control of SDP series power supplies\n"
               "        Use -h or --help for help\n");
                        
}

int main(int argc, char **argv_)
{
        int addr = 1;
        int arg_idx = 1;
        FILE *f_stdout;
        int ret;
        sdp_t sdp;
		const _TCHAR *cmd;

#ifdef _WIN32
		wchar_t **argv;

        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		// TODO call LocalFree(argv);
#else
        char **argv = argv_;
#endif
        /* no args, print short help */
        if (argc < 2) {
                print_help_short();
                return -1;
        }

        if (argc == 2) {
                if (!strcmp(argv[1], TEXT("-h")) ||
                                !strcmp(argv[1], TEXT("--help"))) {
                        print_help();
                        return 0;
                }
                return printe("Invalid or missing argument");
        }

        if (!strcmp(argv[1], TEXT("-a"))) {
                _TCHAR *endptr;

                if (argc < 5)
                        return printe("Invalid or missing argument");
                addr = strtol(argv[2], &endptr, 0);
                if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX ||
                                *endptr)
                        return printe("Device address out of range");
                arg_idx = 3;
        }

#ifdef __linux__
        if (!strcmp(argv[arg_idx], "-")) {
                ret = sdp_openf(&sdp, -1, addr);
                if (ret == -1)
                        return perror_("sdp_open failed");

                sdp.f_in = STDIN_FILENO;
                sdp.f_out = STDOUT_FILENO;
                f_stdout = stderr;
        }
        else {

                ret = sdp_open(&sdp, argv[arg_idx], addr);
                if (ret == -1)
                        return perror_("sdp_open failed");
                f_stdout = stdout;
        }
#elif _WIN32
        if (!strcmp(argv[arg_idx], TEXT("-"))) {
                ret = sdp_openf(&sdp, INVALID_HANDLE_VALUE, addr);
                if (ret == -1)
                        return perror_("sdp_open failed");

                sdp.f_in = GetStdHandle(STD_INPUT_HANDLE);
                sdp.f_out = GetStdHandle(STD_OUTPUT_HANDLE);
                f_stdout = stderr;
        } 
        else {
                ret = sdp_open(&sdp, argv[arg_idx], addr);
                if (ret == -1)
                        return perror_("sdp_open failed");
                f_stdout = stdout;
        }
#endif

        if (sdp_remote(&sdp, 1) == -1)
                return perror_("Failed to switch to remote mode");

        // Drop already processed arguments
        arg_idx++;
		cmd = argv[arg_idx++];
        argv += arg_idx;
        argc -= arg_idx;
        arg_idx = 0;

        if (!strcmp(cmd, TEXT("ccom"))) {
                sdp_ifce_t ifce;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                if (!strcmp(argv[0], TEXT("rs232")))
                        ifce = sdp_ifce_rs232;
                else if (!strcmp(argv[0], TEXT("rs485")))
                        ifce = sdp_ifce_rs485;
                else
                        return printe("Invalid argument");

                ret = sdp_select_ifce(&sdp, ifce);
                if (ret == -1)
                        return perror_("sdp_select_ifce failed");
        }
        else if (!strcmp(cmd, TEXT("gcom"))) {
                if (argc != 0)
                        return printe("Invalid number of parameters");

                ret = sdp_get_dev_addr(&sdp);
                if (ret == -1)
                        return perror_("sdp_get_dev_addr failed");

                fprintf(f_stdout, "%i\n", ret);
        }
        else if (!strcmp(cmd, TEXT("gmax"))) {
                sdp_va_t va;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                if (sdp_get_va_maximums(&sdp, &va) == -1)
                        return perror_("sdp_get_va_maximums failed");

                fprintf(f_stdout, "%2.1f %1.2f\n", va.volt, va.curr);
        }
        else if (!strcmp(cmd, TEXT("govp"))) {
                double volt;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                ret = sdp_get_volt_limit(&sdp, &volt);
                if (ret == -1)
                        return perror_("sdp_get_volt_limit failed");

                fprintf(f_stdout, "%2.1f\n", volt);
        }
        else if (!strcmp(cmd, TEXT("getd"))) {
                sdp_va_data_t va_data;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                if (sdp_get_va_data(&sdp, &va_data) == -1)
                        return perror_("sdp_get_va_data failed");

                if (va_data.mode == sdp_mode_cc)
                        fprintf(f_stdout, "%2.2f %1.3f CC\n", va_data.volt,
                                        va_data.curr);
                else
                        fprintf(f_stdout, "%2.2f %1.3f CV\n", va_data.volt,
                                        va_data.curr);
        }
        else if (!strcmp(cmd, TEXT("gets"))) {
                sdp_va_t va;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                if (sdp_get_va_setpoint(&sdp, &va) == -1)
                        return perror_("sdp_get_va_setpoint failed");

                fprintf(f_stdout, "%2.1f %1.2f\n", va.volt, va.curr);
        }
        else if (!strcmp(cmd, TEXT("getm"))) {
                sdp_va_t va[9];
                int presn, n;
                _TCHAR *endptr;
				int x;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                if (!strcmp(argv[0], TEXT("all"))) {
                        presn = SDP_PRESET_ALL;
                        n = 9;
                } else {
                        presn = strtol(argv[0], &endptr, 0);
                        if (!argv[0][0] || *endptr)
                                return printe("Invalid preset number");
                        n = 1;
                }

                if (sdp_get_preset(&sdp, presn, va) == -1)
                        return perror_("sdp_get_preset failed");

                for (x = 0; x < n; x++)
                        fprintf(f_stdout, "%2.1f %1.2f\n", va[x].volt,
                                        va[x].curr);
        }
        else if (!strcmp(cmd, TEXT("getp"))) {
                int progn, n;
                sdp_program_t prg[20];
                _TCHAR *endptr;
				int x;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                if (!strcmp(argv[0], TEXT("all"))) {
                        progn = SDP_PROGRAM_ALL;
                        n = 20;
                } else {
                        progn = strtol(argv[0], &endptr, 0);
                        if (!argv[0][0] || *endptr)
                                return printe("Invalid preset number");
                        n = 1;
                }

                if (sdp_get_program(&sdp, progn, prg) == -1)
                        return perror_("sdp_get_program failed");

                for (x = 0; x < n; x++)
                        fprintf(f_stdout, "%2.1f %1.2f %2.i:%2.i\n",
                                        prg[x].volt, prg[x].curr,
                                        prg[x].time / 60, prg[x].time % 60);
        }
        else if (!strcmp(cmd, TEXT("gpal"))) {
                sdp_ldc_info_t lcd;

                if (argc != 0)
                        return printe("Invalid number of parameters");

                if (sdp_get_ldc_info(&sdp, &lcd) == -1)
                        return perror_("sdp_get_ldc_info failed");
                // TODO
                return -1;
        }
        else if (!strcmp(cmd, TEXT("volt"))) {
                double u;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                u = strtod(argv[0], &endptr);
                if (*endptr || !argv[0][0])
                        return printe("Invalid argument, not a number");

                if (sdp_set_volt(&sdp, u) == -1)
                        return perror_("sdp_set_volt failed");
        }
        else if (!strcmp(cmd, TEXT("curr"))) {
                double i;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                i = strtod(argv[0], &endptr);
                if (*endptr || !argv[0][0])
                        return printe("Invalid argument, not a number");

                if (sdp_set_curr(&sdp, i) == -1)
                        return perror_("sdp_set_curr failed");
        }
        else if (!strcmp(cmd, TEXT("sovp"))) {
                double u;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                u = strtod(argv[0], &endptr);
                if (*endptr || !argv[0][0])
                        return printe("Invalid argument, not a number");

                if (sdp_set_volt_limit(&sdp, u) == -1)
                        return perror_("sdp_set_volt_limit failed");
        }
        else if (!strcmp(cmd, TEXT("sout"))) {
                int enable;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                if (!strcmp(argv[0], TEXT("on")))
                        enable = 1;
                else if (!strcmp(argv[0], TEXT("off")))
                        enable = 0;
                else
                        return printe("Expected one of on/off");

                if (sdp_set_output(&sdp, enable) == -1)
                        return perror_("sdp_set_output failed");
        }
        else if (!strcmp(cmd, TEXT("poww"))) {
                int enable, presn;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                presn = strtol(argv[0], &endptr, 0);
                if (!argv[0][0] || *endptr)
                        return printe("Expected preset number");

                if (!strcmp(argv[1], TEXT("on")))
                        enable = 1;
                else if (!strcmp(argv[1], TEXT("off")))
                        enable = 0;
                else
                        return printe("Expected one of on/off");

                if (sdp_set_poweron_output(&sdp, presn, enable) == -1)
                        return perror_("sdp_set_poweron_output failed");
        }
        else if (!strcmp(cmd, TEXT("prom"))) {
                int presn;
                sdp_va_t va;
                _TCHAR *endptr;

                if (argc != 3)
                        return printe("Invalid number of parameters");

                presn = strtol(argv[0], &endptr, 0);
                if (!argv[0][0] || *endptr)
                        return printe("Expected preset number");

                va.volt = strtod(argv[1], &endptr);
                if (*endptr || !argv[1][0])
                        return printe("Invalid argument, not a number");

                va.curr = strtod(argv[2], &endptr);
                if (*endptr || !argv[2][0])
                        return printe("Invalid argument, not a number");

                if (sdp_set_preset(&sdp, presn, &va) == -1)
                        return perror_("sdp_set_preset failed");
        }
        else if (!strcmp(cmd, TEXT("prop"))) {
                int progn;
                sdp_program_t prg;
                _TCHAR *endptr;

                if (argc != 4)
                        return printe("Invalid number of parameters");

                progn = strtol(argv[0], &endptr, 0);
                if (!argv[0][0] || *endptr)
                        return printe("Expected program number");

                prg.volt = strtod(argv[1], &endptr);
                if (*endptr || !argv[1][0])
                        return printe("Invalid argument, not a number");

                prg.curr = strtod(argv[2], &endptr);
                if (*endptr || !argv[2][0])
                        return printe("Invalid argument, not a number");

                prg.time = strtol(argv[3], &endptr, 0);
                if (!argv[3][0] || *endptr)
                        return printe("Invalid program number");

                if (sdp_set_program(&sdp, progn, &prg) == -1)
                        return perror_("sdp_set_program failed");
        }
        else if (!strcmp(cmd, TEXT("runm"))) {
                int presn;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                presn = strtol(argv[0], &endptr, 0);
                if (!argv[0][0] || *endptr)
                        return printe("Invalid preset number");

                if (sdp_run_preset(&sdp, presn) == -1)
                        return perror_("sdp_run_preset failed");
        }
        else if (!strcmp(cmd, TEXT("runp"))) {
                int progn;
                _TCHAR *endptr;

                if (argc != 1)
                        return printe("Invalid number of parameters");

                progn = strtol(argv[0], &endptr, 0);
                if (!argv[0][0] || *endptr)
                        return printe("Expected program number");

                if (sdp_run_program(&sdp, progn) == -1)
                        return perror_("sdp_run_program failed");
        }
        else if (!strcmp(cmd, TEXT("stop"))) {
                if (argc != 0)
                        return printe("Invalid number of parameters");

                if (sdp_stop(&sdp) == -1)
                        return perror_("sdp_stop failed");
        }
        else {
                print_help_short();
                printe("Unknown command");
                return -1;
        }

        // TODO: move all SDP functions calling block into function
        // returning 0 / -1, this allows call
        // a) multiple commands
        // b) call this even if no success (want we call this on error?)
        if (sdp_remote(&sdp, 0) == -1)
                return perror_("Failed to disable remote mode");

        return 0;
}

