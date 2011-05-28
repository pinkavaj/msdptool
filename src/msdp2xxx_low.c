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

#include "msdp2xxx_low.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>


#define SDP_INT2VOLT(u) (((double)(u)) / 10)
#define SDP_INT2CURR(i) (((double)(i)) / 100)
#define SDP_VOLT2INT(x) ((int)round((x) * 10))
#define SDP_CURR2INT(x) ((int)round((x) * 100))

/*
 * SDP command templates, see SDP power supply manual for more details about
 *      meaning of terms below.
 * __   - number, device address
 * i    - number, current
 * l    - number, location (position in program or preset value number)
 * u    - number, voltage
 * m    - number, minutes
 * s    - number, seconds
 * n    - number, count (number of repeats)

 * NOTE: there might be performed some optimalisation by replacing mnemonics
 * defined above with '0', but I preffer this for clarity. */
static const char sdp_cmd_sess[] = "SESS__\r";
static const char sdp_cmd_ends[] = "ENDS__\r";
static const char sdp_cmd_ccom_rs232[] = "CCOM__000\r";
static const char sdp_cmd_ccom_rs485[] = "CCOM__001\r";
static const char sdp_cmd_gcom[] = "GCOM__\r";
static const char sdp_cmd_gmax[] = "GMAX__\r";
static const char sdp_cmd_govp[] = "GOVP__\r";
static const char sdp_cmd_getd[] = "GETD__\r";
static const char sdp_cmd_gets[] = "GETS__\r";
static const char sdp_cmd_getm[] = "GETM__\r";
static const char sdp_cmd_getm_loc[] = "GETM__l\r";
static const char sdp_cmd_getp[] = "GETP__\r";
static const char sdp_cmd_getp_prog[] = "GETP__ll\r";
static const char sdp_cmd_gpal[] = "GPAL__\r";
static const char sdp_cmd_volt[] = "VOLT__uuu\r";
static const char sdp_cmd_curr[] = "CURR__iii\r";
static const char sdp_cmd_sovp[] = "SOVP__uuu\r";
static const char sdp_cmd_sout_dis[] = "SOUT__1\r";
static const char sdp_cmd_sout_en[] = "SOUT__0\r";
static const char sdp_cmd_poww_dis[] = "POWW__ll1\r";
static const char sdp_cmd_poww_en[] = "POWW__ll0\r";
static const char sdp_cmd_prom[] = "PROM__luuuiii\r";
static const char sdp_cmd_prop[] = "PROP__lluuuiiimmss\r";
static const char sdp_cmd_runm[] = "RUNM__l\r";
static const char sdp_cmd_runp[] = "RUNP__nnnn\r";
static const char sdp_cmd_stop[] = "STOP__\r";

static const char str_ok[] = "OK\r";

#ifdef _MSVC
/**
 * Rounds number usign common rounding rules, there is missing of round
 *      function in MSVC.
 * @param d     Value to round.
 * @return      Number rounded to nearest integer.
 */
inline double round(double d)
{
        return floor(d + 0.5);
}
#endif

/**
 * Print 3, 2 or 1 digit unsigned integer into buffer.
 * @param buf   Output buffer, mus be at least lenB long.
 * @param len   Count of printed digits.
 * @param num   Value to print.
 * @return      Pointer to buf.
 */
static char *sdp_print_num(char *buf, int len, int num)
{
        switch (len) {
                case 3:
                        *buf++ = (num / 100) + '0';
                        num %= 100;
                case 2:
                        *buf++ = (num / 10) + '0';
                        num %= 10;
                case 1:
                        *buf++ = num + '0';
                default:
                        break;
        }

        return buf - len;
}

/**
 * Convert numeric string into number.
 * @param buf   Buffer with character data.
 * @param len   Number of digits.
 * @param val   Pointer to integet to store result.
 * @return      0 on success, negative number (err no.) on error.
 */
static int sdp_scan_num(const char *buf, int len, int *val)
{
        *val = 0;
        while (len--) {
                if (!isdigit(buf[0])) {
                        errno = EINVAL;
                        return SDP_ENONUM;
                }
                *val *= 10;
                *val += buf[0] - '0';
                buf++;
        }
        return 0;
}

