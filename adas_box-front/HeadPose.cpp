#include "HeadPose.h"

static double K[9] = {640, 0,  320,
0,  640,    240,
0,  0,  1};

HeadPose::HeadPose()
{
	/*mHeadPoints.push_back(P3D_SELLION);
    mHeadPoints.push_back(P3D_RIGHT_EYE);
    mHeadPoints.push_back(P3D_LEFT_EYE);
    mHeadPoints.push_back(P3D_RIGHT_EAR);
    mHeadPoints.push_back(P3D_LEFT_EAR);
    mHeadPoints.push_back(P3D_MENTON);
    mHeadPoints.push_back(P3D_NOSE);
    mHeadPoints.push_back(P3D_STOMMION);
	*/
	double scale = 10.0;
	mHeadPoints.push_back(cv::Point3d(6.825897 * scale, 6.760612 * scale, 4.402142 * scale));     //#33 left brow left corner
    mHeadPoints.push_back(cv::Point3d(1.330353 * scale, 7.122144 * scale, 6.903745 * scale));     //#29 left brow right corner
    mHeadPoints.push_back(cv::Point3d(-1.330353 * scale, 7.122144* scale, 6.903745* scale));    //#34 right brow left corner
    mHeadPoints.push_back(cv::Point3d(-6.825897* scale, 6.760612* scale, 4.402142* scale));    //#38 right brow right corner
    mHeadPoints.push_back(cv::Point3d(5.311432* scale, 5.485328* scale, 3.987654* scale));     //#13 left eye left corner
    mHeadPoints.push_back(cv::Point3d(1.789930* scale, 5.393625* scale, 4.413414* scale));     //#17 left eye right corner
    mHeadPoints.push_back(cv::Point3d(-1.789930* scale, 5.393625* scale, 4.413414* scale));    //#25 right eye left corner
    mHeadPoints.push_back(cv::Point3d(-5.311432* scale, 5.485328* scale, 3.987654* scale));    //#21 right eye right corner
    mHeadPoints.push_back(cv::Point3d(2.005628* scale, 1.409845* scale, 6.165652* scale));     //#55 nose left corner
    mHeadPoints.push_back(cv::Point3d(-2.005628* scale, 1.409845* scale, 6.165652* scale));    //#49 nose right corner
    mHeadPoints.push_back(cv::Point3d(2.774015* scale, -2.080775* scale, 5.048531* scale));    //#43 mouth left corner
    mHeadPoints.push_back(cv::Point3d(-2.774015* scale, -2.080775* scale, 5.048531* scale));   //#39 mouth right corner
    mHeadPoints.push_back(cv::Point3d(0.000000* scale, -3.116408* scale, 6.097667* scale));    //#45 mouth central bottom corner
    mHeadPoints.push_back(cv::Point3d(0.000000* scale, -7.415691* scale, 4.070434* scale));    //#6 chin corner

	mCamMatrix = cv::Mat(3, 3, CV_64FC1, K);

	int axis = 20;
    mAxis.push_back(cv::Point3d(axis, 100, 0));
    mAxis.push_back(cv::Point3d(0, axis + 100, 0));
    mAxis.push_back(cv::Point3d(0, 100, axis));
}
    
HeadPose:: ~HeadPose()
{
	mHeadPoints.clear();
}


