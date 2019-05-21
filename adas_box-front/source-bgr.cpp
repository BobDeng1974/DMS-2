#include "source.h"
#include <opencv2/opencv.hpp>

std::string bgr_filepath[2] = {"camera0.bgr","camera1.bgr"};
FILE* bgr_file[2];



int bgr_frame_amount[2] = {0,0}; 
bool bgr_video_end[2] = {false,false};


int init_adas_video_source(struct Data *data)
{
    printf("enter function init_adas_video_source!\n");
    for(int index=0; index<adas_camera_num; index++) {
    	// open bgr file
    	bgr_file[index] = fopen(bgr_filepath[index].c_str(), "rb");
    	if (!bgr_file[index]) {
    		printf("Could not open input stream %s\n", bgr_filepath[index].c_str());
        	return -1;
    	}

    	// initialize  buffer
    	data->buffers[index][0].start = (unsigned char *)malloc(data->cam_width * data->cam_height * 3);
    	if (NULL != data->buffers[index][0].start) {
            memset(data->buffers[index][0].start, 0, data->cam_width * data->cam_height * 3);
            data->buffers[index][0].length = data->cam_width * data->cam_height * 3;
    	} else {
            printf("Fail to allocate memomy for buffer\n");
            return -1;
    	}
    }
    return 0;
}

int read_one_frame(struct Data* data, int index)
{
    for(int index=0; index<adas_camera_num; index++) {
    	int IMAGE_SIZE = data->cam_width * data->cam_height * 3;
	unsigned char in_buffer[IMAGE_SIZE];
        int cur_size = fread(in_buffer, 1, IMAGE_SIZE, bgr_file[index]);
        if (0 == cur_size)
        {
            bgr_video_end[index] = true;
            return 0;
        }

	uint8_t* ptr = (uint8_t*) data->buffers[index][0].start;
        memcpy(ptr, in_buffer, IMAGE_SIZE);
	bgr_frame_amount[index]++;
    }
    return 0;
}


int dequeue_adas_video(struct Data *data)
{
    for(int index=0; index<adas_camera_num; index++) {
    	if(bgr_video_end[index])
    	{
            printf("frame_count: %d\n",  bgr_frame_amount[index]);
    	} else {
	    read_one_frame(data, index);
	}
    }
}

void fini_adas_video_source(struct Data *data)
{
	for(int index=0; index<adas_camera_num; index++) {
        	fclose(bgr_file[index]);
		free(data->buffers[index][0].start);
	}
}