/**
 * Copy SDP command from template into buffer and fill device address.
 * @param buf   Output buffer, mus have size at least SDP_BUF_SIZE_MIN.
 * @param cmd   template of command wich should be printed.
 * @param addr  RS485 address of device in range 1-31, for device connected on
 *      RS232 use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
static int sdp_print_cmd(char *buf, const char *cmd, int addr)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX) {
                errno = ERANGE;
                return SDP_ERANGE;
        }
       
        strcpy(buf, cmd);
        sdp_print_num(buf + 4, 2, addr);

        return strlen(cmd);
}

/**
 * Request to get devices RS485 address, might be used to detect whatever is
 *      device with specified address available.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address in range 1-31, for RS232 connected
 *      devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_dev_addr(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_gcom, addr);
}

/**
 * Request to get all informations shown on PS LCD screen.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_lcd_info(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_gpal, addr);
}

/**
 * Request to get actual (measured) value of output voltage and current.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_va_data(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_getd, addr);
}

/**
 * Request to get PS maximal U and I output values.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_va_maximums(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_gmax, addr);
}

/**
 * Request to get current setpoint (target U and I).
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_va_setpoint(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_gets, addr);
}

/**
 * Request to get one or all of U, I presets stored in device.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @param presn preset number in range 1-9 to get or SDP_PRESET_ALL to gel
 *      all presets.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_preset(char *buf, int addr, int presn)
{
        int ret;

        if (presn == SDP_PRESET_ALL)
                return sdp_print_cmd(buf, sdp_cmd_getm, addr);

        if (presn < SDP_PRESET_MIN || presn > SDP_PRESET_MAX) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        if ( (ret = sdp_print_cmd(buf, sdp_cmd_getm_loc, addr)) < 0)
                return ret;

        buf[6] = presn + '0';

        return ret;
}

/**
 * Request to get one or all timed program items.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @param progn program number in range 0-19 to get or SDP_PROGRAM_ALL to get
 *      all program items.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_program(char *buf, int addr, int progn)
{
        int ret;

        if (progn == SDP_PROGRAM_ALL)
                return sdp_print_cmd(buf, sdp_cmd_getp, addr);

        if (progn < SDP_PROGRAM_MIN || progn > SDP_PROGRAM_MAX) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        if ( (ret = sdp_print_cmd(buf, sdp_cmd_getp_prog, addr)) < 0)
                return ret;

        sdp_print_num(buf + 6, 2, progn);

        return ret;
}

/**
 * Request to get upper voltage limit.
 * @param buf   Output buffer, must have size at least SDP_BUF_SIZE_MIN.
 * @param addr  RS485 device address, number in range 1-31, for RS232
 *      connected devices is ignored, use anny number in range 1-31.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sget_volt_limit(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_govp, addr);
}

/**
 * Return textual description of SDP error, or value of strerror if
 *      system level error occured.
 * @param err   Error number.
 * @return      Pointer to string describing the error.
 */
const char *sdp_strerror(int err)
{
        switch(err) {
                case SDP_EINRES:
                        return "Recieved invalid response.";
                case SDP_ERANGE:
                        return "Function parameter is out of range.";
                case SDP_ENONUM:
                        return "Value in response is not number.";
                case SDP_ETIMEDOUT:
                        return "Device not responded in specified time.";
                case SDP_ETOLARGE:
                        return "Output is too large to fit in buffer.";
                case SDP_EWINCOMPL:
                        return "Error occured during sending message to device.";
                case SDP_EOK:
                        errno = 0;
                case SDP_EERRNO:
                default:
                        break;
        }

        return strerror(errno);
}

/**
 * Check buffer content for completed response on SDP command.
 * @param buf   Buffer containing recieved response.
 * @param len   Lenght of data in buffer.
 * @return      Response status (nodata, data, incomplete).
 */
sdp_resp_t sdp_resp(const char *buf, int len)
{
        if (len < 3)
                return sdp_resp_incomplete;
        if (!strncmp(buf + len - 3, str_ok, 3)) {
                /* response contains only OK skring */
                if (len == 3)
                        return sdp_resp_nodata;
                /* response contains some data and OK string */
                if (buf[len - 4] == '\r')
                        return sdp_resp_data;
        }
        return sdp_resp_incomplete;
}

