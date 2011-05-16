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

sdp_t *sdp_open(sdp_t *sdp, const char *fname, int addr)
sdp_t *sdp_openf(sdp_t *sdp, SDP_F f, int addr)
sdp_t *sdp_close(sdp_t *sdp)

int sdp_fget_dev_addr(const sdp_t *sdp)
{
        return -1;
}

int sdp_fget_va_maximums(const sdp_t *sdp, sdp_va_t *va_maximums)
{
        return -1;
}

int sdp_fget_volt_limit(const sdp_t *sdp)
{
        return -1;
}

int sdp_fget_va_data(const sdp_t *sdp, sdp_va_data_t *va_data)
{
        return -1;
}

int sdp_fget_va_setpoint(const sdp_t *sdp, sdp_va_t *va_setpoints)
{
        return -1;
}

int sdp_fget_preset(const sdp_t *sdp, int presn, sdp_va_t *va_preset)
{
        return -1;
}

int sdp_fget_program(const sdp_t *sdp, int progn, sdp_program_t *program)
{
        return -1;
}

int sdp_fget_ldc_info(const sdp_t *sdp, sdp_ldc_info_t *lcd_info)
{
        return -1;
}

int sdp_fremote(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_frun_preset(const sdp_t *sdp, int preset)
{
        return -1;
}

int sdp_frun_program(const sdp_t *sdp, int count)
{
        return -1;
}

int sdp_fselect_ifce(const sdp_t *sdp, sdp_ifce_t ifce)
{
        return -1;
}

int sdp_fset_curr(const sdp_t *sdp, int curr)
{
        return -1;
}

int sdp_fset_volt(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_fset_volt_limit(const sdp_t *sdp, int volt)
{
        return -1;
}

int sdp_fset_output(const sdp_t *sdp, int enable)
{
        return -1;
}

int sdp_fset_poweron_output(const sdp_t *sdp, int preset, int enable)
{
        return -1;
}

int sdp_fset_preset(const sdp_t *sdp, int presn, const sdp_va_t *va_preset)
{
        return -1;
}

int sdp_fset_program(const sdp_t *sdp, int progn, const sdp_program_t *program)
{
        return -1;
}

int sdp_fstop(const sdp_t *sdp)
{
        return -1;
}

