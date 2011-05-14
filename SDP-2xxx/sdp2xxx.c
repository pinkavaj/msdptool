#include "sdp2xxx.h"
#include <string.h>
#include <strings.h>

/**
 * SDP command templates, see SDP power supply manual for more details
 * about meaning of terms listed below.
 * __   - device address
 * i    - number, current
 * l    - number, location
 * u    - number, voltage
 * m    - number, minutes
 * s    - number, seconds
 * n    - number, count (number of repeats)
 */
/* NOTE: there might be performed some optimalisation by replacing mnemonics
 * defined above with '0', but i preffer this for clarity. */
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
static const char sdp_cmd_getp_prog[] = "GETP__pp\r";
static const char sdp_cmd_gpal[] = "GPAL__\r";
static const char sdp_cmd_volt[] = "VOLT__uuu\r";
static const char sdp_cmd_curr[] = "CURR__iii\r";
static const char sdp_cmd_sovp[] = "SOVP__uuu\r";
static const char sdp_cmd_sout_dis[] = "SOUT__1\r";
static const char sdp_cmd_sout_en[] = "SOUT__0\r";
static const char sdp_cmd_poww_dis[] = "POWW__i1\r";
static const char sdp_cmd_poww_en[] = "POWW__i0\r";
static const char sdp_cmd_prom[] = "PROM__luuuiii\r";
static const char sdp_cmd_prop[] = "PROP__lluuuiiimmss\r";
static const char sdp_cmd_runm[] = "RUNM__l\r";
static const char sdp_cmd_runp[] = "RUNP__nnn\r";
static const char sdp_cmd_stop[] = "STOP__\r";

static const char str_ok[] = "OK\r";

/**
 * print SDP command into buffer and fill device address
 *
 * buf  - buffer where to write SDP device command
 *      mus have size at least SDP_BUF_SIZE_MIN
 * cmd  - template of command wich should be printed
 * addr - rs485 address of device in range 1 - 31, for
 *      rs232 use anny number in this range
 * returns -1 on error, 0 otherwise - FIXME: think more about this
 */
static int sdp_print_cmd(char *buf, const char *cmd, int addr)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX)
                return -1;
       
        strncpy(buf, cmd, SDP_BUF_SIZE_MIN);
        // write SDP addres into cmd template ("ttttAA...\r")
        buf[5] = (addr % 10) + '0';
        addr /= 10;
        buf[4] = (addr % 10) + '0';

        return 0;
}

/**
 * check whathever data in buffer contains completed response on SDP command
 *
 * buf  - buffer containing recieved response
 * len  - lenght of data in buffer
 * returns response status (nodata, data, incomplete)
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
 * Parse response for sdp_get_dev_addr. When device is connected on
 *      rs485 bus, this function might be used to detect whatever is device
 *      with specified address online, This function has no meaning on rs232,
 *      but might be used to to get last rs485 device address.
 *
 * buf  - buffer with data from response
 * len  - lenght of data in buffer
 * addr - pointer to integer to store recieved addres
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_resp_dev_addr(char *buf, int len, int *addr)
{
        if (len != (sizeof(str_ok) - 1 + 3))
                return -1;

        if (buf[0] != '0' && buf[1] != '0')
                return -1;

        *addr = buf[2] - '0';

        return 0;
}

/**
 * Enable or disable remote control of SDP power supply.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * enable       - when 0 disable remote control, enable otherwise
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_remote(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sess, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_ends, addr);
}

/**
 * Recall preset values from memory
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * preset       - index of preset values: 1-9
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_run_preset(char *buf, int addr, int preset)
{
        if (preset < SDP_PRESET_MIN || preset > SDP_PRESET_MAX)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_runm, addr) == -1)
                return -1;

        buf[6] = ((char)preset % 10) + '0';

        return 0;
}

/**
 * Run timmed program.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * count        - number of program repeats: 0-999 or SDP_RUN_PROG_INF
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_run_program(char *buf, int addr, int count)
{
        if ((count < 1 || count > 999) && count != SDP_RUN_PROG_INF)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_runp, addr) == -1)
                return -1;

        buf[8] = (count % 10) + '0';
        count /= 10;
        buf[7] = (count % 10) + '0';
        count /= 10;
        buf[6] = (count % 10) + '0';
 
        return 0;
}

/**
 * Select interface used for comunication with power supply (rs232/rs485)
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * ifce - selected interface type
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_select_ifce(char *buf, int addr, sdp_ifce_t ifce)
{
        if (ifce == sdp_ifce_rs232)
                return sdp_print_cmd(buf, sdp_cmd_ccom_rs232, addr);
        if (ifce == sdp_ifce_rs485)
                return sdp_print_cmd(buf, sdp_cmd_ccom_rs485, addr);

        return -1;
}

/**
 * Set output voltage to specified value.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * volt - voltage level value (TODO: units)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_volt(char *buf, int addr, int volt)
{
        if (sdp_print_cmd(buf, sdp_cmd_volt, addr) == -1)
                return -1;

        // TODO
        return -1;
}

/**
 * Set current level to specified value.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * curr - current level value
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_curr(char *buf, int addr, int curr)
{
        if (sdp_print_cmd(buf, sdp_cmd_curr, addr) == -1)
                return -1;

        // TODO
        return -1;
}

/**
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_volt_limit(char *buf, int addr, int volt)
{
        if (sdp_print_cmd(buf, sdp_cmd_sovp, addr) == -1)
                return -1;

        // TODO
        return -1;
}

/**
 * Enable or disable power source output.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 * enable       - when 0 set output to off, set to on otherwise
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_output(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sout_en, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_sout_dis, addr);
}

/**
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_poweron_output(char *buf, int addr, int preset, int enable)
{
        // TODO set preset
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_poww_en, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_poww_dis, addr);

        // TODO
        return -1;
}

/**
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_preset(char *buf, int addr, int preset, int volt, int curr)
{
        if (sdp_print_cmd(buf, sdp_cmd_prom, addr) == -1)
                return -1;

        // TODO
        return -1;
}

/**
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_set_program(char *buf, int addr, int program, int volt, int curr,
                int time)
{
        if (sdp_print_cmd(buf, sdp_cmd_prop, addr) == -1)
                return -1;

        // TODO
        return -1;
}

/**
 * Stop timed program.
 *
 * buf  - buffer where to write command (see SDP_BUF_SIZE_MIN)
 * addr - address of device (used only for devices connected with rs485)
 *
 * returns 0 when all fine or -1 on error
 */
int sdp_stop(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_stop, addr);
}