/**
 * Parse response on sdp_sget_dev_addr. When device is connected on
 *      RS485 bus, this function might be used to check for presence of device
 *      with specified address. This function has no meaning on RS232,
 *      but might be used to get last RS485 device address (unsupported).
 * @param buf   buffer with recieved response.
 * @param len   lenght of data in buffer.
 * @param addr  pointer to integer to store recieved addres.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_dev_addr(char *buf, int len, int *addr)
{       
        const char resp[] = "___\rOK\r";
        const int resp_len = sizeof(resp) - 1;

        if (len != resp_len) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        if (memcmp(resp + 3, buf + 3, resp_len - 3)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        return sdp_scan_num(buf + 1, 2, addr);
}

/**
 * Parse response on sdp_sget_va_maximums.
 * @param buf           Buffer with irecieved response.
 * @param len           Lenght of data in buffer.
 * @param va_maximums   Pointer to sdp_va_t where result should be stored.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_va_maximums(char *buf, int len, sdp_va_t *va_maximums)
{
        const char resp[] = "uuuiii\rOK\r";
        int ret, val;

        if (len != (sizeof(resp) - 1)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                return ret;
        va_maximums->volt = SDP_INT2VOLT(val);

        if ( (ret = sdp_scan_num(buf + 3, 3, &val)) < 0)
                return ret;
        va_maximums->curr = SDP_INT2CURR(val);
        
        return 0;
}

/**
 * Parse response on sdp_sget_volt_limit.
 * @param buf   Buffer with irecieved response.
 * @param len   Lenght of data in buffer.
 * @param volt_limit    Pointer to int where recieved value should be stored.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_volt_limit(char *buf, int len, double *volt_limit)
{
        const char resp[] = "uuu\rOK\r";
        int ret, val;

        if (len != (sizeof(resp) - 1)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        if ( (ret = sdp_scan_num(buf, 3, &val)) >= 0)
                *volt_limit = SDP_INT2VOLT(val);

        return ret;
}

/**
 * Parse response on sdp_sget_va_data.
 * @param buf   buffer with irecieved response.
 * @param len   lenght of data in buffer.
 * @param va_data       pointer to sdp_va_data_t to store current U, I and mode.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_va_data(char *buf, int len, sdp_va_data_t *va_data)
{
        const char resp[] = "uuuuiiiic\rOK\r";
        int mode, ret, val;

        if (len != (sizeof(resp) - 1)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        if ( (ret = sdp_scan_num(buf, 4, &val)) < 0)
                return ret;
        // Data from measurement have one more decimal point
        va_data->volt = SDP_INT2VOLT(val) / 10;

        if ( (ret = sdp_scan_num(buf + 4, 4, &val)) < 0)
                return ret;
        va_data->curr = SDP_INT2CURR(val) / 10;

        if ( (ret = sdp_scan_num(buf + 8, 1, &mode)) < 0)
                return ret;
        va_data->mode = (sdp_mode_t)mode;

        return 0;
}

/**
 * Parse response on sdp_sget_va_setpoint.
 * @param buf   buffer with irecieved response.
 * @param len   lenght of data in buffer.
 * @param va_setpoints  pointer to sdp_va_t used to store current setpoint.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_va_setpoint(char *buf, int len, sdp_va_t *va_setpoints)
{
        const char resp[] = "uuuiii\rOK\r";
        int ret, val;

        if (len != (sizeof(resp) - 1)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                return ret;
        va_setpoints->volt = SDP_INT2VOLT(val);

        if ( (ret = sdp_scan_num(buf + 3, 3, &val)) < 0)
                return ret;
        va_setpoints->curr = SDP_INT2CURR(val);

        return 0;
}

/**
 * Parse response on sdp_sget_preset.
 * @param buf   Buffer with irecieved response.
 * @param len   Lenght of data in buffer.
 * @param va_preset     When sdp_sget_preset called with preset number expects pointer
 *      to sdp_va_t to store value from requested preset. 
 *      When sdp_sget_preset called with SDP_PRESET_ALL expects pointer to
 *      sdp_va_t array of size 9 to store all presets values.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_preset(char *buf, int len, sdp_va_t *va_preset)
{
        const char resp[] = "uuuiii\r";
        const int resp_s1 = sizeof(resp) - 1 + sizeof(str_ok) - 1;
        const int resp_s9 = (sizeof(resp) - 1) * 9 + sizeof(str_ok) - 1;
        int count, ret, val;

        if (len == resp_s9) {
                count = 9;
        } else {
                if (len != resp_s1) {
                        errno = EINVAL;
                        return SDP_EINRES;
                }
                count = 1;
        }

        while (count--) {
                if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                        return ret;
                va_preset->volt = SDP_INT2VOLT(val);
                buf += 3;

                if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                        return ret;
                va_preset->curr = SDP_INT2CURR(val);
                buf += 3;

                va_preset++;
                buf++;
        }

        return 0;
}

/**
 * Parse response on sdp_sget_program.
 * @param buf   buffer with irecieved response.
 * @param len   lenght of data in buffer.
 * @param program       When sdp_sget_program called with program number
 *      expects pointer to sdp_program_t to store requested program item.
 *      When sdp_sget_program called with SDP_PROGRAM_ALL expects pointer to
 *      array of sdp_program_t of size 20 to store all program items.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_program(char *buf, int len, sdp_program_t *program)
{
        const char resp[] = "uuuiiimmss\r";
        const int resp_s1 = sizeof(resp) - 1 + sizeof(str_ok) - 1;
        const int resp_s20 = (sizeof(resp) - 1) * 20 + sizeof(str_ok) - 1;
        int count, ret, val;

        if (len == resp_s20) {
                count = 20;
        } else {
                if (len != resp_s1) {
                        errno = EINVAL;
                        return SDP_EINRES;
                }
                count = 1;
        }

        while (count--) {
                int min, sec;

                if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                        return ret;
                program->volt = SDP_INT2VOLT(val);
                buf += 3;

                if ( (ret = sdp_scan_num(buf, 3, &val)) < 0)
                        return ret;
                program->curr = SDP_INT2CURR(val);
                buf += 3;

                if ( (ret = sdp_scan_num(buf, 2, &min)) < 0)
                        return ret;
                buf += 2;

                if ( (ret = sdp_scan_num(buf, 2, &sec)) < 0)
                        return ret;
                buf += 2;
                program->time = min * 60 + sec;

                program++;
                buf++;
        }

        return 0;
}

/**
 * Parse response on sdp_sget_lcd_info.
 * @param buf   buffer with irecieved response.
 * @param len   lenght of data in buffer.
 * @param lcd_info      pointer to sdp_lcd_info_raw_t to store recieved data.
 * @return      0 on success, negative number (err no.) on error.
 */
