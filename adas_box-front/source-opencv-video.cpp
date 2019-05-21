#include "source.h"
#include <opencv2/opencv.hpp>

using namespace cv;

VideoCapture g_vid_fcw;
VideoCapture g_vid_dsm;

const char* video_file[2] = {"camera0.mp4","camera1.mp4"};

void init_adas_opencv_video_source(struct Data *data)
{
	g_vid_fcw.open(video_file[0]);
	g_vid_dsm.open(video_file[1]);
	if (!g_vid_fcw.isOpened() || !g_vid_dsm.isOpened())
		printf("opencv open video file failed!\n");
	else {
		data->cam_width = g_vid_fcw.get(CV_CAP_PROP_FRAME_WIDTH);
		data->cam_height = g_vid_fcw.get(CV_CAP_PROP_FRAME_HEIGHT);
	} 
}

void fini_adas_opencv_video_source(struct Data *data)
{
	g_vid_fcw.release();
	g_vid_dsm.release();
}

void dequeue_adas_opencv_video(struct Data *data)
{
	if(g_vid_fcw.isOpened()){
		g_vid_fcw >> data->fcwMat;
		//printf("get fcw frame!\n");
	}
	if(g_vid_dsm.isOpened()){
		g_vid_dsm >> data->dsmMat;
		//printf("get dsm frame!\n");
	}
}

void enqueue_adas_opencv_video(struct Data *data)
{
}
