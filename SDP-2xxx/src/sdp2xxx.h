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

#ifndef __SDP2XXX_H___
#define __SDP2XXX_H___

#ifdef __cplusplus
extern "C" {
#endif

#define SDP_DEV_ADDR_MIN    (1)
#define SDP_DEV_ADDR_MAX    (31)

#define SDP_PRESET_MIN (1)
#define SDP_PRESET_MAX (9)
#define SDP_PRESET_ALL ((SDP_PRESET_MAX + 1))

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL ((SDP_PROGRAM_MAX + 1))

#define SDP_RUN_PROG_INF (0)

#ifdef __linux__
#define SDP_F int
#endif
#ifdef _WIN32
#define SDP_F HANDLE
#endif

typedef enum {
        sdp_ifce_rs232,
        sdp_ifce_rs485,
} sdp_ifce_t;

typedef enum {
        sdp_mode_cv = 0,
        sdp_mode_cc = 1,
} sdp_mode_t;

typedef struct {
        int curr;
        int volt;
} sdp_va_t;

typedef struct {
        int curr;
        int volt;
        sdp_mode_t mode;
} sdp_va_data_t;

typedef struct {
        int curr;
        int volt;
        int time;
} sdp_program_t;

typedef struct {
        sdp_va_t va_data;
        int wats;
        int time;
        int timer;
        sdp_va_t va_setpoints;
        int program;
        int key;
        int fault;
        int output;
        int remote;
} sdp_ldc_info_t;

typedef struct {
        unsigned char addr;
        SDP_F f;
} sdp_t;

/* High leve operation functions */
sdp_t *sdp_open(sdp_t *sdp, const char *fname, int addr);
sdp_t *sdp_openf(sdp_t *sdp, SDP_F f, int addr);
sdp_t *sdp_close(sdp_t *sdp);

int sdp_fget_dev_addr(const sdp_t *sdp);
int sdp_fget_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums);
int sdp_fget_volt_limit(const sdp_t *sdp);
int sdp_fget_va_data(const sdp_t *sdp, sdp_va_data_t *va_data);
int sdp_fget_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints);
int sdp_fget_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset);
int sdp_fget_program(const sdp_t *sdp, int progn, sdp_program_t *program);
int sdp_fget_ldc_info(const sdp_t *sdp, sdp_ldc_info_t *lcd_info);
int sdp_fremote(const sdp_t *sdp, int enable);
int sdp_frun_preset(const sdp_t *sdp, int preset);
int sdp_frun_program(const sdp_t *sdp, int count);
int sdp_fselect_ifce(const sdp_t *sdp, sdp_ifce_t ifce);
int sdp_fset_curr(const sdp_t *sdp, int curr);
int sdp_fset_volt(const sdp_t *sdp, int volt);
int sdp_fset_volt_limit(const sdp_t *sdp, int volt);
int sdp_fset_output(const sdp_t *sdp, int enable);
int sdp_fset_poweron_output(const sdp_t *sdp, int preset, int enable);
int sdp_fset_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset);
int sdp_fset_program(const sdp_t *sdp, int progn, const sdp_program_t *program);
int sdp_fstop(const sdp_t *sdp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
