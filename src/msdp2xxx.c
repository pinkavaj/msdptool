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

#include "msdp2xxx.h"
#include "msdp2xxx_low.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __linux__

#include <termios.h>
#include <sys/select.h>
#include <unistd.h>

/**
 * Open serial port and set parameters as defined for SDP power source
 *
 * fname:       file name of serial port
 * returns:     file descriptor on success, -1 on error
 */
static int open_serial(const char* fname)
{
        int fd;
        struct termios tio;

        fd = open(fname, O_RDWR | O_NONBLOCK);
        if (fd == -1)
                return -1;

        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CREAD | CLOCAL;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 5;
        cfsetispeed(&tio, B9600);
        cfsetospeed(&tio, B9600);

        if (tcsetattr(fd, TCSANOW, &tio) == -1) {
                int e = errno;
                close(fd);
                errno = e;

                return -1;
        }

        return fd;
}

/**
 * Close opened serial port, if f == -1 do nothigh
 *
 * f:           file descriptor
 */
static void close_serial(int f)
{
        if (f != -1)
                close(f);
}

/**
 * Reads data from serial port
 *
 * fd:          file descriptor
 * buf:         buffer to store readed data
 * count:       number of bytes to read
 */
static ssize_t sdp_read_resp(int fd, char *buf, ssize_t count)
{
        const char *buf_ = buf;
        fd_set readfds;
        int ret;
        ssize_t size = 0;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // TODO: check this value
        timeout.tv_sec = 0;
        // (bytes + something) * msec / (bitrate / bits per byte)
        timeout.tv_usec = ((count + 10) * 1000) / (9600 / 10);
        do {
                ssize_t size_;

                ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
                if (ret <= 0) {
                        if (ret == 0)
                                errno = ETIMEDOUT;
                        return -1;
                }
                size_ = read(fd, buf, count);
                if (size_ == -1)
                        return -1;
                size += size_;
                count -= size_;
                buf += size_;
                if (sdp_resp(buf_, size) != sdp_resp_incomplete)
                        return size;
        } while (count > 0);

        errno = ERANGE;
        return -1;
}

/**
 * Write data into serial port
 *
 * fd:          file descriptor
 * buf:         buffer with data to write
 * count:       number of bytes to write
 */
static ssize_t sdp_write(int fd, char *buf, ssize_t count)
{
        return write(fd, buf, count);
}
#endif

#ifdef _WIN32
/**
 * Open serial port and set parameters as defined for SDP power source
 *
 * fname:       file name of serial port
 * returns:     file descriptor on success, -1 on error
 */
static HANDLE open_serial(LPCWSTR fname)
{
        HANDLE h;

        h = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, 0,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (h == INVALID_HANDLE_VALUE)
                return INVALID_HANDLE_VALUE;

        DCB dcbSerialParams;

        SecureZeroMemory(&dcbSerialParams, sizeof(DCB));
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(h, &dcbSerialParams)) {
                DWORD e;

                e = GetLastError();
                CloseHandle(h);
                SetLastError(e);
                return INVALID_HANDLE_VALUE;
        }

        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(h, &dcbSerialParams)) {
                DWORD e;

                e = GetLastError();
                CloseHandle(h);
                SetLastError(e);
                return INVALID_HANDLE_VALUE;
        }

        COMMTIMEOUTS timeouts = { 0 };

        // TODO: check this values
        timeouts.ReadIntervalTimeout = 1;
        timeouts.ReadTotalTimeoutConstant = 10;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.WriteTotalTimeoutConstant = 1;
        timeouts.WriteTotalTimeoutMultiplier = 1;

        if(!SetCommTimeouts(h, &timeouts)) {
                DWORD e;

                e = GetLastError();
                CloseHandle(h);
                SetLastError(e);
                return INVALID_HANDLE_VALUE;
        }

        return h;
}

/**
 * Close opened serial port, if f == INVALID_HANDLE_VALUE do nothigh
 *
 * f:           file handle
 */
static void close_serial(HANDLE f)
{
        if (f != INVALID_HANDLE_VALUE)
                CloseHandle(f);
}

/**
 * Read data from serial port
 *
 * h:           file handle
 * buf:         buffer to store readed data
 * count:       number of bytes to read
 * returns:     number of bytes read
 */
