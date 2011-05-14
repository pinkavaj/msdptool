#ifndef __SDP2XXX_H___
#define __SDP2XXX_H___

typedef enum sdp_ifce_e {
        sdp_ifce_rs232,
        sdp_ifce_rs485,
} sdp_ifce_t;

typedef enum sdp_response_e {
        sdp_response_incomplete = 0,
        sdp_response_data,
        sdp_response_nodata,
} sdp_response_t;
/* minimal lenght of buffer where SDP command is writen */
#define SDP_BUF_SIZE_MIN (20)

#define SDP_DEV_ADDR_MIN    (1)
#define SDP_DEV_ADDR_MAX    (31)

#define SDP_PRESET_MIN (1)
#define SDP_PRESET_MAX (9)
#define SDP_PRESET_ALL ((SDP_PRESET_MAX + 1))

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL ((SDP_PROGRAM_MAX + 1))

sdp_response_t sdp_response(const char *buf, int len);

/* FIXME: remake SDP function stuff, this is just prototype */
int sdp_remote(char *buf, int addr, int enable);
int sdp_set_ifce(char *buf, int addr, sdp_ifce_t ifce);
int sdp_get_dev_addr(char *buf, int addr);
int sdp_get_va_maximums(char *buf, int addr); // ??
int sdp_get_volt_limit(char *buf, int addr);
int sdp_get_va_data(char *buf, int addr); // ??
int sdp_get_setpoint(char *buf, int addr);
int sdp_get_preset(char *buf, int addr, int preset);
int sdp_get_program(char *buf, int addr, int program_num);
int sdp_get_ldc_info(char *buf, int addr); // ??
int sdp_set_voltage(char *buf, int addr);
int sdp_set_current(char *buf, int addr);
int sdp_set_volt_limit(char *buf, int addr);
int sdp_output(char *buf, int addr, int enable);
int sdp_set_poweron(char *buf, int addr);
int sdp_set_program_memory(char *buf, int addr); // ??
int sdp_run_preset(char *buf, int addr); // ??
int sdp_run_timed(char *buf, int addr); // ??
int sdp_stop(char *buf, int addr);

/* Response parsing function should look something like: */
int sdp_response_xxx(char *buf, int len, ...);

#endif
