#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <string>
#include <unistd.h> 
#include "data.h"
#include "source.h"
#include "timer.h"
#include "dsm.h"
#include "audio-player.h"
#include "thpool.h"
#include "client-proxy.h"

threadpool g_thpool = NULL;
struct Data g_data = {1280,720,0,0};
//struct Data g_data = {640,480,0,0};
//struct Data g_data = {1920,1080,0,0};
int adas_source_type = 0;
int running = 1;

static void usage(int error_code) {
        std::string stderr_info = "Usage: adas-avm [OPTIONS]\n\n"
                        "  -c\tRun with camera source\n"
                        "  -v\tRun with video file source\n"
                        "  -h\tThis help text\n\n";
    
        fprintf(stderr, "%s", stderr_info.c_str());
        exit(error_code);
}

static void signal_int(int signum) {
        running = 0;
}

static void init_sig(struct sigaction *sigint) {
        sigint->sa_handler = signal_int;
        sigemptyset(&sigint->sa_mask);
        sigint->sa_flags = SA_RESETHAND;
        sigaction(SIGINT, sigint, NULL);
}

static void parse_parameter(int argc, char**argv) {
        int i = 0;
        for (i = 1; i < argc; i++) {
                if (strcmp("-c", argv[i]) == 0)
                        adas_source_type = 0;
		if (strcmp("-cc", argv[i]) == 0)
                        adas_source_type = 10;
                else if (strcmp("-v", argv[i]) == 0)
                        adas_source_type = 1;
		else if (strcmp("-vv", argv[i]) == 0)
                        adas_source_type = 11;
                else if (strcmp("-h", argv[i]) == 0)
                        usage(EXIT_SUCCESS);
                else
                        usage(EXIT_FAILURE);
        }   
}

void calculateFPS(struct Data *data) {
        static const uint32_t benchmark_interval = 5;
        struct timeval tv;
        gettimeofday(&tv, NULL );
        uint32_t time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        if (data->frames == 0)
                data->benchmark_time = time;
        if (time - data->benchmark_time > (benchmark_interval * 1000)) {
                printf("%d frames in %d seconds: %f fps\n", data->frames,
                                benchmark_interval,
                                (float) data->frames / benchmark_interval);
                data->benchmark_time = time;
                data->frames = 0;
        }
}

void getCameraIndex(struct Data *data)
{
                        data->fcwCameraIndex = 1;
                        data->dsmCameraIndex = 0;
	return;
        int temp1 = -1, temp2 = -2;
        FILE *pf;
        char buffer[4096];
        pf = popen("lsusb", "r");
        fread(buffer, sizeof(buffer), 1, pf);
        //printf("%s\n", buffer);
        char* token = strtok(buffer,"\n");
        int index = 0;
        while(token!=NULL){
                //printf("line %d: %s\n",index++,token);
                char* r1 = strstr(token,"Alcor Micro Corp");
                char* r2 = strstr(token,"Logitech, Inc. HD Webcam C525");
                char buf[1024];
                int tempInt;
                int count = 0;
                if(r1 != NULL){
                        int ret = sscanf(token, "Bus %d Device %d: ID %s", &tempInt, &temp1, buf);
                        printf("Get Alcor camera device ID: %d\n",temp1);
                } else if(r2 != NULL) {
                        int ret = sscanf(token, "Bus %d Device %d: ID %s", &tempInt, &temp2, buf);
                        printf("Get Logitech camera device ID: %d\n",temp2);
                }
                token = strtok(NULL,"\n");
        }
        if(temp1>0 && temp2>0){
                if (temp1 > temp2){
                        data->fcwCameraIndex = 0;
                        data->dsmCameraIndex = 1;
                        printf("LDW camera: video0\nDSM camera: Video1\n");
                } else {
                        data->fcwCameraIndex = 1;
                        data->dsmCameraIndex = 0;
                        printf("DSM camera: video0\nLDW camera: Video1\n");
                }
        }
        pclose(pf);
}

int main(int argc, char **argv) {

	parse_parameter(argc, argv);
	//handle ctrl+c signal to quit application. 
        struct sigaction sigint;
        init_sig(&sigint);

	getCameraIndex(&g_data);	
	init_adas_source(&g_data, adas_source_type);

	switch (adas_source_type) {
	case 0:
		g_data.df = DF_YUYV;
		break;
	case 1:
		g_data.df = DF_BGR;
		break;
	default:
		g_data.df = DF_UNKNOWN;
	}

	dsm_init(&g_data);
	
	//mainloop
	while (running) {
		calculateFPS(&g_data);
		dequeue_adas(&g_data, adas_source_type);
		dsm_process(&g_data);
		g_data.frames++;
		enqueue_adas(&g_data, adas_source_type);
	}

	fini_adas_source(&g_data, adas_source_type);

	return 0;
}
