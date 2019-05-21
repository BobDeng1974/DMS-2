#ifndef DSMOPENVINO_H_
#define DSMOPENVINO_H_

#include <functional>
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <chrono>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <map>
#include <opencv2/opencv.hpp>
#include <inference_engine.hpp>


struct BaseDetection {
    InferenceEngine::ExecutableNetwork net;
    InferenceEngine::InferencePlugin * plugin;
    InferenceEngine::InferRequest::Ptr request;
    std::string topoName;
    std::string pathToModel;
    std::string deviceForInference;
    const int maxBatch;
    bool isBatchDynamic;
    const bool isAsync;
    mutable bool enablingChecked;
    mutable bool _enabled;

    BaseDetection(std::string topoName,
                  const std::string &pathToModel,
                  const std::string &deviceForInference,
                  int maxBatch, bool isBatchDynamic, bool isAsync);

    virtual ~BaseDetection();

    InferenceEngine::ExecutableNetwork* operator ->();
    virtual InferenceEngine::CNNNetwork read() = 0;
    virtual void submitRequest();
    virtual void wait();
    bool enabled() const;
};


struct FaceDetection : BaseDetection {
    struct Result {
        int label;
        float confidence;
        cv::Rect location;
    };

    std::string input;
    std::string output;
    double detectionThreshold;
    bool doRawOutputMessages;
    int maxProposalCount;
    int objectSize;
    int enquedFrames;
    float width;
    float height;
    const float bb_enlarge_coefficient;
	const float bb_y_coefficient;
    bool resultsFetched;
    std::vector<std::string> labels;
    std::vector<Result> results;

    FaceDetection(const std::string &pathToModel,
                  const std::string &deviceForInference,
                  int maxBatch, bool isBatchDynamic, bool isAsync,
                  double detectionThreshold, bool doRawOutputMessages);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &frame);
    void fetchResults();
};

struct HeadPoseDetection : BaseDetection {
    struct Results {
        float angle_r;
        float angle_p;
        float angle_y;
    };

    std::string input;
    std::string outputAngleR;
    std::string outputAngleP;
    std::string outputAngleY;
    int enquedFaces;
    cv::Mat cameraMatrix;

    HeadPoseDetection(const std::string &pathToModel,
                      const std::string &deviceForInference,
                      int maxBatch, bool isBatchDynamic, bool isAsync);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    Results operator[] (int idx) const;
    void buildCameraMatrix(int cx, int cy, float focalLength);
    void drawAxes(cv::Mat& frame, cv::Point3f cpoint, Results headPose, float scale);
};


struct FacialLandmarksDetection : BaseDetection {
    std::string input;
    std::string outputFacialLandmarksBlobName;
    int enquedFaces;
    std::vector<std::vector<float>> landmarks_results;
    std::vector<cv::Rect> faces_bounding_boxes;

    FacialLandmarksDetection(const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    std::vector<float> operator[] (int idx) const;
};

typedef struct BaseStatus {
	int status;
	int count;	
} basestatus;

class DsmOpenVino {

public:
	
	DsmOpenVino();
	DsmOpenVino(std::string facemodel);
	~DsmOpenVino();

	double computeMAR(std::vector<cv::Point> vec);

	void initOpenVino();
	int getInteractiveFace(cv::Mat &frame, cv::Mat &faceFrame, cv::Mat &righteyeFrame, cv::Mat &lefteyeFrame, int left, int top, double *angle, basestatus *dmsstatus);

	void initFaceSSD(std::string facemodel);
	int getFaceSSD(cv::Mat frame, cv::Mat& faceFrame, int *left, int *top,  basestatus *dmsstatus);

private:
	
	int inferPhoneSmoke(cv::Mat frame, basestatus *dmsstatus);

private:
	
	std::vector<std::string> mLabels;
	std::string mInputName;
	std::string mOutputName;
	int mMaxProposalCount;
	int mObjectSize;
	std::string mFacemodel;
	InferenceEngine::InferRequest::Ptr mAsync_infer_request_curr;
	FaceDetection *mFace;
	HeadPoseDetection *mHeadPose;
	FacialLandmarksDetection *mLandmark;


};



#endif
