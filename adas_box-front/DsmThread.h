#ifndef DSMTHREAD_H_
#define DSMTHREAD_H_

#include "Thread.h"
#include "singleton.h"
// #include "MvncNetwork.h"
#include "HeadPose.h"
#include "DsmGPU.h"
#include "DsmOpenVino.h"


class DsmThread {

SINGLETON_DECL(DsmThread)

public:
	DsmThread();
	virtual ~DsmThread();

	int init();
	int run(struct Data* data);

	int destroy();
private:	

	void initLandmark();	

	cv::Mat select_dms_face(std::vector<cv::Mat> faces);
	void showStatus(cv::Mat &frame, basestatus *status);

	double computeEAR(std::vector<cv::Point> vec);

	double computeMAR(std::vector<cv::Point> vec);

	double computeGDR(std::vector<cv::Point> vec);

	double computeEA(std::vector<cv::Point> vec);
	double computeMA(std::vector<cv::Point> vec);
	
	

private:

	HeadPose mheadPose;
	DsmGPU dsmgpu;
	DsmOpenVino dsmopenvino;
	cv::Mat mRetImage[5];
	cv::Mat mMask[5];
};



#endif

