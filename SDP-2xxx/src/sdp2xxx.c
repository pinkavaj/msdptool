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

#include "sdp2xxx.h"
#include "sdp2xxx_low.h"
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
static int open_serial(const char* fname)
{
        int fd;

        fd = open(fname, O_RDWR);
/*        if (fd == -1)
                perror_("Failed to open port");*/
        // TODO set parameters of serial port to 9600 8n1
        return fd;
}

// TODO
static int sdp_read(int fd, char *buf, ssize_t count)
{
        ssize_t size;

        size = read(fd, buf, 1);
        if (size == -1)
                return -1;
                //error
        // TODO
        return -1;
}

static ssize_t sdp_write(int fd, char *buf, ssize_t count)
{
        // TODO
        return write(fd, buf, count);
}
#endif

#ifdef _WIN32
static HANDLE open_serial(const char* fname)
{
        HANDLE h;

        h = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, 0,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (h == INVALID_HANDLE_VALUE) {
                char buf[1024];

                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, GetLastError(),
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                buf, sizeof(buf), NULL);
                fprintf(stderr, buf);

                return h;
        }
        // TODO set parameters of serial port to 9600 8n1

        return h;
}

static sdp_resp_t sdp_read(HANDLE h, char *buf, ssize_t count)
{
        DWORD readb;

        if (!ReadFile(h, buf, 1, &readb, NULL)) {
                // error
        }
        // TODO
        return -1;
}

static ssize_t sdp_write(HANDLE h, char *buf, ssize_t count)
{
        DWORD writeb;
        // TODO
        if (!WriteFile(h, buf, count, &writeb, NULL))
            return -1;
        return count;
}
#endif

int sdp_open(sdp_t *sdp, const char *fname, int addr)
{
        SDP_F f;

        f = open_serial(fname);
        if (f == SDP_F_ERR)
                return -1;

        return -1;
}

int sdp_openf(sdp_t *sdp, SDP_F f, int addr)
{
        return -1;
}

void sdp_close(sdp_t *sdp)
{
        return;
}


int sdp_get_dev_addr(const sdp_t *sdp)
{
        return -1;
}

int sdp_get_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums)
{
        return -1;
}

int sdp_get_volt_limit(const sdp_t *sdp)
{
        return -1;
}

int sdp_get_va_data(const sdp_t *sdp, sdp_va_data_t *va_data)
{
        return -1;
}

int sdp_get_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints)
{
        return -1;
}

int sdp_get_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset)
{
        return -1;
}

int sdp_get_program(const sdp_t *sdp, int progn, sdp_program_t *program)
{
        return -1;
}

int sdp_get_ldc_info(const sdp_t *sdp, sdp_ldc_info_t *lcd_info)
{
        return -1;
}

int sdp_remote(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_run_preset(const sdp_t *sdp, int preset)
{
        return -1;
}

int sdp_run_program(const sdp_t *sdp, int count)
{
        return -1;
}

// TODO
int sdp_select_ifce(const sdp_t *sdp, sdp_ifce_t ifce)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sselect_ifce(buf, sdp->addr, ifce);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;
                //return perror_("Comunication with device failed");

        if (sdp_read(sdp->f_in, buf, sizeof(buf) != sdp_resp_nodata))
                return -1;
                //return perror_("Comunication with device failed");

        return -1;
}

int sdp_set_curr(const sdp_t *sdp, int curr)
{
        return -1;
}

int sdp_set_volt(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_set_volt_limit(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_set_output(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_set_poweron_output(const sdp_t *sdp, int preset, int enable)
{
        return -1;
}

int sdp_set_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset)
{
        return -1;
}

int sdp_set_program(const sdp_t *sdp, int progn, const sdp_program_t *program)
{
        return -1;
}

int sdp_stop(const sdp_t *sdp)
{
        return -1;
}