static ssize_t sdp_read_resp(HANDLE h, char *buf, ssize_t count)
{
        DWORD readb;

        // TODO
        if (!ReadFile(h, buf, count, &readb, NULL))
                return -1;

        return readb;
}

/**
 * Write data into serial port
 *
 * h:           file handle
 * buf:         buffer with data to send
 * count:       number of bytes to send
 * returns:     number of bytes succesfully writen
 */
static ssize_t sdp_write(HANDLE h, char *buf, ssize_t count)
{
        DWORD writeb;

        if (!WriteFile(h, buf, count, &writeb, NULL))
            return -1;

        return writeb;
}
#endif

/**
 * Open serial port to comunicate with SDP power supply
 *
 * sdp:         pointer to uninitialized sdp_t structure
 * fname:       name of serial port to open
 * addr:        rs485 address of device, for rs232 is ignored - use anny valid
 */
int sdp_open(sdp_t *sdp, const char *fname, int addr)
{
        SDP_F f;

        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX) {
                errno = ERANGE;
                return -1;
        }

        f = open_serial(fname);
        if (f == SDP_F_ERR)
                return -1;

        sdp->f_in = sdp->f_out = f;
        sdp->addr = addr;

        return 0;
}

/**
 * Initialize sdp usign existing opened file
 *
 * sdp:         pointer to uninitialized sdp_t structure
 * f:           file descriptor or handle, depend on OS
 * addr:        rs485 address of device, for rs232 is ignored - use anny valid
 */
int sdp_openf(sdp_t *sdp, SDP_F f, int addr)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX) {
                errno = ERANGE;
                return -1;
        }

        sdp->f_in = sdp->f_out = f;
        sdp->addr = addr;

        return 0;
}

/**
 * Close SDP and all asociated ports
 *
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 */
void sdp_close(sdp_t *sdp)
{
        close_serial(sdp->f_in);
        if (sdp->f_in != sdp->f_out)
                close_serial(sdp->f_out);
}


/**
 * Get SDP device address. For devices connected on rs485 this returns
 *      same value as specified on sdp_open addr field or -1 when device is
 *      not connected. For rs232 returns last set rs485 device address.
 *
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     SDP device address, -1 if device not found or error
 */
int sdp_get_dev_addr(const sdp_t *sdp)
{
        int ret, addr;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_dev_addr(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_dev_addr(buf, ret, &addr) == -1)
                return -1;

        return addr;
}

/**
 * Get maximal values of current and voltage for this PS
 *
 * va_maximums: pointer to sdp_va_t, used to store recieved values
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_va_maximums(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_va_maximums(buf, ret, va_maximums) == -1)
                return -1;

        return 0;
}

/**
 * Get upper voltage limit
 *
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * volt:        pointer to float, uset to store retrieved value
 * returns:     0 on success, -1 on error
 */
int sdp_get_volt_limit(const sdp_t *sdp, float *volt)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_volt_limit(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_volt_limit(buf, ret, volt) == -1)
                return -1;

        return 0;
}