int sdp_resp_lcd_info(char *buf, int len, sdp_lcd_info_raw_t *lcd_info)
{
        const char resp[] = "UUUUUUUUVIIIIIIIIAPPPPPPPPWmmmmssss____uuuuuu___iiiiii___pp_________\rOK\r";
        int idx;

        if (len != (sizeof(resp) - 1)) {
                errno = EINVAL;
                return SDP_EINRES;
        }

        // convert from "ASCII" to binary, (data are in lower nibble)
        for (idx = 0; idx < (sizeof(resp) - 4 - 1); idx++)
                buf[idx] &= 0x0f;

        lcd_info->read_V[0] = (buf[0] << 4) | buf[1];
        lcd_info->read_V[1] = (buf[2] << 4) | buf[3];
        lcd_info->read_V[2] = (buf[4] << 4) | buf[5];
        lcd_info->read_V[3] = (buf[6] << 4) | buf[7];
        lcd_info->read_V_ind = !buf[8];
        lcd_info->read_A[0] = (buf[9] << 4) | buf[10];
        lcd_info->read_A[1] = (buf[11] << 4) | buf[12];
        lcd_info->read_A[2] = (buf[13] << 4) | buf[14];
        lcd_info->read_A[3] = (buf[15] << 4) | buf[16];
        lcd_info->read_A_ind = !buf[17];
        lcd_info->read_W[0] = (buf[18] << 4) | buf[19];
        lcd_info->read_W[1] = (buf[20] << 4) | buf[21];
        lcd_info->read_W[2] = (buf[22] << 4) | buf[23];
        lcd_info->read_W[3] = (buf[24] << 4) | buf[25];
        lcd_info->read_W_ind = !buf[26];
        lcd_info->time[0] = (buf[27] << 4) | buf[28];
        lcd_info->time[1] = (buf[29] << 4) | buf[30];
        lcd_info->time[2] = (buf[31] << 4) | buf[32];
        lcd_info->time[3] = (buf[33] << 4) | buf[34];
        lcd_info->timer_ind = !buf[35];
        lcd_info->colon_ind = !buf[36];
        lcd_info->m_ind = !buf[37];
        lcd_info->s_ind = !buf[38];
        lcd_info->set_V[0] = (buf[39] << 4) | buf[40];
        lcd_info->set_V[1] = (buf[41] << 4) | buf[42];
        lcd_info->set_V[2] = (buf[43] << 4) | buf[44];
        lcd_info->set_V_const = !buf[45];
        lcd_info->set_V_bar = !buf[46];
        lcd_info->set_V_ind = !buf[47];
        lcd_info->set_A[0] = (buf[48] << 4) | buf[49];
        lcd_info->set_A[1] = (buf[50] << 4) | buf[51];
        lcd_info->set_A[2] = (buf[52] << 4) | buf[53];
        lcd_info->set_A_const = !buf[54];
        lcd_info->set_A_bar = !buf[55];
        lcd_info->set_A_ind = !buf[56];
        lcd_info->prog = (buf[57] << 4) | buf[58];
        lcd_info->prog_on = !buf[59];
        lcd_info->prog_bar = !buf[60];
        lcd_info->setting_ind = !buf[61];
        lcd_info->key_lock = !buf[62];
        lcd_info->key_open = !buf[63];
        lcd_info->fault_ind = !buf[64];
        lcd_info->output_on = !buf[65];
        lcd_info->output_off = !buf[66];
        lcd_info->remote_ind = !buf[67];

        return 0;
}


