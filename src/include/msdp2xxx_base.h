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

#ifndef __MSDP2XXX_BASE_H___
#define __MSDP2XXX_BASE_H___

#ifdef _WIN32

#ifndef WINVER                  // Minimum required platform is Win XP
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT            // Minimum required platform is Win XP
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS          // Minimum required platform is Win XP
#define _WIN32_WINDOWS 0x0501
#endif

#include <windows.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal lenght of buffer where SDP command is writen, it is garanted
 * that all commands will fit into this buffer including trailing \0 */
#define SDP_BUF_SIZE_MIN (20)

#define SDP_DEV_ADDR_MIN    (1)
#define SDP_DEV_ADDR_MAX    (31)

#define SDP_PRESET_MIN (1)
#define SDP_PRESET_MAX (9)
#define SDP_PRESET_ALL ((SDP_PRESET_MAX) + 1)

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL ((SDP_PROGRAM_MAX) + 1)

#define SDP_RUN_PROG_INF (0)

#ifdef __linux__
#define SDP_F int
#define SDP_F_ERR -1
#endif
#ifdef _WIN32
#define SDP_F HANDLE
#define SDP_F_ERR INVALID_HANDLE_VALUE
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
        double curr;
        double volt;
} sdp_va_t;

typedef struct {
        double curr;
        double volt;
        sdp_mode_t mode;
} sdp_va_data_t;

/**
 * curr:        current: [A]
 * volt:        voltage: [V]
 * time:        lenght of program item duration [sec]
 */
typedef struct {
        double curr;
        double volt;
        int time;
} sdp_program_t;


/**
 * This structure stores data returned from GPAL call.
 * GPAL command returns "ASCII encoded" raw dump of registers
 * used to drive LCD panel. Use sdp_xxx to convert this data into
 * more usefull form shown in structure sdp_lcd_info_t
 */
typedef struct {
        unsigned char read_V[4];
        unsigned char read_V_ind;
        unsigned char read_A[4];
        unsigned char read_A_ind;
        unsigned char read_W[4];
        unsigned char read_W_ind;
        unsigned char time[4];
        unsigned char timer_ind;
        unsigned char colon_ind;
        unsigned char m_ind;
        unsigned char s_ind;
        unsigned char set_V[3];
        unsigned char set_V_const;
        unsigned char set_V_bar;
        unsigned char set_V_ind;
        unsigned char set_A[3];
        unsigned char set_A_const;
        unsigned char set_A_bar;
        unsigned char set_A_ind;
        unsigned char prog;
        unsigned char prog_on;
        unsigned char prog_bar;
        unsigned char setting_ind;
        unsigned char key_lock;
        unsigned char key_open;
        unsigned char fault_ind;
        unsigned char output_on;
        unsigned char output_off;
        unsigned char remote_ind;
} sdp_lcd_info_raw_t;

typedef struct {
	int output;
} sdp_lcd_info_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif
