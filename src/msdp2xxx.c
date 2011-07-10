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
 * Open serial port and set parameters as defined for SDP power source.
 * @param fname File name of serial port.
 * @return      File descriptor on success, negative number (err no.) on error.
 */
static int open_serial(const char* fname)
{
        int fd;
        struct termios tio;

        fd = open(fname, O_RDWR | O_NONBLOCK);
        if (fd < 0)
                return SDP_EERRNO;

        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CREAD | CLOCAL;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 5;
        cfsetispeed(&tio, B9600);
        cfsetospeed(&tio, B9600);

        if (tcsetattr(fd, TCSANOW, &tio) < 0) {
                int e = errno;
                close(fd);
                errno = e;

                return SDP_EERRNO;
        }

        return fd;
}

/**
 * Close opened serial port, if f == -1 do nothigh.
 * @param f     File descriptor.
 */
static void close_serial(int f)
{
        if (f >= 0)
                close(f);
}

/**
 * Reads data from serial port.
 * @param fd    File descriptor.
 * @param buf   Buffer to store readed data.
 * @param count Maximal amount of bytes to read.
 * @return      Number of bytes succesfully readed, or gefative number
 *      (error no.) on error.
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
        // (bytes * 10 * usec) / bitrate + delay_to_reaction;
        timeout.tv_usec = (count * 10l * 1000000l) / 9600l + 70000l;
        do {
                ssize_t size_;

                ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
                if (ret <= 0) {
                        if (ret == 0)
                                errno = ETIMEDOUT;
                        return SDP_ETIMEDOUT;
                }
                size_ = read(fd, buf, count);
                if (size_ < 0)
                        return SDP_EERRNO;
                size += size_;
                count -= size_;
                buf += size_;
                if (sdp_resp(buf_, size) != sdp_resp_incomplete)
                        return size;
        } while (count > 0);

        errno = ERANGE;
        return SDP_ETOLARGE;
}

/**
 * Write data into serial port. Return error when not all data
 *      succesfully writen.
 * @param fd    File descriptor.
 * @param buf   Buffer with data to write.
 * @param count Maximal amout of bytes to write.
 * @return      Number of bytes succesfully writen, or negative number
 *      (error no.) on error.
 */
static ssize_t sdp_write(int fd, char *buf, ssize_t count)
{
        ssize_t count_;

        count_ = write(fd, buf, count);
        if (count_ >= 0 && count_ != count)
                return SDP_EWINCOMPL;

        return count_;
}
#endif

#ifdef _WIN32

#ifdef _MSVC
typedef int ssize_t;
#endif

/**
 * Open serial port and set parameters as defined for SDP power source.
 * @param fname File name of serial port.
 * @return      File descriptor on success, -1/INVALID_HANDLE_VALUE on error.
 */
