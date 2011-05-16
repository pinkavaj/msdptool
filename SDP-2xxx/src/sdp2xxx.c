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

#include "sdp2xxx.h"
#include <ctype.h>
#include <string.h>
#include <strings.h>

int sdp_open(sdp_t *sdp, const char *fname, int addr)
{
        return -1;
}

int sdp_openf(sdp_t *sdp, SDP_F f, int addr)
{
        return -1;
}

void sdp_close(sdp_t *sdp)
{
        return -1;
}


int sdp_get_dev_addr(const sdp_t *sdp)
{
        return -1;
}

int sdp_get_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums)
{
        return -1;
}

int sdp_get_volt_limit(const sdp_t *sdp)
{
        return -1;
}

int sdp_get_va_data(const sdp_t *sdp, sdp_va_data_t *va_data)
{
        return -1;
}

int sdp_get_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints)
{
        return -1;
}

int sdp_get_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset)
{
        return -1;
}

int sdp_get_program(const sdp_t *sdp, int progn, sdp_program_t *program)
{
        return -1;
}

int sdp_get_ldc_info(const sdp_t *sdp, sdp_ldc_info_t *lcd_info)
{
        return -1;
}

int sdp_remote(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_run_preset(const sdp_t *sdp, int preset)
{
        return -1;
}

int sdp_run_program(const sdp_t *sdp, int count)
{
        return -1;
}

int sdp_select_ifce(const sdp_t *sdp, sdp_ifce_t ifce)
{
        return -1;
}

int sdp_set_curr(const sdp_t *sdp, int curr)
{
        return -1;
}

int sdp_set_volt(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_set_volt_limit(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_set_output(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_set_poweron_output(const sdp_t *sdp, int preset, int enable)
{
        return -1;
}

int sdp_set_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset)
{
        return -1;
}

int sdp_set_program(const sdp_t *sdp, int progn, const sdp_program_t *program)
{
        return -1;
}

int sdp_stop(const sdp_t *sdp)
{
        return -1;
}