/**
 * Enable or disable remote control of SDP power supply.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address  1-31 (use anny valid for RS232).
 * @param enable        when 0 disable remote control, enable otherwise.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sremote(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sess, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_ends, addr);
}

/**
 * Recall preset values from memory.
 * @param buf   Output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address  1-31 (use anny valid for RS232).
 * @param presn Preset number to recall 1-9.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_srun_preset(char *buf, int addr, int presn)
{
        int ret;

        if (presn < SDP_PRESET_MIN || presn > SDP_PRESET_MAX) {
                errno = ERANGE;
                return SDP_EINRES;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_runm, addr);
        if (ret >= 0)
                buf[6] = ((char)presn % 10) + '0';

        return ret;
}

/**
 * Run timed program.
 * @param buf   Output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param count Number of program repeats: 0-999 or SDP_RUN_PROG_INF.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_srun_program(char *buf, int addr, int count)
{
        int ret;

        if ((count < 1 || count > 9999) && count != SDP_RUN_PROG_INF) {
                errno = ERANGE;
                return SDP_EINRES;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_runp, addr);
        if (ret >= 0)
                sdp_print_num(buf + 6, 4, count);
 
        return ret;
}

/**
 * Select interface used for comunication with power supply (RS232/RS485).
 * @param buf   Output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param ifce  Selected interface type.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sselect_ifce(char *buf, int addr, sdp_ifce_t ifce)
{
        if (ifce == sdp_ifce_rs232)
                return sdp_print_cmd(buf, sdp_cmd_ccom_rs232, addr);
        if (ifce == sdp_ifce_rs485)
                return sdp_print_cmd(buf, sdp_cmd_ccom_rs485, addr);

        errno = ERANGE;
        return SDP_ERANGE;
}

/**
 * Set output voltage.
 * @param buf   Output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param volt  Voltage level value.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_volt(char *buf, int addr, double volt)
{
        int ret, val;

        val = SDP_VOLT2INT(volt);
        if (val < 0 || val > 999) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_volt, addr);
        if (ret >= 0)
                sdp_print_num(buf + 6, 3, val);
        
        return ret;
}

/**
 * Set output current.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param curr  current level value.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_curr(char *buf, int addr, double curr)
{
        int ret, val;

        val = SDP_CURR2INT(curr);
        if (val < 0 || val > 999) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_curr, addr);
        if (ret >= 0)
                sdp_print_num(buf + 6, 3, val);

        return ret;
}

/**
 * Set upper voltage limit.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param volt  voltage limit [V].
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_volt_limit(char *buf, int addr, double volt)
{
        int ret, val;

        val = SDP_VOLT2INT(volt);
        if (val < 0 || val > 999) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_sovp, addr);
        if (ret >= 0)
                sdp_print_num(buf + 6, 3, val);

        return ret;
}

/**
 * Enable or disable power source output.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param addr  address of device (used only for devices connected on RS485).
 * @param enable        when 0 set output to off, set to on otherwise.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_output(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sout_en, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_sout_dis, addr);
}

/**
 * Set output power on status for specified preset.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param presn preset number: 1-9.
 * @param enable        When 0 disable, oneble oterwise.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_poweron_output(char *buf, int addr, int presn, int enable)
{
        int ret;

        if (presn < SDP_PRESET_MIN || presn > SDP_PRESET_MAX) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        if (enable)
                ret = sdp_print_cmd(buf, sdp_cmd_poww_en, addr);
        else
                ret = sdp_print_cmd(buf, sdp_cmd_poww_dis, addr);

        if (ret >= 0)
                sdp_print_num(buf + 6, 2, presn);
        
        return ret;
}

/**
 * Set preset values in memory.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param presn number of preset to set: 1-9.
 * @param va_preset     pointer to sdp_va_t containign values to be set.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_preset(char *buf, int addr, int presn, const sdp_va_t *va_preset)
{
        int ret;
        int volt, curr;

        volt = SDP_VOLT2INT(va_preset->volt);
        curr = SDP_CURR2INT(va_preset->curr);
        if (presn < SDP_PRESET_MIN || presn > SDP_PRESET_MAX ||
                        volt < 0 || volt > 999 || 
                        curr < 0 || curr > 999) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_prom, addr);
        if (ret >= 0) {
                buf[6] = presn + '0';
                sdp_print_num(buf + 7, 3, volt);
                sdp_print_num(buf + 10, 3, curr);
        }

        return ret;
}

/**
 * Set program item to specified values.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @param progn program number for which values should be set: 0-19.
 * @param program       pointer to sdp_program_t containing new program item values.
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sset_program(char *buf, int addr, int progn, const sdp_program_t *program)
{
        int ret;
        int volt, curr;

        volt = SDP_VOLT2INT(program->volt);
        curr = SDP_CURR2INT(program->curr);

        if (progn < SDP_PROGRAM_MIN || progn > SDP_PROGRAM_MAX ||
                        volt < 0 || volt > 999 || 
                        curr < 0 || curr > 999 ||
                        program->time < 0 || program->time > (99*60+59)) {
                errno = ERANGE;
                return SDP_ERANGE;
        }

        ret = sdp_print_cmd(buf, sdp_cmd_prop, addr);
        if (ret >= 0) {
                sdp_print_num(buf + 6, 2, progn);
                sdp_print_num(buf + 8, 3, volt);
                sdp_print_num(buf + 11, 3, curr);
                sdp_print_num(buf + 14, 2, program->time / 60);
                sdp_print_num(buf + 16, 2, program->time % 60);
        }
        
        return ret;
}

/**
 * Stop timed program.
 * @param buf   output buffer (see SDP_BUF_SIZE_MIN).
 * @param addr  RS485 device address: 1-31 (use anny valid for RS232).
 * @return      Number of characters writen, not including trailing '\0',
 *      or negative number (error no.) on error.
 */
