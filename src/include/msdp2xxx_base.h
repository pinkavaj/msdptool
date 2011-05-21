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

/* Minimal lenght of buffer where SDP command is writen, it is garanted
 * that all commands will fit into this buffer including trailing \0 */
#define SDP_BUF_SIZE_MIN (20)

#define SDP_DEV_ADDR_MIN    (1)
#define SDP_DEV_ADDR_MAX    (31)

#define SDP_PRESET_MIN (1)
#define SDP_PRESET_MAX (9)
#define SDP_PRESET_ALL (SDP_PRESET_MAX + 1)

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL (SDP_PROGRAM_MAX + 1)

#define SDP_RUN_PROG_INF (0)

#ifdef __linux__
#define SDP_F int
#define SDP_F_ERR -1
#endif
#ifdef _WIN32
#define SDP_F HANDLE
#define SDP_F_ERR INVALID_HANDLE_VALUE
#endif

/**
 * @sdp_ifce_rs232:     use RS232 interface
 * sdp_ifce_rs485:      use RS485 interface
 */
typedef enum {
        sdp_ifce_rs232,
        sdp_ifce_rs485,
} sdp_ifce_t;

/**
 * @sdp_mode_cv:        PS operate in constant voltage mode
 * @sdp_mode_cc:        PS operate in constant current mode
 */
typedef enum {
        sdp_mode_cv = 0,
        sdp_mode_cc = 1,
} sdp_mode_t;

/**
 * @curr:       current [A]
 * @volt:       voltage [V]
 */
typedef struct {
        double curr;
        double volt;
} sdp_va_t;

/**
 * @curr:       current [A]
 * @volt:       voltage [V]
 * @mode:       power supply mode (CC/CV)
 */
typedef struct {
        double curr;
        double volt;
        sdp_mode_t mode;
} sdp_va_data_t;

/**
 * @curr:       current: [A]
 * @volt:       voltage: [V]
 * @time:       duration of program item [sec]
 */
typedef struct {
        double curr;
        double volt;
        int time;
} sdp_program_t;

/**
 * Container for processed data from GPAL call
 * @read_V:      voltage measured at output [V]
 * @read_V_ind:  V behing reading value, if 1 read_V data are valid
 * @read_A:      current measured at output [A]
 * @read_A_ind:  A behind reading value, if 1 read_A data are valid
 * @read_W:      power to output, P = U * I [W]
 * @read_W_ind:  W behind reading value, if 1 read_W data are valid
 * @time:        time remaining to end of current proceeding timed programitem
 * @timer_ind:   ???
 * @colon_ind:   ???
 * @m_ind:       m letter in time, used only when setting time
 * @s_ind:       s letter in time, used only when setting time 
 * @set_V:       voltage set point [V]
 * @set_V_const: 1 when PS operate in constant voltage mode, 0 otherwise
 * @set_V_bar:   ???
 * @set_V_ind:   ???
 * @set_A:       current set point [A]
 * @set_A_const: 1 when PS operate in constant current mode, 0 otherwise
 * @set_A_bar:   ???
 * @set_A_ind:   ???
 * @prog:        program/perset number, used only when setting program
 * @prog_on:     program sign, used only when setting program
 * @prog_bar:    1 when program is running, 0 otherwise
 * @setting_ind: 1 when uset do some setting at front pannel, 0 otherwise
 * @key:         0 when PS keypad is locked, 1 when unlocked
 * @fault_ind:   normaly 0, set to 1 when fault at PS indicated
 * @output:      when 0 output is off, when 1 output is on
 * @remote_ind:  when 0 PS use local control, when 1 remote is enabled
 */
typedef struct {
	double read_V;
	unsigned char read_V_ind;
	double read_A;
	unsigned char read_A_ind;
	double read_W;
	unsigned char read_W_ind;
	int time;
	unsigned char timer_ind;
	unsigned char colon_ind;
	unsigned char m_ind;
	unsigned char s_ind;
	double set_V;
	unsigned char set_V_const;
	unsigned char set_V_bar;
	unsigned char set_V_ind;
	double set_A;
	unsigned char set_A_const;
	unsigned char set_A_bar;
	unsigned char set_A_ind;
	int prog;
	unsigned char prog_on;
	unsigned char prog_bar;
	unsigned char setting_ind;
	unsigned char key;
	unsigned char fault_ind;
	unsigned char output;
	unsigned char remote_ind;
} sdp_lcd_info_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif
