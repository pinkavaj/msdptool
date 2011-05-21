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

#ifndef __MSDP2XXX_H___
#define __MSDP2XXX_H___

#include "msdp2xxx_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SDP device structure.
 */
typedef struct {
        /** Address of device (1-31). */
        unsigned char addr;
        /** SDP device input file handler. */
        SDP_F f_in;
        /** SDP device output file handler, normaly f_in == f_out. */
        SDP_F f_out;
} sdp_t;

/* High leve operation functions */
#ifdef _WIN32
int sdp_open(sdp_t *sdp, const wchar_t *fname, int addr);
#else
int sdp_open(sdp_t *sdp, const char *fname, int addr);
#endif
int sdp_openf(sdp_t *sdp, SDP_F f, int addr);
void sdp_close(sdp_t *sdp);

int sdp_get_dev_addr(const sdp_t *sdp);
int sdp_get_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums);
int sdp_get_volt_limit(const sdp_t *sdp, double *volt);
int sdp_get_va_data(const sdp_t *sdp, sdp_va_data_t *va_data);
int sdp_get_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints);
int sdp_get_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset);
int sdp_get_program(const sdp_t *sdp, int progn, sdp_program_t *program);
int sdp_get_lcd_info(const sdp_t *sdp, sdp_lcd_info_t *lcd_info);
int sdp_remote(const sdp_t *sdp, int enable);
int sdp_run_preset(const sdp_t *sdp, int preset);
int sdp_run_program(const sdp_t *sdp, int count);
int sdp_select_ifce(const sdp_t *sdp, sdp_ifce_t ifce);
int sdp_set_curr(const sdp_t *sdp, double curr);
int sdp_set_volt(const sdp_t *sdp, double volt);
int sdp_set_volt_limit(const sdp_t *sdp, double volt);
int sdp_set_output(const sdp_t *sdp, int enable);
int sdp_set_poweron_output(const sdp_t *sdp, int presn, int enable);
int sdp_set_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset);
int sdp_set_program(const sdp_t *sdp, int progn, const sdp_program_t *program);
int sdp_stop(const sdp_t *sdp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
