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
const static char sdp_cmd_sess[] = "SESS__\r";
const static char sdp_cmd_ends[] = "ENDS__\r";
const static char sdp_cmd_ccom_rs232[] = "CCOM__000\r";
const static char sdp_cmd_ccom_rs485[] = "CCOM__001\r";
const static char sdp_cmd_gcom[] = "GCOM__\r";
const static char sdp_cmd_gmax[] = "GMAX__\r";
const static char sdp_cmd_govp[] = "GOVP__\r";
const static char sdp_cmd_getd[] = "GETD__\r";
const static char sdp_cmd_gets[] = "GETS__\r";
const static char sdp_cmd_getm[] = "GETM__\r";
const static char sdp_cmd_getm_loc[] = "GETM__l\r";
const static char sdp_cmd_getp[] = "GETP__\r";
const static char sdp_cmd_getp_prog[] = "GETP__pp\r";
const static char sdp_cmd_gpal[] = "GPAL__\r";
const static char sdp_cmd_volt[] = "VOLT__uuu\r";
const static char sdp_cmd_curr[] = "CURR__iii\r";
const static char sdp_cmd_sovp[] = "SOVP__uuu\r";
const static char sdp_cmd_sout_dis[] = "SOUT__1\r";
const static char sdp_cmd_sout_en[] = "SOUT__0\r";
const static char sdp_cmd_poww_dis[] = "POWW__i1\r";
const static char sdp_cmd_poww_en[] = "POWW__i0\r";
const static char sdp_cmd_prom[] = "PROM__luuuiii\r";
const static char sdp_cmd_prop[] = "PROP__lluuuiiimmss\r";
const static char sdp_cmd_runm[] = "RUNM__l\r";
const static char sdp_cmd_runp[] = "RUNP__nnn\r";
const static char sdp_cmd_stop[] = "STOP__\r";

/**
 * print SDP command into buffer and fill device address
 *
 * buf  - buffer where to write SDP device command
 *      mus have size at least SDP_BUF_SIZE_MIN
 * addr - rs485 address of device in range 1 - 31, for
 *      rs232 use anny number in this range
 * cmd  - template of command wich should be printed
 * returns -1 on error, 0 otherwise - FIXME: think more about this
 */
static int sdp_print_cmd(char *buf, int addr, const char *cmd)
{
        if (addr < SDP_DEV_ADDR_MIN || addr > SDP_DEV_ADDR_MAX)
                return -1;
       
        strncpy(buf, cmd, SDP_BUF_SIZE_MIN);
        // write SDP addres into cmd template ("ttttAA...\r")
        buf[5] = addr % 10;
        addr /= 10;
        buf[4] = addr % 10;

        return 0;
}

/**
 * check whathever data in buffer contains completed response on SDP command
 *
 * buf  - buffer containing recieved response
 * len  - lenght of data in buffer
 * returns response status (nodata, data, incomplete)
 */
sdp_response_t sdp_response(const char *buf, int len)
{
        static const char str_ok[] = "OK\r";

        if (len < 3)
                return sdp_response_incomplete;
        if (!strncmp(buf + len - 3, str_ok, 3)) {
                /* response contains only OK skring */
                if (len == 3)
                        return sdp_response_nodata;
                /* response contains some data and OK string */
                if (buf[len - 4] == '\r')
                        return sdp_response_data;
        }
        return sdp_response_incomplete;
}

