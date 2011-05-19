/*##############################################################################
* Copyright (c) 2009-2010, Jiří Pinkava                                        #
# All rights reserved.                                                         #
#                                                                              #
# Redistribution and use in source and binary forms, with or without           #
# modification, are permitted provided that the following conditions are met:  #
#     * Redistributions of source code must retain the above copyright         #
#       notice, this list of conditions and the following disclaimer.          #
#     * Redistributions in binary form must reproduce the above copyright      #
#       notice, this list of conditions and the following disclaimer in the    #
#       documentation and/or other materials provided with the distribution.   #
#     * Neither the name of the Jiří Pinkava nor the                           #
#       names of its contributors may be used to endorse or promote products   #
#       derived from this software without specific prior written permission.  #
#                                                                              #
# THIS SOFTWARE IS PROVIDED BY Jiří Pinkava ''AS IS'' AND ANY                  #
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED    #
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE       #
# DISCLAIMED. IN NO EVENT SHALL Jiří Pinkava BE LIABLE FOR ANY                 #
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES   #
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  #
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS#
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                 *
##############################################################################*/

#ifndef __MSDP2XXX_LOW_H___
#define __MSDP2XXX_LOW_H___

#include "msdp2xxx_base.h"

#ifdef __cplusplus
extern "C" {
#endif

// lenght of shortest valid response ("OK\r")
#define SDP_RESP_LEN_OK 3

typedef enum {
        sdp_resp_incomplete = 0,
        sdp_resp_data,
        sdp_resp_nodata,
} sdp_resp_t;

/* Low level operation functions */
sdp_resp_t sdp_resp(const char *buf, int len);

/* This functions return some data (sdp_resp_data), use corecponding
 * sdp_resp_* function to get this data from response message */
int sdp_sget_dev_addr(char *buf, int addr);
int sdp_sget_lcd_info(char *buf, int addr);
int sdp_sget_preset(char *buf, int addr, int presn);
int sdp_sget_program(char *buf, int addr, int progn);
int sdp_sget_va_maximums(char *buf, int addr);
int sdp_sget_va_data(char *buf, int addr);
int sdp_sget_va_setpoint(char *buf, int addr);
int sdp_sget_volt_limit(char *buf, int addr);

/* Response parsing function should look something like: */
int sdp_resp_dev_addr(char *buf, int len, int *addr);
int sdp_resp_lcd_info(char *buf, int len, sdp_lcd_info_t *lcd_info);
int sdp_resp_preset(char *buf, int len, sdp_va_t *va_preset);
int sdp_resp_program(char *buf, int len, sdp_program_t *program);
int sdp_resp_va_maximums(char *buf, int len, sdp_va_t *va_maximums);
int sdp_resp_va_data(char *buf, int len, sdp_va_data_t *va_data);
int sdp_resp_va_setpoint(char *buf, int len, sdp_va_t *va_setpoints);
int sdp_resp_volt_limit(char *buf, int len, double *volt_limit);

/* This functions respond only "OK" (sdp_resp_nodata) */
int sdp_sremote(char *buf, int addr, int enable);
int sdp_srun_preset(char *buf, int addr, int preset);
int sdp_srun_program(char *buf, int addr, int count);
int sdp_sselect_ifce(char *buf, int addr, sdp_ifce_t ifce);
int sdp_sset_curr(char *buf, int addr, double curr);
int sdp_sset_output(char *buf, int addr, int enable);
int sdp_sset_poweron_output(char *buf, int addr, int presn, int enable);
int sdp_sset_preset(char *buf, int addr, int presn, const sdp_va_t *va_preset);
int sdp_sset_program(char *buf, int addr, int progn,
                const sdp_program_t *program);
int sdp_sset_volt(char *buf, int addr, double volt);
int sdp_sset_volt_limit(char *buf, int addr, double volt);
int sdp_sstop(char *buf, int addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
