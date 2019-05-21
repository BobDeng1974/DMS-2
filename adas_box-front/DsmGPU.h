#ifndef DSMGPU_H_
#define DSMGPU_H_

#include <opencv2/opencv.hpp>
#include <inference_engine.hpp>

class DsmGPU {

public:
	DsmGPU();
	~DsmGPU();

	void initModel();
	int process(cv::Mat frame);

private:

	InferenceEngine::InputInfo::Ptr input_info;
	InferenceEngine::ExecutableNetwork executable_network;
	InferenceEngine::InferRequest infer_request;
	std::string input_name;
	std::string output_name;

};



#endif
