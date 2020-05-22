
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <sys/stat.h>
#include <uci.h>
#include <linux/videodev2.h>

#include "uci_config.h"
#include "video_device.h"

#define SENS_NONE        -1
#define SENS_CSI          0
#define SENS_USB          1

typedef struct {
    int pid_capture;
    int pid_server;
} ch_pid_t;

static ch_pid_t ch_pid;
pthread_t p_start_app;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int sv_exit = 0;
static int g_sensor_type = SENS_CSI;
static unsigned int rtsp_start_en = 0;

#define LOG(FMT, ...) { \
    char buf1[512]={0}; \
    sprintf(buf1,FMT, ## __VA_ARGS__); \
    openlog("RTSP SVC", 0, LOG_UUCP); \
    syslog(LOG_INFO, "%s", buf1); \
    closelog(); \
}

void close_all_child(void) {  
    if (ch_pid.pid_server != -1) kill(ch_pid.pid_server, SIGTERM);
    if (ch_pid.pid_capture != -1) kill(ch_pid.pid_capture, SIGTERM);

    return;
}
/*
 *
 */
static void signal_handler(int sig) {
    int pid, i;
    int status;    
    //printf("find signal %d\n", sig);
    if (sig == SIGCHLD) {
        while (1) {
            pid = waitpid(-1 /* any child */, &status, WNOHANG); // gdu: important - WNOHANG
            if (pid == ch_pid.pid_capture) {
                ch_pid.pid_capture = -1;
                rtsp_start_en = 0;
            }
            
            if (pid == ch_pid.pid_server) {
                ch_pid.pid_server = -1;
            }

            if (pid <= 0) {
                break;
            }            
        }       
    // Обработка сигнала при изменении uci param, отправляемого скриптом     
    } else if (sig == SIGHUP) {
        LOG("config updated\n");
        uci_read_config();
        close_all_child();
    } else if (sig == SIGTERM || sig == SIGINT || sig == SIGKILL) {
        sv_exit = 1;
    } else if (sig == SIGUSR1) {
        
    } else if (sig == SIGUSR2) {

    }
}

/*
 *
 */
static int init_signal_handler() {
    struct sigaction act;
    int i;
    /* We need block signals while handler works */
    /* SIGPIPE - error signal */
    int signals[] = { SIGCHLD, SIGHUP, SIGTERM, SIGINT, SIGKILL, SIGBUS, SIGFPE, SIGSYS, SIGTSTP, SIGUSR1, SIGUSR2};
     
    (void) sigemptyset (&act.sa_mask);

    for(i = 0; i < (sizeof(signals)/sizeof(signals[0])); i++) {
        sigaddset(&act.sa_mask, signals[i]);
    }
  
    act.sa_handler = signal_handler; // указатель на обработчик сигнала 
    act.sa_flags = SA_RESTART; // изменения поведения сигнала: перезапуск системного вызова после возврата из обраб.сигнала

    for(i = 0; i < (sizeof(signals)/sizeof(signals[0])); i++) {
        sigdelset(&act.sa_mask, signals[i]);
        (void) sigaction (signals[i], &act, (struct sigaction *) NULL);
        sigaddset(&act.sa_mask, signals[i]);
    }
   
    return 0;
}


/*
 *
 */
int parse_video_size(char *width_ptr, char *height_ptr, char *str){
    int i, j, k;
    char *point;
    char width[50], height[50];
    
    memset(width, 0, sizeof(width));
    memset(height, 0, sizeof(height));
    point = str;

    for (i = 0, j = 0, k = 0; *point != '\0'; point++){
        if (*point != 'x' && k != 1){
            width[i] = *point;
            i++;
        }
        if (*point == 'x'){
            k = 1;
            point++;
        }
        if (k == 1){
            height[j] = *point;
            j++;
        }           
    }    
    if (width == NULL || height == NULL)
        return -1; 
    
    strcpy(width_ptr, width);
    strcpy(height_ptr, height);
    return 0;
}


/*
 *
 */
pid_t start_capture() {
    int i;
    pid_t pid;
    char src_w[50];
    char src_h[50];
    char *pdev;
    char *pformat;
    char *pfps;
    char *pres;

    memset(src_w, 0, sizeof(src_w));
    memset(src_h, 0, sizeof(src_h));
   // memset(str_mask, 0, sizeof(str_mask));

    pdev = config_get_val(RTSP_DEVICE);
    pformat = config_get_val(RTSP_FORMAT);
    pfps = config_get_val(RTSP_FPS);
    pres = config_get_val(RTSP_RESOLUTION);
        
    if (parse_video_size(src_w, src_h, pres) < 0) {
        fprintf(stderr,"Invalid resolution '%s', must be in the form WxH\n", pres);    
    } 

    LOG("capture start with resolution %s\n", pres);
    pid = fork();
    if (pid == 0) {     
    // /usr/sbin/capture -v /dev/video0 -w 1280 -h 720 -f YUYV         
        execl("/usr/sbin/capture", "capture", "-v", pdev, "-w", src_w, "-h", src_h, "-f", pformat,  NULL);
        exit(1);
        
    } else {          
        //free(src_w);
        //free(src_h);
    }
    sleep(1);
    rtsp_start_en = 1;
      
    return pid;
}