/**
 * Get actual value of current, voltage and mode of PS output
 *
 * va_data:     pointer to sdp_va_data_t, used to store returned values
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_va_data(const sdp_t *sdp, sdp_va_data_t *va_data)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_va_data(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_va_data(buf, ret, va_data) == -1)
                return -1;

        return 0;
}

/**
 * Get actual setpoint - desired current / voltage value
 *
 * va_setpoints: pointer to sdp_va_t, used to store retrieved values
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_va_setpoint(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_va_setpoint(buf, ret, va_setpoints) == -1)
                return -1;

        return 0;
}

/**
 * Get value of one or all presets stored in PS
 *
 * presn:       number of preset in range 1 - 9 to get from PS or
 *      SDP_PRESET_ALL to get all 9 preset values at once.
 * va_preset:   pointer to sdp_va_t, used to store retrieved velue
 *      or pointer to first item of array of 9 sdp_va_t, to store all presets
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset)
{
        int ret;
        char buf[(7*9+3+1)];

        ret = sdp_sget_preset(buf, sdp->addr, presn);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_preset(buf, ret, va_preset) == -1)
                return -1;

        return 0;
}

/**
 * Get one or all program items from PS
 *
 * progn:       program item number to get (0-19) or SDP_PROGRAM_ALL to get
 *      all program items at once
 * program:     pointer to sdp_program_t to store one program item or
 *      pointer to firt item of array of 20 items of type sdp_program_t to
 *      store all program items
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_program(const sdp_t *sdp, int progn, sdp_program_t *program)
{
        int ret;
        char buf[(11*20+3+1)];

        ret = sdp_sget_program(buf, sdp->addr, progn);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_program(buf, ret, program) == -1)
                return -1;

        return 0;
}

/**
 * Get LCD info, return data about all informations currently shown on LCD
 *
 * lcd_info:    pointer to sdp_ldc_info_t where informations should be stored
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_get_ldc_info(const sdp_t *sdp, sdp_ldc_info_t *lcd_info)
{
        int ret;
        // TODO: setup proper buffer size
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sget_ldc_info(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf));
        if (ret == -1)
                return -1;

        if (sdp_resp_ldc_info(buf, ret, lcd_info) == -1)
                return -1;

        return 0;
}

/**
 * Enable/disable remote operation, disable/enable keyboard (manula) operation
 *
 * enable:      when 0 disable remote operation, otherwise enable
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_remote(const sdp_t *sdp, int enable)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_sremote(buf, sdp->addr, enable);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK);
        if (ret == -1)
                return -1;

        return 0;
}

/**
 * Load preset and set it as current wanted value
 *
 * preset:      number of preset to load
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_run_preset(const sdp_t *sdp, int preset)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_srun_preset(buf, sdp->addr, preset);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK);
        if (ret == -1)
                return -1;

        return 0;
}

/**
 * Run timed program, once or repeatly
 *
 * count:       count of program repeats or SDP_RUN_PROG_INF to repeat forever
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_run_program(const sdp_t *sdp, int count)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        ret = sdp_srun_program(buf, sdp->addr, count);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) == -1)
                return -1;

        ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK);
        if (ret == -1)
                return -1;

        return 0;
}

/**
 * Select comunication interface (rs232/rs485)
 *
 * ifce:        one of sdp_ifce_rs232 or sdp_ifce_rs485
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_select_ifce(const sdp_t *sdp, sdp_ifce_t ifce)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sselect_ifce(buf, sdp->addr, ifce);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set setpont for current
 *
 * curr:        wanted output current of PS: [A]
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_curr(const sdp_t *sdp, float curr)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_curr(buf, sdp->addr, curr);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set setpoint for voltage
 *
 * volt:        wanted output voltage of PS: [V]
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_volt(const sdp_t *sdp, float volt)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_volt(buf, sdp->addr, volt);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set upper voltage limit
 *
 * volt:        wanted upper voltage limit: [V]
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_volt_limit(const sdp_t *sdp, float volt)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_volt_limit(buf, sdp->addr, volt);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set PS output to on or off
 *
 * enable:      when 0 turn output off, otherwise turn on
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_output(const sdp_t *sdp, int enable)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_output(buf, sdp->addr, enable);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set power on status of output for specific preset item
 *
 * presn:       number of preset to set output state
 * enable:      when 0 output is disablen on power, otherwise enabled
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_poweron_output(const sdp_t *sdp, int presn, int enable)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_poweron_output(buf, sdp->addr, presn, enable);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set value of preset item
 *
 * presn:       number of preset to set
 * va_preset:   new value of preset
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_preset(buf, sdp->addr, presn, va_preset);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Set value of program item
 *
 * progn:       number of program item to set
 * program:     new value of program item
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_set_program(const sdp_t *sdp, int progn, const sdp_program_t *program)
{
        char buf[SDP_BUF_SIZE_MIN];
        ssize_t size;

        size = sdp_sset_program(buf, sdp->addr, progn, program);
        if (size == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, size) != size)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK))
                return -1;

        return 0;
}

/**
 * Stop running program
 *
 * sdp:         pointer to sdp_t structure, initialized by sdp_open
 * returns:     0 on success, -1 on error
 */
int sdp_stop(const sdp_t *sdp)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        ret = sdp_sstop(buf, sdp->addr);
        if (ret == -1)
                return -1;

        if (sdp_write(sdp->f_out, buf, ret) != ret)
                return -1;

        if (sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK) == -1)
                return -1;

        return 0;
}

