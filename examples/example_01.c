/*##############################################################################
* Copyright (c) 2011, Jiří Pinkava                                             #
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

/*
 * This is moust trivial example of use of MSDP library.
 *
 * For simplicity it miss some of error handling!
 */

#include <errno.h>
#include <stdio.h>

#include "msdp2xxx.h"

#ifdef __linux__
const char filename[] = "/dev/ttyS0";
#elif _WIN32 == 1
const char filename[] = "COM1";
#endif

int main()
{
        sdp_t sdp;

        // Open SDP power supply, start remote operations
        if (sdp_open(&sdp, filename, 1) == -1) {
                perror("Failed to open serial port connected to SDP");
                return -1;
        }

        // start remote mode
        if (sdp_remote(&sdp, 1) == -1) {
                perror("Could not start remote control");
                return -1;
        }

        // set output voltage and current
        sdp_set_volt(&sdp, 1.5);
        sdp_set_curr(&sdp, 0.05);

        // set output to on
        sdp_set_output(&sdp, 1);

        // Set to local controll mode and close comunication
        sdp_close(&sdp);
}