/*
 * 
 */
pid_t start_server() {
    char src_w[50];
    char src_h[50];
    //char *log_pass = NULL;
    pid_t pid;  
    int rtsp_ver = 1;
    char *vformat_fix = "160x120";
    char AV_DEVICE[50];

    char *pport = config_get_val(RTSP_PORT);
    char *purl = config_get_val(RTSP_CHANNEL);
    char *pfps = config_get_val(RTSP_FPS);
    char *pres = config_get_val(RTSP_RESOLUTION);
    
    //log_pass = (char*)malloc(sizeof(char)*(VAL_LEN*2) + 1);
    
    memset(src_w, 0, sizeof(src_w));
    memset(src_h, 0, sizeof(src_h));
    //memset(log_pass, 0, sizeof(log_pass));

    if (parse_video_size(src_w, src_h, pres) < 0) {
        fprintf(stderr,"Invalid size '%s', must be in the form WxH\n", pres);    
    }   

    sprintf(AV_DEVICE, "%s", "/dev/video8");

    LOG("rtspserver_stream start: resolution %s, port %s, channel %s\n", pres, pport, purl);
    pid = fork();

    if (pid == 0) {
        // without auth
         execl("/usr/sbin/v4l2rtspserver", "v4l2rtspserver", AV_DEVICE, "-W",src_w,"-H",src_h,"-F",pfps, 
                "-P", pport, "-u", purl, NULL);
        exit(1);

    } else {
        //free(src_w);
        //free(src_h);
        //free(log_pass);
    }    
    return pid;
}

/*
 * 
 */
static void *thread_start_app(void *unused) {
    
    while (1) {
        if (g_sensor_type != SENS_NONE) {
            if (ch_pid.pid_capture == -1) {
                ch_pid.pid_capture = start_capture();
                LOG("Starting capture\n");
                sleep(1);
            }
                    
            if (ch_pid.pid_server == -1) {
                 if (!rtsp_start_en) goto done;
                LOG("Starting RTSP\n");
                ch_pid.pid_server = start_server();
            }
        }
done:        
        sleep(1);
    }
 
    return 0;
}

/*
 *
 */
static void exit_with_error() {
    exit(1);
}

int main(int argc, char** argv) {
    pthread_attr_t attr[2];
    int fd;

    memset(&ch_pid, -1, sizeof(ch_pid));

    //0. sensor type
    //g_sensor_type = get_cam_type();
    // Setup USB sensor if any
    //if (g_sensor_type >= SENS_USB) {
    //    usb_cam_setup(USB_H264_DEV);
    //}

    // 1. Read and save uci start config or set default values
    uci_init_config();
    uci_read_config();
    
    // 2. Init signals
    init_signal_handler();

    // 3. Start kill thread   
    pthread_mutex_lock(&mutex);

    {
        int fd;
        unsigned int cap_dev_pix_fmt;
        char src_w[50];
        char src_h[50];
        int w,h;
        char *pdev = config_get_val(RTSP_DEVICE);
        char *pformat = config_get_val(RTSP_FORMAT);
        char *pres = config_get_val(RTSP_RESOLUTION);

        if (open_capture_dev(pdev, &fd)) {
            LOG("Failed to open capture device %s\n", pdev);
            goto run;
        }
        memset(src_w, 0, sizeof(src_w));
        memset(src_h, 0, sizeof(src_h));

        if (parse_video_size(src_w, src_h, pres) < 0) {
            LOG("Invalid resolution '%s', must be in the form WxH\n", pres);    
            goto run;
        } 

        w = atoi(src_w);
        h = atoi(src_h);
        cap_dev_pix_fmt = v4l2_fourcc(pformat[0], pformat[1], pformat[2], pformat[3]);

        if (dev_try_format(fd, w, h, cap_dev_pix_fmt) ) {
            LOG("Input format %s is not supported\n", pformat);
            goto run;
        }
        close(fd);
    }
    
    // 4. Start all our applications
    pthread_attr_init(&attr[1]);
    if (pthread_create(&p_start_app, &attr[1], thread_start_app, NULL) != 0) {
        exit_with_error();
    }

    //6.
    LOG("RTSP subsystem started successfully\n");

run:    
    while (!sv_exit){
        sleep(1);
    }

    pthread_cancel(p_start_app);
    pthread_join(p_start_app, NULL);

    close_all_child();

    pthread_mutex_destroy(&mutex);

    LOG("RTSP subsystem stopped\n");

    return 0;
}
