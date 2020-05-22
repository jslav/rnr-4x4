#ifndef UCI_CONFIG_H
#define UCI_CONFIG_H

#define CFG_STR     0
#define CFG_INT     1
#define CFG_FLOAT   2

enum {
    RTSP_DEVICE = 0,
    RTSP_PROTO,
    RTSP_FORMAT,
    RTSP_COLOR,
    RTSP_PORT,
    RTSP_RESOLUTION,
    RTSP_CHANNEL,
    RTSP_FPS,
    
    RTSP_N_CONFIG
};


typedef struct config_t {
    int idx;
    char *name;
    char *dflt;
    char value[128];
} config_t;

extern config_t uvc_config[];

void uci_init_config();
int uci_read_config();
int config_get_value(int idx, void *pval, int type);
void config_print_all();
int config_get_local_index(int idx);
char *config_get_val(int idx);
int config_set_val(int idx, char *val);
int uci_get_value (char *path, char *buffer);
void uci_update_config(int cfg_idx);
#endif