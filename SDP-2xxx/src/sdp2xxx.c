#include "sdp2xxx.h"
#include <string.h>
#include <strings.h>

/**
 * SDP command templates, see SDP power supply manual for more details about
 *      meaning of terms below.
 * __   - number, device address
 * i    - number, current
 * l    - number, location (position in program or preset value number)
 * u    - number, voltage
 * m    - number, minutes
 * s    - number, seconds
 * n    - number, count (number of repeats)
 */
/* NOTE: there might be performed some optimalisation by replacing mnemonics
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
static const char sdp_cmd_poww_dis[] = "POWW__i1\r";
static const char sdp_cmd_poww_en[] = "POWW__i0\r";
static const char sdp_cmd_prom[] = "PROM__luuuiii\r";
static const char sdp_cmd_prop[] = "PROP__lluuuiiimmss\r";
static const char sdp_cmd_runm[] = "RUNM__l\r";
static const char sdp_cmd_runp[] = "RUNP__nnn\r";
static const char sdp_cmd_stop[] = "STOP__\r";

static const char str_ok[] = "OK\r";

/**
 * Print 2 digits unsigned number into buffer.
 *
 * buf:         output buffer, mus be at least 2B long
 * num:         number to write, must be in range 0-99
 * returns:     pointer to buf
 */
static char *sdp_print_num2(char *buf, int num)
{
        buf[1] = (num % 10) + '0';
        num /= 10;
        buf[0] = (num % 10) + '0';

        return buf;
}

/**
 * Print 3 digits unsigned number into buffer.
 *
 * buf:         output buffer, mus be at least 3B long
 * num:         number to write, must be in range 0-999
 * returns:     pointer to buf
 */
static char *sdp_print_num3(char *buf, int num)
{
        buf[2] = (num % 10) + '0';
        num /= 10;
        sdp_print_num2(buf + 1, num);

        return buf;
}

/**
 * Copy SDP command from template into buffer and fill device address.
 *
 * buf:         output buffer, mus have size at least SDP_BUF_SIZE_MIN
 * cmd:         template of command wich should be printed
 * addr:        rs485 address of device in range 1 - 31,
 *      for device connected on rs232 use anny number in range 1 - 31
 * returns:     -1 on error, 0 otherwise
 */
static int sdp_print_cmd(char *buf, const char *cmd, int addr)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX)
                return -1;
       
        strcpy(buf, cmd);
        sdp_print_num2(buf + 4, addr);

        return 0;
}

/**
 * Check buffer content for completed response on SDP command.
 *
 * buf:         buffer containing recieved response
 * len:         lenght of data in buffer
 * returns:     response status (nodata, data, incomplete)
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
 * Parse response on sdp_get_dev_addr. When device is connected on
 *      rs485 bus, this function might be used to check for presence of device
 *      with specified address. This function has no meaning on rs232,
 *      but might be used to get last rs485 device address (unsupported).
 *
 * buf:         buffer with recieved response
 * len:         lenght of data in buffer
 * addr:        pointer to integer to store recieved addres
 * returns:     0 on success, -1 on error
 */
int sdp_resp_dev_addr(char *buf, int len, int *addr)
{       
        const char resp[] = "RS__\rOK\r";

        if (len != (sizeof(resp) - 1))
                return -1;

        if (buf[0] != 'R' || buf[1] != 'S' ||
                        buf[2] < '0' || buf[2] > '9' ||
                        buf[3] < '0' || buf[3] > '9')
                return -1;

        *addr = (buf[2] - '0') * 10;
        *addr += buf[3] - '0';

        return 0;
}

/**
 * Parse response on sdp_get_va_maximums.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * va_maximums: pointer to sdp_va_t where result should be stored
 * returns:     0 on success, -1 on error
 */
int sdp_resp_va_maximums(char *buf, int len, sdp_va_t *va_maximums)
{
        return -1;
}

/**
 * Parse response on sdp_get_volt_limit.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * volt_limit:  pointer to int where upper voltage limit should be stored
 * returns:     0 on success, -1 on error
 */
int sdp_resp_volt_limit(char *buf, int len, int *volt_limit)
{
        return -1;
}

/**
 * Parse response on sdp_get_va_data.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * va_data:     pointer to sdp_va_t to store current U and I value
 * returns:     0 on success, -1 on error
 */
int sdp_resp_va_data(char *buf, int len, sdp_va_t *va_data)
{
        return -1;
}

/**
 * Parse response on sdp_get_va_setpoint.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * va_setpoints: pointer to sdp_va_t used to store current setpoint
 * returns:     0 on success, -1 on error
 */
int sdp_resp_va_setpoint(char *buf, int len, sdp_va_t *va_setpoints)
{
        return -1;
}

/**
 * Parse response on sdp_get_preset.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * va_preset:   When sdp_get_preset called with preset number expects pointer
 *      to sdp_preset_t to store value from requested preset. 
 *      When sdp_get_preset called with SDP_PRESET_ALL expects pointer to
 *      sdp_preset_t array of size 9 to store all presets values.
 * returns:     0 on success, -1 on error
 */
int sdp_resp_preset(char *buf, int len, sdp_preset_t *preset)
{
        return -1;
}

/**
 * Parse response on sdp_get_program.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * program:     When sdp_get_program called with program number expects
 *      pointer to sdp_program_t to store requested program item.
 *      When sdp_get_program called with SDP_PROGRAM_ALL expects pointer to
 *      array of sdp_program_t of size 20 to store all program items.
 * returns:     0 on success, -1 on error
 */
int sdp_resp_program(char *buf, int len, sdp_program_t *program)
{
        return -1;
}