int sdp_sstop(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_stop, addr);
}

/**
 * Convert LCD coded number into integet, ignote top bit mostly used as dot.
 * @param lcd_num       LCD coded number.
 * @return      Number value (integer) or -1 on error.
 */
static int lcd_bcd(unsigned char lcd_num)
{
        int idx;
        // codes representing numbers 0-9 is coded by LCD LED segments
        const unsigned char lcd_nums[] = {
                // 0     1     2     3     4     5     6     7      8     9
                0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x07f, 0x6f,
        };

        if (!lcd_num)
                return 0;

        for (idx = 0; idx < 10; idx++) {
                if (lcd_nums[idx] == (lcd_num & 0x7f))
                        return idx;
        }

        return -1;
}

/**
 * Convert raw data from LCD registry dump into its numerical representation.
 * lcd_info     Pointer to sdp_lcd_info_t, used to store conversion result.
 * lcd_info_raw Pointer to sdp_lcd_info_raw_t, LCD registry dump.
 */
void sdp_lcd_to_data(sdp_lcd_info_t *lcd_info,
                const sdp_lcd_info_raw_t *lcd_info_raw)
{
        // FIXME: check for decimal point
        lcd_info->read_V = (lcd_bcd(lcd_info_raw->read_V[0]) * 1000 +
                        lcd_bcd(lcd_info_raw->read_V[1]) * 100 +
                        lcd_bcd(lcd_info_raw->read_V[2]) * 10 +
                        lcd_bcd(lcd_info_raw->read_V[3])) / 100.;
        lcd_info->read_V_ind = lcd_info_raw->read_V_ind;

        lcd_info->read_A = (lcd_bcd(lcd_info_raw->read_A[0]) * 1000 +
                        lcd_bcd(lcd_info_raw->read_A[1]) * 100 +
                        lcd_bcd(lcd_info_raw->read_A[2]) * 10 +
                        lcd_bcd(lcd_info_raw->read_A[3])) / 1000.;
        lcd_info->read_A_ind = lcd_info_raw->read_A_ind;

        lcd_info->read_W = (lcd_bcd(lcd_info_raw->read_W[0]) * 1000 +
                        lcd_bcd(lcd_info_raw->read_W[1]) * 100 +
                        lcd_bcd(lcd_info_raw->read_W[2]) * 10 +
                        lcd_bcd(lcd_info_raw->read_W[3])) / 100.;
        lcd_info->read_W_ind = lcd_info_raw->read_W_ind;

        lcd_info->time = lcd_bcd(lcd_info_raw->time[0]) * 600 +
                lcd_bcd(lcd_info_raw->time[1]) * 60 +
                lcd_bcd(lcd_info_raw->time[2]) * 10 +
                lcd_bcd(lcd_info_raw->time[3]);
        lcd_info->timer_ind = lcd_info_raw->timer_ind;
        lcd_info->colon_ind = lcd_info_raw->colon_ind;
        lcd_info->m_ind = lcd_info_raw->m_ind;
        lcd_info->s_ind = lcd_info_raw->s_ind;

        lcd_info->set_V = (lcd_bcd(lcd_info_raw->set_V[0]) * 100 +
                        lcd_bcd(lcd_info_raw->set_V[1]) * 10 +
                        lcd_bcd(lcd_info_raw->set_V[2])) / 10.;
        lcd_info->set_V_const = lcd_info_raw->set_V_const;
        lcd_info->set_V_bar = lcd_info_raw->set_V_bar;
        lcd_info->set_V_ind = lcd_info_raw->set_V_ind;

        lcd_info->set_A = (lcd_bcd(lcd_info_raw->set_A[0]) * 100 +
                        lcd_bcd(lcd_info_raw->set_A[1]) * 10 +
                        lcd_bcd(lcd_info_raw->set_A[2])) / 100.;
        lcd_info->set_A_const = lcd_info_raw->set_A_const;
        lcd_info->set_A_bar = lcd_info_raw->set_A_bar;
        lcd_info->set_A_ind = lcd_info_raw->set_A_ind;

        lcd_info->prog = lcd_bcd(lcd_info_raw->prog);
        lcd_info->prog_on = lcd_info_raw->prog_on;
        lcd_info->prog_bar = lcd_info_raw->prog_bar;
        lcd_info->setting_ind = lcd_info_raw->setting_ind;
        //key_lock;
        lcd_info->key = lcd_info_raw->key_open;
        lcd_info->fault_ind = lcd_info_raw->fault_ind;
        lcd_info->output = lcd_info_raw->output_on;
        //output_off;
        lcd_info->remote_ind = lcd_info_raw->remote_ind;
}
