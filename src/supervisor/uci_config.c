#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <uci.h>

#include "uci_config.h"

config_t rtsp_config[] = {
    {RTSP_DEVICE,       "system.rtsp.device",   "/dev/video0"},
    {RTSP_PROTO,        "system.rtsp.proto",    "udp"},
    {RTSP_FORMAT,       "system.rtsp.format",   "YUYV"},
    {RTSP_COLOR,        "system.rtsp.color",     "1"},
    {RTSP_PORT,         "system.rtsp.port",      "8554"},
    {RTSP_RESOLUTION,   "system.rtsp.resolution","1280x720"},
    {RTSP_CHANNEL,      "system.rtsp.channel",   "live.cam2"},
    {RTSP_FPS,          "system.rtsp.fps",       "20"},
};

/*
 * 
 */
int uci_get_value (char *path, char *buffer) {
    int ret = -1;
    char tmp[128];
    struct  uci_ptr ptr;    
    struct  uci_context *c = uci_alloc_context();      
    if (!c)
        return ret;
    memset(&ptr, 0, sizeof(struct uci_ptr));
    strcpy(tmp, path);
    
    if (uci_lookup_ptr(c, &ptr, tmp, true) != UCI_OK) { 
        uci_free_context(c);
        return ret;
    }
    if (ptr.o == NULL || ptr.o->v.string == NULL) 
        ret = -1;
    else {
        strcpy(buffer, ptr.o->v.string);
        ret = 0;
    } 
    uci_free_context(c);
    return ret;        
}

/*
 * 
 */
void uci_init_config() {
    int i;
    for (i = 0;i < RTSP_N_CONFIG;i++) {
        strcpy(rtsp_config[i].value, rtsp_config[i].dflt);
    }
}

/*
 * 
 */
int uci_read_config() {
    int i;

    for (i = 0;i < RTSP_N_CONFIG;i++) {
        uci_get_value(rtsp_config[i].name, rtsp_config[i].value);
    }
    return 0;
}

/*
 * 
 */
int uci_read_config_ext(config_t config[]) {
    int i;

    for (i = 0;i < RTSP_N_CONFIG;i++) {
        uci_get_value(config[i].name, config[i].value);
    }
    return 0;
}

/*
 * 
 */
int config_get_value(int idx, void *pval, int type) {
    config_t *pcval;
    int i;

    for (i = 0;i < RTSP_N_CONFIG;i++) {
        if (idx == rtsp_config[i].idx)  {
            pcval = &rtsp_config[i];
            break;
        }
    }

    if (i == RTSP_N_CONFIG) return -1;
    
    if (type == CFG_STR) {
        strcpy((char *)pval, pcval->value);
    } else if (type == CFG_INT) {
        *((int *)pval) = atoi(pcval->value);
    } else if (type == CFG_FLOAT) {
        *((float *)pval) = atof(pcval->value);
    } else {
        return -1;
    }
    return 0;
}

/*
 * 
 */
int config_get_local_index(int idx) {
    int i;
    for (i = 0;i < RTSP_N_CONFIG;i++) {
        if (idx == rtsp_config[i].idx)  {
            return i;
        }
    }
    return -1;
}

/*
 * 
 */
char *config_get_val(int idx) {
    int i;
    for (i = 0;i < RTSP_N_CONFIG;i++) {
        if (idx == rtsp_config[i].idx)  {
            return rtsp_config[i].value;
        }
    }
    return NULL;
}

/*
 * 
 */
int config_set_val(int idx, char *val) {
    int i;
    
    if (!val) return -1;

    for (i = 0;i < RTSP_N_CONFIG;i++) {
        if (idx == rtsp_config[i].idx)  {
            strcpy(rtsp_config[i].value, val);
        }
    }
    return 0;
}

/*
 *
 */
void uci_update_config(int cfg_idx) {
   int idx = config_get_local_index(cfg_idx);
   if (idx == -1) return;
   uci_get_value(rtsp_config[idx].name, rtsp_config[idx].value);
}

/*
 * 
 */
void config_print_all() {
    int i;
    for (i = 0;i < RTSP_N_CONFIG;i++) {
        printf("%s=%s\n", rtsp_config[i].name, rtsp_config[i].value);
    }
}