#ifdef _WIN32
static HANDLE open_serial(const wchar_t *fname)
#else
static HANDLE open_serial(const char *fname)
#endif
{
        HANDLE h;

        h = CreateFileW(fname, GENERIC_READ | GENERIC_WRITE, 0, 0,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (h == INVALID_HANDLE_VALUE)
                return INVALID_HANDLE_VALUE;

        DCB dcbSerialParams;

        memset(&dcbSerialParams, 0, sizeof(DCB));
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
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 2;
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
 * Close opened serial port, if f == INVALID_HANDLE_VALUE do nothigh.
 * @param f     File handle.
 */
static void close_serial(HANDLE f)
{
        if (f != INVALID_HANDLE_VALUE)
                CloseHandle(f);
}

/**
 * Read data from serial port.
 * @param h     File handle.
 * @param buf   Buffer to store readed data.
 * @param count Number of bytes to read.
 * @return      Number of bytes readed, or negative number (error no.)
 *      on error.
 */
static ssize_t sdp_read_resp(HANDLE h, char *buf, ssize_t count)
{
        DWORD readb;

        // TODO
        if (!ReadFile(h, buf, count, &readb, NULL))
                return SDP_EERRNO;

	if (readb == 0) {
		errno = EIO;
		return SDP_ETIMEDOUT;
	}

        return readb;
}

/**
 * Write data into serial port.
 * @param h     File handle.
 * @param buf   Buffer with data to send.
 * @param count Number of bytes to send.
 * @return      Number of bytes succesfully writen, or negative number
 *      (error no.) on error.
 */
static ssize_t sdp_write(HANDLE h, char *buf, ssize_t count)
{
        DWORD writeb;

        if (!WriteFile(h, buf, count, &writeb, NULL))
            return SDP_EERRNO;

        if (writeb != count)
                return SDP_EWINCOMPL;

        return writeb;
}
#endif

/**
 * Open serial port to comunicate with SDP power supply.
 * @param sdp   Pointer to uninitialized sdp_t structure.
 * @param fname Name of serial port to open.
 * @param addr  RS485 address of device, for RS232 is ignored - use anny valid.
 * @return      On success 0, on error negative number (error no.).
 */
#ifdef _WIN32
int sdp_open(sdp_t *sdp, const wchar_t *fname, int addr)
#else
int sdp_open(sdp_t *sdp, const char *fname, int addr)
#endif
{
        SDP_F f;

        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        f = open_serial(fname);
        if (f == SDP_F_ERR)
                return SDP_EERRNO;

        sdp->f_in = sdp->f_out = f;
        sdp->addr = addr;

        return 0;
}

/**
 * Initialize sdp usign existing opened file.
 * @param sdp   Pointer to uninitialized sdp_t structure.
 * @param f     File descriptor or handle, depend on OS.
 * @param addr  RS485 address of device, for RS232 is ignored - use anny valid.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_openf(sdp_t *sdp, SDP_F f, int addr)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX) {
                errno = ERANGE;
                return SDP_EERRNO;
        }

        sdp->f_in = sdp->f_out = f;
        sdp->addr = addr;

        return 0;
}

/**
 * Close SDP and all asociated ports.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 */
void sdp_close(sdp_t *sdp)
{
        close_serial(sdp->f_in);
        if (sdp->f_in != sdp->f_out)
                close_serial(sdp->f_out);
}


/**
 * Get SDP device address. For devices connected on RS485 this returns
 *      same value as specified on sdp_open addr field or -1 when device is
 *      not connected. For RS232 returns last set RS485 device address.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @return      SDP device address, or negative number (error no.) on error.
 */
int sdp_get_dev_addr(const sdp_t *sdp)
{
        int addr, ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sget_dev_addr(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_dev_addr(buf, ret, &addr)) < 0)
                return ret;

        return addr;
}

/**
 * Get maximal values of current and voltage for this PS.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param va_maximums   Pointer to sdp_va_t, used to store recieved values.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sget_va_maximums(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_va_maximums(buf, ret, va_maximums)) < 0)
                return ret;

        return 0;
}

/**
 * Get upper voltage limit.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param volt  pointer to double, uset to store retrieved value.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_volt_limit(const sdp_t *sdp, double *volt)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sget_volt_limit(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_volt_limit(buf, ret, volt)) < 0)
                return ret;

        return 0;
}

/**
 * Get actual value of current, voltage and mode of PS output.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param va_data       pointer to sdp_va_data_t, used to store returned values.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_va_data(const sdp_t *sdp, sdp_va_data_t *va_data)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sget_va_data(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_va_data(buf, ret, va_data)) < 0)
                return ret;

        return 0;
}

/**
 * Get actual setpoint - desired current / voltage value.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param       va_setpoints  pointer to sdp_va_t, used to store retrieved
 *      values.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sget_va_setpoint(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_va_setpoint(buf, ret, va_setpoints)) < 0)
                return ret;

        return 0;
}

/**
 * Get value of one or all presets stored in PS.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param presn       Number of preset in range 1 - 9 to get from PS or
 *      SDP_PRESET_ALL to get all 9 preset values at once.
 * @param va_preset   Pointer to sdp_va_t, used to store retrieved velue
 *      or pointer to first item of array of 9 sdp_va_t, to store all presets.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset)
{
        int ret;
        char buf[(7*9+3+1)];

        if ( (ret = sdp_sget_preset(buf, sdp->addr, presn)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_preset(buf, ret, va_preset)) < 0)
                return ret;

        return 0;
}

/**
 * Get one or all program items from PS.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param progn       program item number to get (0-19) or SDP_PROGRAM_ALL to get
 *      all program items at once.
 * @param program     pointer to sdp_program_t to store one program item or
 *      pointer to firt item of array of 20 items of type sdp_program_t to
 *      store all program items.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_program(const sdp_t *sdp, int progn, sdp_program_t *program)
{
        int ret;
        char buf[11*20+3+1];

        if ( (ret = sdp_sget_program(buf, sdp->addr, progn)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_program(buf, ret, program)) < 0)
                return ret;

        return 0;
}

/**
 * Get LCD info, return data about all informations currently shown on LCD.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param lcd_info      pointer to sdp_lcd_info_raw_t where informations
 *      should be stored.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_get_lcd_info(const sdp_t *sdp, sdp_lcd_info_t *lcd_info)
{
        int ret;
        char buf[100];
        sdp_lcd_info_raw_t lcd_info_raw;

        if ( (ret = sdp_sget_lcd_info(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, sizeof(buf))) < 0)
                return ret;

        if ( (ret = sdp_resp_lcd_info(buf, ret, &lcd_info_raw)) < 0)
                return ret;

	sdp_lcd_to_data(lcd_info, &lcd_info_raw);

        return 0;
}

/**
 * Enable/disable remote operation operation mode.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param enable        when 0 disable remote operation, otherwise enable.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_remote(const sdp_t *sdp, int enable)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_sremote(buf, sdp->addr, enable)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Load preset and set it as current wanted value.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param preset        number of preset to load.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_run_preset(const sdp_t *sdp, int preset)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_srun_preset(buf, sdp->addr, preset)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Run timed program, once or repeatly.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param count Count of program repeats or SDP_RUN_PROG_INF to
 *      repeat forever.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_run_program(const sdp_t *sdp, int count)
{
        int ret;
        char buf[SDP_BUF_SIZE_MIN];

        if ( (ret = sdp_srun_program(buf, sdp->addr, count)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Select comunication interface (RS232/RS485).
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param ifce  One of sdp_ifce_rs232 or sdp_ifce_rs485.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_select_ifce(const sdp_t *sdp, sdp_ifce_t ifce)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sselect_ifce(buf, sdp->addr, ifce)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set setpont for current.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param curr  Wanted output current of PS: [A].
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_curr(const sdp_t *sdp, double curr)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_curr(buf, sdp->addr, curr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set setpoint for voltage.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param volt  Wanted output voltage of PS: [V].
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_volt(const sdp_t *sdp, double volt)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_volt(buf, sdp->addr, volt)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set upper voltage limit.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param volt  Wanted upper voltage limit: [V].
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_volt_limit(const sdp_t *sdp, double volt)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_volt_limit(buf, sdp->addr, volt)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set PS output to on or off.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param enable        When 0 turn output off, otherwise turn on.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_output(const sdp_t *sdp, int enable)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_output(buf, sdp->addr, enable)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set power on status of output for specific preset item.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param presn Number of preset to set output state.
 * @param enable        When 0 output is disablen on power, otherwise enabled.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_poweron_output(const sdp_t *sdp, int presn, int enable)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_poweron_output(buf, sdp->addr, presn, enable)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set value of preset item.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param presn number of preset to set.
 * @param va_preset     new value of preset.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_preset(buf, sdp->addr, presn, va_preset)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Set value of program item.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @param progn Number of program item to set.
 * @param program       New value of program item.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_set_program(const sdp_t *sdp, int progn, const sdp_program_t *program)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sset_program(buf, sdp->addr, progn, program)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

/**
 * Stop running program.
 * @param sdp   Pointer to sdp_t structure, initialized by sdp_open.
 * @return      On success 0, on error negative number (error no.).
 */
int sdp_stop(const sdp_t *sdp)
{
        char buf[SDP_BUF_SIZE_MIN];
        int ret;

        if ( (ret = sdp_sstop(buf, sdp->addr)) < 0)
                return ret;

        if ( (ret = sdp_write(sdp->f_out, buf, ret)) < 0)
                return ret;

        if ( (ret = sdp_read_resp(sdp->f_in, buf, SDP_RESP_LEN_OK)) < 0)
                return ret;

        return 0;
}

