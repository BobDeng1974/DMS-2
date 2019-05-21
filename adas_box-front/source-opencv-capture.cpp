#include "source.h"
#include <opencv2/opencv.hpp>

using namespace cv;

VideoCapture g_cap_fcw;
VideoCapture g_cap_dsm;


void init_adas_opencv_capture_source(struct Data *data)
{
//	g_cap_fcw.open(data->fcwCameraIndex);
	g_cap_dsm.open(data->dsmCameraIndex);
	if  (!g_cap_dsm.isOpened())
		printf("opencv openc camera failed!\n");
	else {
//		g_cap_fcw.set(CV_CAP_PROP_FRAME_WIDTH, data->cam_width);
//		g_cap_fcw.set(CV_CAP_PROP_FRAME_HEIGHT, data->cam_height);
		g_cap_dsm.set(CV_CAP_PROP_FRAME_WIDTH, data->cam_width);
                g_cap_dsm.set(CV_CAP_PROP_FRAME_HEIGHT, data->cam_height);
	} 
}

void fini_adas_opencv_capture_source(struct Data *data)
{
//	g_cap_fcw.release();
	g_cap_dsm.release();
}

void dequeue_adas_opencv_capture(struct Data *data)
{
/*	if(g_cap_fcw.isOpened()){
		g_cap_fcw >> data->fcwMat;
		//printf("get fcw frame!\n");
	}
*/
	if(g_cap_dsm.isOpened()){
		g_cap_dsm >> data->dsmMat;
		//printf("get dsm frame!\n");
	}
}

void enqueue_adas_opencv_capture(struct Data *data)
{
}
