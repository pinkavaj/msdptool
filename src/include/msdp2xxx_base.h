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
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Minimal lenght of buffer where SDP command is writen, it is garanted
 * that all commands will fit into this buffer including trailing '\0'. */
#define SDP_BUF_SIZE_MIN (20)

#define SDP_DEV_ADDR_MIN    (1)
#define SDP_DEV_ADDR_MAX    (31)

#define SDP_PRESET_MIN (1)
#define SDP_PRESET_MAX (9)
#define SDP_PRESET_ALL (SDP_PRESET_MAX + 1)

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL (SDP_PROGRAM_MAX + 1)

/** Repeat runned program indefinitely */
#define SDP_RUN_PROG_INF (0)

/** No error. */
#define SDP_EOK         (0)
/** System error, error code is in errno */
#define SDP_EERRNO      (-1)
/** Response parsing failed, expected number. */
#define SDP_ENONUM      (-2)
/** Function parameter is out of range. */
#define SDP_ERANGE      (-3)
/** Invalid response. */
#define SDP_EINRES      (-4)
/** Response read timeout expired. */
#define SDP_ETIMEDOUT   (-5)
/** Output is too large to fit into buffer. */
#define SDP_ETOLARGE    (-6)
/** Write operation returned with data partialy writen. */
#define SDP_EWINCOMPL   (-8)

#ifdef __linux__
#define SDP_F int
#define SDP_F_ERR       (-1)
#endif
#ifdef _WIN32
#define SDP_F HANDLE
#define SDP_F_ERR       (INVALID_HANDLE_VALUE)
#endif

typedef enum {
        /** Use RS232 interface. */
        sdp_ifce_rs232,
        /** Use RS485 interface. */
        sdp_ifce_rs485,
} sdp_ifce_t;

typedef enum {
        /** PS operate in constant voltage mode */
        sdp_mode_cv = 0,
        /** PS operate in constant current mode */
        sdp_mode_cc = 1,
} sdp_mode_t;

typedef struct {
        /** current [A] */
        double curr;
        /** voltage [V] */
        double volt;
} sdp_va_t;

typedef struct {
        /** current [A] */
        double curr;
        /** voltage [V] */
        double volt;
        /** power supply mode (CC/CV) */
        sdp_mode_t mode;
} sdp_va_data_t;

typedef struct {
        /** current: [A] */
        double curr;
        /** voltage: [V] */
        double volt;
        /** duration of program item [sec] */
        int time;
} sdp_program_t;

/**
 * Container for processed data from GPAL call.
 */
typedef struct {
        /** voltage measured at output [V] */
	double read_V;
        /** V behing reading value, if 1 read_V data are valid */
	unsigned char read_V_ind;
        /** current measured at output [A] */
	double read_A;
        /** A behind reading value, if 1 read_A data are valid */
	unsigned char read_A_ind;
        /** power to output, P = U * I [W] */
	double read_W;
        /** W behind reading value, if 1 read_W data are valid */
	unsigned char read_W_ind;
        /** time remaining to end of current proceeding timed programitem */
	int time;
        /** ??? */
	unsigned char timer_ind;
        /** ??? */
	unsigned char colon_ind;
        /** m letter in time, used only when setting time */
	unsigned char m_ind;
        /** s letter in time, used only when setting time  */
	unsigned char s_ind;
        /** voltage set point [V] */
	double set_V;
        /** 1 when PS operate in constant voltage mode, 0 otherwise */
	unsigned char set_V_const;
        /** ??? */
	unsigned char set_V_bar;
        /** ??? */
	unsigned char set_V_ind;
        /** current set point [A] */
	double set_A;
        /** 1 when PS operate in constant current mode, 0 otherwise */
	unsigned char set_A_const;
        /** ??? */
	unsigned char set_A_bar;
        /** ??? */
	unsigned char set_A_ind;
        /** program/perset number, used only when setting program */
	int prog;
        /** program sign, used only when setting program */
	unsigned char prog_on;
        /** 1 when program is running, 0 otherwise */
	unsigned char prog_bar;
        /** 1 when uset do some setting at front pannel, 0 otherwise */
	unsigned char setting_ind;
        /** 0 when PS keypad is locked, 1 when unlocked */
	unsigned char key;
        /** normaly 0, set to 1 when fault at PS indicated */
	unsigned char fault_ind;
        /** when 0 output is off, when 1 output is on */
	unsigned char output;
        /** when 0 PS use local control, when 1 remote is enabled */
	unsigned char remote_ind;
} sdp_lcd_info_t;

const char *sdp_strerror(int err);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
