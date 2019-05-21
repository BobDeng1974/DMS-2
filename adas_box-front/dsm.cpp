#include <stdio.h>
#include <stdlib.h>
#include "dsm.h"
#include "DsmThread.h"
#include <opencv2/opencv.hpp>


static cv::Mat mRetImage[5];
static cv::Mat mMask[5];

extern int gCurIndex;

DsmThread dsmthread;

static std::string labels[] = {
                        "(safe)",
                        "(drowsy)",
                        "(yawn)",
                        "(phone)",
                        "(distracted)"
                        };


int dsm_init(struct Data* data)
{
 //   INIT(DsmThread);
//	CREATE(DsmThread, data);

	dsmthread.init();

//	cv::namedWindow("DSM", CV_WINDOW_NORMAL);
//    cv::setWindowProperty("DSM", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	mRetImage[0] = cv::imread("./dsm_retimage/safe.png");
    mRetImage[1] = cv::imread("./dsm_retimage/sleepy.png");
    mRetImage[2] = cv::imread("./dsm_retimage/yawn.png");
    mRetImage[3] = cv::imread("./dsm_retimage/phone.png");
    mRetImage[4] = cv::imread("./dsm_retimage/distracted.png");

    mMask[0] = cv::imread("./dsm_retimage/safe.png", 0);
    mMask[1] = cv::imread("./dsm_retimage/sleepy.png", 0);
    mMask[2] = cv::imread("./dsm_retimage/yawn.png", 0);
    mMask[3] = cv::imread("./dsm_retimage/phone.png", 0);
    mMask[4] = cv::imread("./dsm_retimage/distracted.png", 0);


    return 0;
}

int dsm_process(struct Data* data)
{
 
//	START(DsmThread);
	dsmthread.run(data);
	
    return 0;
}


int dsm_fini()
{
    DESTROY(DsmThread);
	return 0;
}


