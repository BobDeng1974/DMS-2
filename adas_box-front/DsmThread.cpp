#include "DsmThread.h"
#include <opencv2/opencv.hpp>
// #include <mvnc.h>
#include "audio-player.h"
#include "util.h"
// #include "MvncHelper.h"
#include <math.h>

#define MVNC_V2 1
#define PLAY_AUDIO 0

#define PI 3.1415926
#define SCALE 0.2

#define GRAPH_FILE_NAME "./dsm_graph"
#define CONTINUOUS_STATUS_THRESHOLD 5

#define DMS_IMAGE_WIDTH  1280
#define DMS_IMAGE_HEIGHT 720

#define RELU(x) (x>0?x:0)
#define MAXWIDTH(x) (  x>DMS_IMAGE_WIDTH?DMS_IMAGE_WIDTH:(x>0?x:0) )
#define MAXHEIGHT(y) (  y>DMS_IMAGE_HEIGHT?DMS_IMAGE_HEIGHT:(y>0?y:0) )

int gCurIndex = 0;

static std::string labels[] = {
                        "(safe)",
                        "(drowsy)",
                        "(yawn)",
                        "(phone)",
                        "(distracted)"
						};

basestatus dmsstatus[6]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};

SINGLETON_IMPL(DsmThread)

DsmThread::DsmThread()
{
	
}

DsmThread::~DsmThread(){
}


int DsmThread::init() {

	initLandmark();
	dsmgpu.initModel();

	//dsmopenvino.initFaceSSD("frozen_inference_graph.xml");
	dsmopenvino.initFaceSSD("phone_smoke.xml");
	//dsmopenvino.initOpenVino();

	return 1;
}

int DsmThread::run(struct Data* data) {

	static int statusCount = 0;

	
	cv::Mat faceFrame;
	cv::Mat righteyeFrame;
	cv::Mat lefteyeFrame;

	int prevStatus = 0;
	int currentStatus = 0;

	 cv::Mat  frame = data->dsmMat;
	//frame = imread("81.jpg");
	//cv::resize(frame, frame, cv::Size(DMS_IMAGE_WIDTH, DMS_IMAGE_HEIGHT));
	if (frame.empty()) 
	{	
		return 0;
	}

    double start = cv::getTickCount();
	static int prevIndex = -1;
	static int curIndex = 0;
    int eyeclosed = 0;

	int left = 0;
	int top = 0;
	//int isfindface = dsmopenvino.getFaceSSD(frame, faceFrame, &left, &top, dmsstatus);
	double angles[3]={0, 0, 0};
	int eyedetect = dsmopenvino.getInteractiveFace(frame, faceFrame, righteyeFrame, lefteyeFrame, left, top, angles, dmsstatus);
	if (eyedetect == 1 && righteyeFrame.empty() == false) {
			eyeclosed = dsmgpu.process(righteyeFrame);
			if (eyeclosed == 1) {
				dmsstatus[5].status = 1;
				++dmsstatus[5].count;					
			} else {
				dmsstatus[5].status = 0;
				dmsstatus[5].count = 0;	
			}
	}
	/*std::ostringstream outtext;
	outtext << "y: " << std::setprecision(3) << angles[0] << "  p: " << angles[1] << " r: " << angles[2];
	cv::putText(frame, outtext.str(), cvPoint(30, 30),
		 CV_FONT_HERSHEY_PLAIN, 2, cv::Scalar(0, 255, 0), 2);*/

	
	

	showStatus(frame, dmsstatus);
	imshow("DSM", frame);
	cv::waitKey(1);

	return 1;
}

void DsmThread::initLandmark() {


}

void DsmThread::showStatus(cv::Mat &frame, basestatus *dmsstatus)
{
	const char* statusstr[] = {"Safe", "Phone", "Smoke", "Distracted", "Yawn", "Sleepy"};
	cv::Scalar defaut(0, 220, 0);
	cv::Scalar warning(0, 255, 255);
	bool isSafe = true;
	 
	for (int i = 0; i < 6; i++) {
		if (dmsstatus[i].status == 1 && dmsstatus[i].count >= CONTINUOUS_STATUS_THRESHOLD) {
			cv::putText(frame, statusstr[i], cvPoint(30, 400 + i * 50),
				CV_FONT_HERSHEY_DUPLEX, 2, warning, 2);
			isSafe = false;
			
		}
		else {
			cv::putText(frame, statusstr[i], cvPoint(30, 400 + i * 50),
				CV_FONT_HERSHEY_DUPLEX, 1, defaut, 2);
		}
		dmsstatus[i].status = 0;
	}
	if (isSafe) {
		//cv::putText(frame, statusstr[0], cvPoint(1080, 50),
		//		CV_FONT_HERSHEY_PLAIN, 3, defaut, 2);
	}
}

cv::Mat DsmThread::select_dms_face(std::vector<cv::Mat> faces)
{
	if (faces.size() == 1) {
		return faces[0];
	}

	int dmsfacearea = 0;
	int dmsfaceindex = 0;
	for (int i = 0; i < faces.size(); i++) {
		int facearea = faces[i].cols * faces[i].rows;
		if(dmsfacearea < facearea) {
			dmsfacearea = facearea;
			dmsfaceindex = i;
		}
	}	

	return faces[dmsfaceindex];
}

double DsmThread::computeEAR(std::vector<cv::Point> vec) // EAR : eye aspect ratio
{

    double a = cv::norm(cv::Mat(vec[1]), cv::Mat(vec[5]));
    double b = cv::norm(cv::Mat(vec[2]), cv::Mat(vec[4]));
    double c = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[3]));
    //compute EAR
    double ear = (a + b) / (2.0 * c);
    return ear;
}

double DsmThread::computeMAR(std::vector<cv::Point> vec) // MAR : mouth aspect ratio
{
    double a = cv::norm(cv::Mat(vec[1]), cv::Mat(vec[3]));
    double b = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[2]));
    //compute MAR
    double mar = a / b;
    return mar;
}

double DsmThread::computeGDR(std::vector<cv::Point> vec) // GDR : gaze directon ratio
{
    double a = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[1]));
    double b = cv::norm(cv::Mat(vec[1]), cv::Mat(vec[2]));
    //compute gaze direction ratio
    double gdr = a / b;
    return gdr;
}

double DsmThread::computeEA(std::vector<cv::Point> vec) // EA : eye angle
{

	cv::Point eyeMid;
	eyeMid.x = (vec[1].x + vec[2].x) / 2;
	eyeMid.y = (vec[1].y + vec[2].y) / 2;
    /* cosine */		
	double b = cv::norm(cv::Mat(vec[0]), cv::Mat(eyeMid));
	double c = cv::norm(cv::Mat(eyeMid), cv::Mat(vec[3]));
	double a = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[3]));

	double eyeAngle = acos((b*b + c*c - a*a) / (2*b*c));

	return eyeAngle;
}

double DsmThread::computeMA(std::vector<cv::Point> vec) // MA : mouth angle
{
    /* cosine */		
	double b = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[1]));
	double c = cv::norm(cv::Mat(vec[1]), cv::Mat(vec[2]));
	double a = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[2]));

	double mouthAngle = acos((b*b + c*c - a*a) / (2*b*c));

	return mouthAngle;

}

int DsmThread::destroy() {
	
}

