#ifndef __SDP2XXX_H___
#define __SDP2XXX_H___

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
#define SDP_PRESET_ALL ((SDP_PRESET_MAX + 1))

#define SDP_PROGRAM_MIN (0)
#define SDP_PROGRAM_MAX (19)
#define SDP_PROGRAM_ALL ((SDP_PROGRAM_MAX + 1))

#define SDP_RUN_PROG_INF (0)

typedef enum {
        sdp_ifce_rs232,
        sdp_ifce_rs485,
} sdp_ifce_t;

typedef enum {
        sdp_resp_incomplete = 0,
        sdp_resp_data,
        sdp_resp_nodata,
} sdp_resp_t;

typedef struct {
        int curr;
        int volt;
} sdp_va_t;

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

sdp_resp_t sdp_resp(const char *buf, int len);

/* This functions return some data (sdp_resp_data), use corecponding
 * sdp_resp_* function to get this data from response message */
int sdp_get_dev_addr(char *buf, int addr);
int sdp_get_va_maximums(char *buf, int addr);
int sdp_get_volt_limit(char *buf, int addr);
int sdp_get_va_data(char *buf, int addr);
int sdp_get_va_setpoint(char *buf, int addr);
int sdp_get_preset(char *buf, int addr, int preset);
int sdp_get_program(char *buf, int addr, int program);
int sdp_get_ldc_info(char *buf, int addr);

/* Response parsing function should look something like: */
int sdp_resp_dev_addr(char *buf, int len, int *addr);
int sdp_resp_va_maximums(char *buf, int len, sdp_va_t *va_maximums);
int sdp_resp_volt_limit(char *buf, int len, int *volt_limit);
// FIXME: missing information about mode: constant voltage/current
int sdp_resp_va_data(char *buf, int len, sdp_va_t *va_data);
int sdp_resp_va_setpoint(char *buf, int len, sdp_va_t *va_setpoints);
int sdp_resp_preset(char *buf, int len, sdp_va_t *va_preset);
int sdp_resp_program(char *buf, int len, sdp_program_t *program);
int sdp_resp_ldc_info(char *buf, int len, sdp_ldc_info_t *lcd_info);

/* This functions respond only "OK" (sdp_resp_nodata) */
int sdp_remote(char *buf, int addr, int enable);
int sdp_run_preset(char *buf, int addr, int preset);
int sdp_run_program(char *buf, int addr, int count);
int sdp_select_ifce(char *buf, int addr, sdp_ifce_t ifce);
int sdp_set_curr(char *buf, int addr, int curr);
int sdp_set_volt(char *buf, int addr, int volt);
int sdp_set_volt_limit(char *buf, int addr, int volt);
int sdp_set_output(char *buf, int addr, int enable);
int sdp_set_poweron_output(char *buf, int addr, int preset, int enable);
int sdp_set_preset(char *buf, int addr, int preset, int volt, int curr);
int sdp_set_program(char *buf, int addr, int program, int volt, int curr, 
                int time);
int sdp_stop(char *buf, int addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