/**
 * Parse response on sdp_get_ldc_info.
 *
 * buf:         buffer with irecieved response
 * len:         lenght of data in buffer
 * lcd_info:    pointer to sdp_ldc_info_t to store recieved LCD info
 * returns:     0 on success, -1 on error
 */
int sdp_resp_ldc_info(char *buf, int len, sdp_ldc_info_t *lcd_info)
{
        return -1;
}


/**
 * Enable or disable remote control of SDP power supply.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * enable:      when 0 disable remote control, enable otherwise
 * returns:     0 on success, -1 on error
 */
int sdp_remote(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sess, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_ends, addr);
}

/**
 * Recall preset values from memory.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * preset:      index of preset values: 1-9
 * returns:     0 on success, -1 on error
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
 * Run timed program.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * count:       number of program repeats: 0-999 or SDP_RUN_PROG_INF
 * returns:     0 on success, -1 on error
 */
int sdp_run_program(char *buf, int addr, int count)
{
        if ((count < 1 || count > 999) && count != SDP_RUN_PROG_INF)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_runp, addr) == -1)
                return -1;

        sdp_print_num3(buf + 6, count);
 
        return 0;
}

/**
 * Select interface used for comunication with power supply (rs232/rs485).
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * ifce:        selected interface type
 * returns:     0 on success, -1 on error
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
 * Set output voltage.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * volt:        voltage level value (TODO: units)
 * returns:     0 on success, -1 on error
 */
int sdp_set_volt(char *buf, int addr, int volt)
{
        if (volt < 0 || volt > 999)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_volt, addr) == -1)
                return -1;

        sdp_print_num3(buf + 6, volt);
        
        return 0;
}

/**
 * Set output current.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * curr:        current level value
 * returns:     0 on success, -1 on error
 */
int sdp_set_curr(char *buf, int addr, int curr)
{
        if (curr < 0 || curr > 999)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_curr, addr) == -1)
                return -1;

        sdp_print_num3(buf + 6, curr);

        return 0;
}

/**
 * Set upper voltage limit.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * volt:        voltage limit (TODO: units)
 * returns:     0 on success, -1 on error
 */
int sdp_set_volt_limit(char *buf, int addr, int volt)
{
        if (volt < 0 || volt > 999)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_sovp, addr) == -1)
                return -1;

        sdp_print_num3(buf + 6, volt);

        return 0;
}

/**
 * Enable or disable power source output.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * addr:        address of device (used only for devices connected with rs485)
 * enable:      when 0 set output to off, set to on otherwise
 * returns:     0 on success, -1 on error
 */
int sdp_set_output(char *buf, int addr, int enable)
{
        if (enable)
                return sdp_print_cmd(buf, sdp_cmd_sout_en, addr);
        else
                return sdp_print_cmd(buf, sdp_cmd_sout_dis, addr);
}

/**
 * Set output power on status for specified preset.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * preset:      preset number: 1-9
 * enable:      when 0 disable, oneble oterwise
 * returns:     0 on success, -1 on error
 */
int sdp_set_poweron_output(char *buf, int addr, int preset, int enable)
{
        int ret;

        if (preset < SDP_PRESET_MIN || preset > SDP_PRESET_MAX)
                return -1;

        if (enable)
                ret = sdp_print_cmd(buf, sdp_cmd_poww_en, addr);
        else
                ret = sdp_print_cmd(buf, sdp_cmd_poww_dis, addr);

        if (ret == -1)
                return -1;
        
        buf[6] = preset + '0';
        
        return 0;
}

/**
 * Set preset values in memory.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * preset:      index of preset: 1-9
 * volt:        preset voltage value: 0-999
 * curr:        preset current value: 0-999
 * returns:     0 on success, -1 on error
 */
int sdp_set_preset(char *buf, int addr, int preset, int volt, int curr)
{
        if (preset < SDP_PRESET_MIN || preset > SDP_PRESET_MAX ||
                        volt < 0 || volt > 999 || 
                        curr < 0 || curr > 999)
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_prom, addr) == -1)
                return -1;

        buf[6] = preset + '0';
        sdp_print_num3(buf + 7, volt);
        sdp_print_num3(buf + 7 + 3, curr);

        return 0;
}

/**
 * Set program item to specified values.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * program:     program item index for which values should be set: 0-19
 * volt:        new volage for specified program item: 0-999
 * curr:        new curret for specified program item: 0-999
 * time:        new lenght of program item duration in seconds: 0-5999
 * returns:     0 on success, -1 on error
 */
int sdp_set_program(char *buf, int addr, int program, int volt, int curr,
                int time)
{
        if (program < SDP_PROGRAM_MIN || program > SDP_PROGRAM_MAX ||
                        volt < 0 || volt > 999 || 
                        curr < 0 || curr > 999 ||
                        time < 0 || time > (59*60+59))
                return -1;

        if (sdp_print_cmd(buf, sdp_cmd_prop, addr) == -1)
                return -1;

        sdp_print_num2(buf + 6, program);
        sdp_print_num3(buf + 6 + 2, volt);
        sdp_print_num3(buf + 6 + 2 + 3, curr);
        sdp_print_num2(buf + 6 + 2 + 3 + 3 + 2, time % 60);
        sdp_print_num2(buf + 6 + 2 + 3 + 3, time / 60);
        
        return 0;
}

/**
 * Stop timed program.
 *
 * buf:         output buffer (see SDP_BUF_SIZE_MIN)
 * addr:        rs485 device address: 1 - 31 (use anny valid for rs232)
 * returns:     0 on success, -1 on error
 */
int sdp_stop(char *buf, int addr)
{
        return sdp_print_cmd(buf, sdp_cmd_stop, addr);
}

