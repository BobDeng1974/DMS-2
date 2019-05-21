#include "DsmOpenVino.h"
	
#define FLAGS_FACE_MODEL_XML			"face-detection-adas-0001.xml"
#define FLAGS_HEADPOSE_MODEL_XML		"headpose.xml"
#define FLAGS_LANDMARKS_MODEL_XML		"facial-landmarks-35-adas-0001.xml"
#define FLAGS_DEVICE					"GPU"
#define FLAGS_NUM_FACE					1
#define FLAGS_MAX_BATCHSIZE				1		
#define FLAGS_ASYNC						false

#define SMOKE_DETECTION_OUT 			1
#define FACE_DETECTION_OUT 				0
#define PHONE_DETECTION_OUT 			2

#define DMS_IMAGE_WIDTH  				1280
#define DMS_IMAGE_HEIGHT 				720

#define FACE_SCALE_X					0.2
#define FACE_SCALE_Y					0.1
#define EYE_SCALE_X						0.4
#define EYE_SCALE_Y						0.7

#define RELU(x) 						(x>0?x:0)
#define MAXWIDTH(x) 					( x>DMS_IMAGE_WIDTH?DMS_IMAGE_WIDTH:(x>0?x:0) )
#define MAXHEIGHT(y) 					( y>DMS_IMAGE_HEIGHT?DMS_IMAGE_HEIGHT:(y>0?y:0) )

#define OPENVINOLIBPATH			"/home/yang/intel/computer_vision_sdk/inference_engine/lib/ubuntu_16.04/intel64"
//#define OPENVINOLIBPATH			"../../../lib/inteal64"
// #define OPENVINOLIBPATH			"/home/yang/intel/openvino/deployment_tools/inference_engine/lib/intel64"

using namespace InferenceEngine;

template <typename T>
void matU8ToBlob(const cv::Mat &orig_image, InferenceEngine::Blob::Ptr &blob, int batchIndex = 0)
{
    InferenceEngine::SizeVector blobSize = blob->getTensorDesc().getDims();
    const size_t width = blobSize[3];
    const size_t height = blobSize[2];
    const size_t channels = blobSize[1];
    T *blob_data = blob->buffer().as<T *>();

    cv::Mat resized_image(orig_image);
    if (width != orig_image.size().width || height != orig_image.size().height)
    {
        cv::resize(orig_image, resized_image, cv::Size(width, height));
    }

    int batchOffset = batchIndex * width * height * channels;

    for (size_t c = 0; c < channels; c++)
    {
        for (size_t h = 0; h < height; h++)
        {
            for (size_t w = 0; w < width; w++)
            {
                blob_data[batchOffset + c * width * height + h * width + w] =
                    resized_image.at<cv::Vec3b>(h, w)[c];
            }
        }
    }
}

void frameToBlob(const cv::Mat& frame,
                 InferRequest::Ptr& inferRequest,
                 const std::string& inputName) {
 
    /* Resize and copy data from the image to the input blob */
    Blob::Ptr frameBlob = inferRequest->GetBlob(inputName);
    matU8ToBlob<uint8_t>(frame, frameBlob);
   
}

static std::string fileNameNoExt(const std::string &filepath) {
    auto pos = filepath.rfind('.');
    if (pos == std::string::npos) return filepath;
    return filepath.substr(0, pos);
}


BaseDetection::BaseDetection(std::string topoName,
                             const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync)
    : topoName(topoName), pathToModel(pathToModel), deviceForInference(deviceForInference),
      maxBatch(maxBatch), isBatchDynamic(isBatchDynamic), isAsync(isAsync),
      enablingChecked(false), _enabled(false) {
}

BaseDetection::~BaseDetection() {}

ExecutableNetwork* BaseDetection::operator ->() {
    return &net;
}

void BaseDetection::submitRequest() {
    if (!enabled() || request == nullptr) return;
    if (isAsync) {
        request->StartAsync();
    } else {
        request->Infer();
    }
}

void BaseDetection::wait() {
    if (!enabled()|| !request || !isAsync)
        return;
    request->Wait(IInferRequest::WaitMode::RESULT_READY);
}

bool BaseDetection::enabled() const  {
    if (!enablingChecked) {
        _enabled = !pathToModel.empty();
        if (!_enabled) {
        }
        enablingChecked = true;
    }
    return _enabled;
}


FaceDetection::FaceDetection(const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync,
                             double detectionThreshold, bool doRawOutputMessages)
    : BaseDetection("Face Detection", pathToModel, deviceForInference, maxBatch, isBatchDynamic, isAsync),
      detectionThreshold(detectionThreshold), doRawOutputMessages(doRawOutputMessages),
      enquedFrames(0), width(0), height(0), bb_enlarge_coefficient(1.2), bb_y_coefficient(1.0), resultsFetched(false) {
}

void FaceDetection::submitRequest() {
    if (!enquedFrames) return;
    enquedFrames = 0;
    resultsFetched = false;
    results.clear();
    BaseDetection::submitRequest();
}

void FaceDetection::enqueue(const cv::Mat &frame) {
    if (!enabled()) return;

    if (!request) {
        request = net.CreateInferRequestPtr();
    }

    width = frame.cols;
    height = frame.rows;

    Blob::Ptr  inputBlob = request->GetBlob(input);

    matU8ToBlob<uint8_t>(frame, inputBlob);

    enquedFrames = 1;
}

CNNNetwork FaceDetection::read()  {
    CNNNetReader netReader;
    /** Read network model **/
    netReader.ReadNetwork(pathToModel);
    /** Set batch size to 1 **/
   
    netReader.getNetwork().setBatchSize(maxBatch);
    /** Extract model name and load its weights **/
    std::string binFileName = fileNameNoExt(pathToModel) + ".bin";
    netReader.ReadWeights(binFileName);
    /** Read labels (if any)**/
    std::string labelFileName = fileNameNoExt(pathToModel) + ".labels";

    std::ifstream inputFile(labelFileName);
    std::copy(std::istream_iterator<std::string>(inputFile),
              std::istream_iterator<std::string>(),
              std::back_inserter(labels));
    // -----------------------------------------------------------------------------------------------------

    /** SSD-based network should have one input and one output **/
    // ---------------------------Check inputs -------------------------------------------------------------
   
    InputsDataMap inputInfo(netReader.getNetwork().getInputsInfo());
    if (inputInfo.size() != 1) {
        throw std::logic_error("Face Detection network should have only one input");
    }
    InputInfo::Ptr inputInfoFirst = inputInfo.begin()->second;
    inputInfoFirst->setPrecision(Precision::U8);
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
   
    OutputsDataMap outputInfo(netReader.getNetwork().getOutputsInfo());
    if (outputInfo.size() != 1) {
        throw std::logic_error("Face Detection network should have only one output");
    }
    DataPtr& _output = outputInfo.begin()->second;
    output = outputInfo.begin()->first;

    const CNNLayerPtr outputLayer = netReader.getNetwork().getLayerByName(output.c_str());
    if (outputLayer->type != "DetectionOutput") {
        throw std::logic_error("Face Detection network output layer(" + outputLayer->name +
                               ") should be DetectionOutput, but was " +  outputLayer->type);
    }

    if (outputLayer->params.find("num_classes") == outputLayer->params.end()) {
        throw std::logic_error("Face Detection network output layer (" +
                               output + ") should have num_classes integer attribute");
    }

    const int num_classes = outputLayer->GetParamAsInt("num_classes");
    if (labels.size() != num_classes) {
        if (labels.size() == (num_classes - 1))  // if network assumes default "background" class, which has no label
            labels.insert(labels.begin(), "fake");
        else
            labels.clear();
    }
    const SizeVector outputDims = _output->getTensorDesc().getDims();
    maxProposalCount = outputDims[2];
    objectSize = outputDims[3];
    if (objectSize != 7) {
        throw std::logic_error("Face Detection network output layer should have 7 as a last dimension");
    }
    if (outputDims.size() != 4) {
        throw std::logic_error("Face Detection network output dimensions not compatible shoulld be 4, but was " +
                               std::to_string(outputDims.size()));
    }
    _output->setPrecision(Precision::FP32);

    std::cout << "Loading Face Detection model to the "<< deviceForInference << " plugin" << std::endl;
    input = inputInfo.begin()->first;
    return netReader.getNetwork();
}

void FaceDetection::fetchResults() {
    if (!enabled()) return;
    results.clear();
    if (resultsFetched) return;
    resultsFetched = true;
    const float *detections = request->GetBlob(output)->buffer().as<float *>();

    for (int i = 0; i < maxProposalCount; i++) {
        float image_id = detections[i * objectSize + 0];
        Result r;
        r.label = static_cast<int>(detections[i * objectSize + 1]);
        r.confidence = detections[i * objectSize + 2];

        if (r.confidence <= detectionThreshold) {
            continue;
        }

        r.location.x = detections[i * objectSize + 3] * width;
        r.location.y = detections[i * objectSize + 4] * height;
        r.location.width = detections[i * objectSize + 5] * width - r.location.x;
        r.location.height = detections[i * objectSize + 6] * height - r.location.y;

        // Make square and enlarge face bounding box for more robust operation of face analytics networks
        int bb_width = r.location.width;
        int bb_height = r.location.height;

        int bb_center_x = r.location.x + bb_width / 2;
        int bb_center_y = r.location.y + bb_height / 2;

        int max_of_sizes = std::max(bb_width, bb_height);

        int bb_new_width = bb_enlarge_coefficient * max_of_sizes;
        int bb_new_height = bb_enlarge_coefficient * max_of_sizes;

        r.location.x = bb_center_x - bb_new_width / 2;
        r.location.y = bb_center_y - bb_y_coefficient * bb_new_height / 2;

        r.location.width = bb_new_width;
        r.location.height = bb_new_height;

        if (image_id < 0) {
            break;
        }
        if (doRawOutputMessages) {
            std::cout << "[" << i << "," << r.label << "] element, prob = " << r.confidence <<
                         "    (" << r.location.x << "," << r.location.y << ")-(" << r.location.width << ","
                      << r.location.height << ")"
                      << ((r.confidence > detectionThreshold) ? " WILL BE RENDERED!" : "") << std::endl;
        }

        results.push_back(r);
    }
}

HeadPoseDetection::HeadPoseDetection(const std::string &pathToModel,
                                     const std::string &deviceForInference,
                                     int maxBatch, bool isBatchDynamic, bool isAsync)
    : BaseDetection("Head Pose", pathToModel, deviceForInference, maxBatch, isBatchDynamic, isAsync),
      outputAngleR("angle_r_fc"), outputAngleP("angle_p_fc"), outputAngleY("angle_y_fc"), enquedFaces(0) {
}

void HeadPoseDetection::submitRequest()  {
    if (!enquedFaces) return;
    if (isBatchDynamic) {
        request->SetBatch(enquedFaces);
    }
    BaseDetection::submitRequest();
    enquedFaces = 0;
}

void HeadPoseDetection::enqueue(const cv::Mat &face) {
    if (!enabled()) {
        return;
    }
    if (enquedFaces == maxBatch) {
        return;
    }
    if (!request) {
        request = net.CreateInferRequestPtr();
    }

    Blob::Ptr inputBlob = request->GetBlob(input);

    matU8ToBlob<uint8_t>(face, inputBlob, enquedFaces);

    enquedFaces++;
}

HeadPoseDetection::Results HeadPoseDetection::operator[] (int idx) const {
    Blob::Ptr  angleR = request->GetBlob(outputAngleR);
    Blob::Ptr  angleP = request->GetBlob(outputAngleP);
    Blob::Ptr  angleY = request->GetBlob(outputAngleY);

    return {angleR->buffer().as<float*>()[idx],
                angleP->buffer().as<float*>()[idx],
                angleY->buffer().as<float*>()[idx]};
}

CNNNetwork HeadPoseDetection::read() {
    CNNNetReader netReader;
    // Read network model
    netReader.ReadNetwork(pathToModel);
    // Set maximum batch size
    netReader.getNetwork().setBatchSize(maxBatch);
    // Extract model name and load its weights
    std::string binFileName = fileNameNoExt(pathToModel) + ".bin";
    netReader.ReadWeights(binFileName);

    // ---------------------------Check inputs -------------------------------------------------------------
    InputsDataMap inputInfo(netReader.getNetwork().getInputsInfo());
    if (inputInfo.size() != 1) {
        throw std::logic_error("Head Pose Estimation network should have only one input");
    }
    InputInfo::Ptr& inputInfoFirst = inputInfo.begin()->second;
    inputInfoFirst->setPrecision(Precision::U8);
    input = inputInfo.begin()->first;
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
    OutputsDataMap outputInfo(netReader.getNetwork().getOutputsInfo());
    if (outputInfo.size() != 3) {
        throw std::logic_error("Head Pose Estimation network should have 3 outputs");
    }
    for (auto& output : outputInfo) {
        output.second->setPrecision(Precision::FP32);
    }
    std::map<std::string, bool> layerNames = {
        {outputAngleR, false},
        {outputAngleP, false},
        {outputAngleY, false}
    };

    for (auto && output : outputInfo) {
        CNNLayerPtr layer = output.second->getCreatorLayer().lock();
        if (!layer) {
            throw std::logic_error("Layer pointer is invalid");
        }
        if (layerNames.find(layer->name) == layerNames.end()) {
            throw std::logic_error("Head Pose Estimation network output layer unknown: " + layer->name + ", should be " +
                                   outputAngleR + " or " + outputAngleP + " or " + outputAngleY);
        }
        if (layer->type != "FullyConnected") {
            throw std::logic_error("Head Pose Estimation network output layer (" + layer->name + ") has invalid type: " +
                                   layer->type + ", should be FullyConnected");
        }
        auto fc = dynamic_cast<FullyConnectedLayer*>(layer.get());
        if (!fc) {
            throw std::logic_error("Fully connected layer is not valid");
        }
        if (fc->_out_num != 1) {
            throw std::logic_error("Head Pose Estimation network output layer (" + layer->name + ") has invalid out-size=" +
                                   std::to_string(fc->_out_num) + ", should be 1");
        }
        layerNames[layer->name] = true;
    }

    std::cout << "Loading Head Pose Estimation model to the "<< deviceForInference << " plugin" << std::endl;

    _enabled = true;
    return netReader.getNetwork();
}

void HeadPoseDetection::buildCameraMatrix(int cx, int cy, float focalLength) {
    if (!cameraMatrix.empty()) return;
    cameraMatrix = cv::Mat::zeros(3, 3, CV_32F);
    cameraMatrix.at<float>(0) = focalLength;
    cameraMatrix.at<float>(2) = static_cast<float>(cx);
    cameraMatrix.at<float>(4) = focalLength;
    cameraMatrix.at<float>(5) = static_cast<float>(cy);
    cameraMatrix.at<float>(8) = 1;
}

void HeadPoseDetection::drawAxes(cv::Mat& frame, cv::Point3f cpoint, Results headPose, float scale) {
    double yaw   = headPose.angle_y;
    double pitch = headPose.angle_p;
    double roll  = headPose.angle_r;

    pitch *= CV_PI / 180.0;
    yaw   *= CV_PI / 180.0;
    roll  *= CV_PI / 180.0;

    cv::Matx33f        Rx(1,           0,            0,
                          0,  cos(pitch),  -sin(pitch),
                          0,  sin(pitch),  cos(pitch));
    cv::Matx33f Ry(cos(yaw),           0,    -sin(yaw),
                   0,           1,            0,
                   sin(yaw),           0,    cos(yaw));
    cv::Matx33f Rz(cos(roll), -sin(roll),            0,
                   sin(roll),  cos(roll),            0,
                   0,           0,            1);


    auto r = cv::Mat(Rz*Ry*Rx);
    buildCameraMatrix(frame.cols / 2, frame.rows / 2, 950.0);

    cv::Mat xAxis(3, 1, CV_32F), yAxis(3, 1, CV_32F), zAxis(3, 1, CV_32F), zAxis1(3, 1, CV_32F);

    xAxis.at<float>(0) = 1 * scale;
    xAxis.at<float>(1) = 0;
    xAxis.at<float>(2) = 0;

    yAxis.at<float>(0) = 0;
    yAxis.at<float>(1) = -1 * scale;
    yAxis.at<float>(2) = 0;

    zAxis.at<float>(0) = 0;
    zAxis.at<float>(1) = 0;
    zAxis.at<float>(2) = -1 * scale;

    zAxis1.at<float>(0) = 0;
    zAxis1.at<float>(1) = 0;
    zAxis1.at<float>(2) = 1 * scale;

    cv::Mat o(3, 1, CV_32F, cv::Scalar(0));
    o.at<float>(2) = cameraMatrix.at<float>(0);

    xAxis = r * xAxis + o;
    yAxis = r * yAxis + o;
    zAxis = r * zAxis + o;
    zAxis1 = r * zAxis1 + o;

    cv::Point p1, p2;

    p2.x = static_cast<int>((xAxis.at<float>(0) / xAxis.at<float>(2) * cameraMatrix.at<float>(0)) + cpoint.x);
    p2.y = static_cast<int>((xAxis.at<float>(1) / xAxis.at<float>(2) * cameraMatrix.at<float>(4)) + cpoint.y);
    cv::line(frame, cv::Point(cpoint.x, cpoint.y), p2, cv::Scalar(0, 0, 255), 2);

    p2.x = static_cast<int>((yAxis.at<float>(0) / yAxis.at<float>(2) * cameraMatrix.at<float>(0)) + cpoint.x);
    p2.y = static_cast<int>((yAxis.at<float>(1) / yAxis.at<float>(2) * cameraMatrix.at<float>(4)) + cpoint.y);
    cv::line(frame, cv::Point(cpoint.x, cpoint.y), p2, cv::Scalar(0, 255, 0), 2);

    p1.x = static_cast<int>((zAxis1.at<float>(0) / zAxis1.at<float>(2) * cameraMatrix.at<float>(0)) + cpoint.x);
    p1.y = static_cast<int>((zAxis1.at<float>(1) / zAxis1.at<float>(2) * cameraMatrix.at<float>(4)) + cpoint.y);

    p2.x = static_cast<int>((zAxis.at<float>(0) / zAxis.at<float>(2) * cameraMatrix.at<float>(0)) + cpoint.x);
    p2.y = static_cast<int>((zAxis.at<float>(1) / zAxis.at<float>(2) * cameraMatrix.at<float>(4)) + cpoint.y);
    cv::line(frame, p1, p2, cv::Scalar(255, 0, 0), 2);
    cv::circle(frame, p2, 3, cv::Scalar(255, 0, 0), 2);
}

struct Load {
    BaseDetection& detector;
    explicit Load(BaseDetection& detector) : detector(detector) { }

    void into(InferencePlugin & plg, bool enable_dynamic_batch = false) const {
        if (detector.enabled()) {
            std::map<std::string, std::string> config;
            if (enable_dynamic_batch) {
                config[PluginConfigParams::KEY_DYN_BATCH_ENABLED] = PluginConfigParams::YES;
            }
            detector.net = plg.LoadNetwork(detector.read(), config);
            detector.plugin = &plg;
        }
    }
};

FacialLandmarksDetection::FacialLandmarksDetection(const std::string &pathToModel,
                                                   const std::string &deviceForInference,
                                                   int maxBatch, bool isBatchDynamic, bool isAsync)
    : BaseDetection("Facial Landmarks", pathToModel, deviceForInference, maxBatch, isBatchDynamic, isAsync),
      outputFacialLandmarksBlobName("align_fc3"), enquedFaces(0) {
}

void FacialLandmarksDetection::submitRequest() {
    if (!enquedFaces) return;
    if (isBatchDynamic) {
        request->SetBatch(enquedFaces);
    }
    BaseDetection::submitRequest();
    enquedFaces = 0;
}

void FacialLandmarksDetection::enqueue(const cv::Mat &face) {
    if (!enabled()) {
        return;
    }
    if (enquedFaces == maxBatch) {
        return;
    }
    if (!request) {
        request = net.CreateInferRequestPtr();
    }

    Blob::Ptr inputBlob = request->GetBlob(input);

    matU8ToBlob<uint8_t>(face, inputBlob, enquedFaces);

    enquedFaces++;
}

std::vector<float> FacialLandmarksDetection::operator[] (int idx) const {
    std::vector<float> normedLandmarks;

    auto landmarksBlob = request->GetBlob(outputFacialLandmarksBlobName);
    auto n_lm = landmarksBlob->dims()[0];
    const float *normed_coordinates = request->GetBlob(outputFacialLandmarksBlobName)->buffer().as<float *>();

    for (auto i = 0UL; i < n_lm; ++i)
        normedLandmarks.push_back(normed_coordinates[i + n_lm * idx]);

    return normedLandmarks;
}

CNNNetwork FacialLandmarksDetection::read() {
    CNNNetReader netReader;
    // Read network model
    netReader.ReadNetwork(pathToModel);
    // Set maximum batch size
    netReader.getNetwork().setBatchSize(maxBatch);
    // Extract model name and load its weights
    std::string binFileName = fileNameNoExt(pathToModel) + ".bin";
    netReader.ReadWeights(binFileName);

    // ---------------------------Check inputs -------------------------------------------------------------
    InputsDataMap inputInfo(netReader.getNetwork().getInputsInfo());
    if (inputInfo.size() != 1) {
        throw std::logic_error("Facial Landmarks Estimation network should have only one input");
    }
    InputInfo::Ptr& inputInfoFirst = inputInfo.begin()->second;
    inputInfoFirst->setPrecision(Precision::U8);
    input = inputInfo.begin()->first;
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
    OutputsDataMap outputInfo(netReader.getNetwork().getOutputsInfo());
    if (outputInfo.size() != 1) {
        throw std::logic_error("Facial Landmarks Estimation network should have only one output");
    }
    for (auto& output : outputInfo) {
        output.second->setPrecision(Precision::FP32);
    }
    std::map<std::string, bool> layerNames = {
        {outputFacialLandmarksBlobName, false}
    };

    for (auto && output : outputInfo) {
        CNNLayerPtr layer = output.second->getCreatorLayer().lock();
        if (!layer) {
            throw std::logic_error("Layer pointer is invalid");
        }
        if (layerNames.find(layer->name) == layerNames.end()) {
            throw std::logic_error("Facial Landmarks Estimation network output layer unknown: " + layer->name + ", should be " +
                                   outputFacialLandmarksBlobName);
        }
        if (layer->type != "FullyConnected") {
            throw std::logic_error("Facial Landmarks Estimation network output layer (" + layer->name + ") has invalid type: " +
                                   layer->type + ", should be FullyConnected");
        }
        auto fc = dynamic_cast<FullyConnectedLayer*>(layer.get());
        if (!fc) {
            throw std::logic_error("Fully connected layer is not valid");
        }
        if (fc->_out_num != 70) {
            throw std::logic_error("Facial Landmarks Estimation network output layer (" + layer->name + ") has invalid out-size=" +
                                   std::to_string(fc->_out_num) + ", should be 70");
        }
        layerNames[layer->name] = true;
    }

    _enabled = true;
    return netReader.getNetwork();
}

DsmOpenVino::DsmOpenVino()
{
	mFace = new FaceDetection(FLAGS_FACE_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false, 0.6, false);
	mHeadPose = new HeadPoseDetection(FLAGS_HEADPOSE_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false);
	mLandmark = new FacialLandmarksDetection(FLAGS_LANDMARKS_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false);
}

DsmOpenVino::DsmOpenVino(std::string facemodel)
{

	mFacemodel = facemodel;

	mFace = new FaceDetection(FLAGS_FACE_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false, 0.6, false);
	mHeadPose = new HeadPoseDetection(FLAGS_HEADPOSE_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false);
	mLandmark = new FacialLandmarksDetection(FLAGS_LANDMARKS_MODEL_XML, FLAGS_DEVICE, FLAGS_MAX_BATCHSIZE, false, false);
}


DsmOpenVino::~DsmOpenVino()
{
	if (mFace != NULL)
		delete mFace;
	if (mHeadPose != NULL)
		delete mHeadPose;
	if (mLandmark != NULL)
		delete mLandmark;

}

void DsmOpenVino::initOpenVino()
{

	PluginDispatcher dispatcher({OPENVINOLIBPATH, ""});
    InferencePlugin plugin(dispatcher.getSuitablePlugin(TargetDevice::eGPU));

	Load(*mFace).into(plugin, false);
	Load(*mHeadPose).into(plugin, false);
	Load(*mLandmark).into(plugin, false);

}

double DsmOpenVino::computeMAR(std::vector<cv::Point> vec) // MAR : mouth aspect ratio
{
    double a = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[1]));
    double b = cv::norm(cv::Mat(vec[2]), cv::Mat(vec[3]));
    //compute MAR
    double mar = b / a;
    return mar;
}

int DsmOpenVino::getInteractiveFace(cv::Mat &frame, cv::Mat &faceFrame, cv::Mat &righteyeFrame, cv::Mat &lefteyeFrame, 
			int left, int top, double *angle, basestatus *dmsstatus) {

	if (mHeadPose == NULL) {
		return 0;
	}
/*
	cv::Mat eyedetectframe;
   	frame.copyTo(eyedetectframe);

	const double mouth_threshold = 0.5;
	const double headpose_threshold = 30.0;

	mFace->enqueue(frame);
	mFace->submitRequest();
	mFace->wait();
	mFace->fetchResults();
    auto prev_detection_results = mFace->results;

	int dmsfacearea = 0;
	cv::Mat faceMat;
	int faceleft = 0;
	int facetop = 0;

	cv::Mat facePhoneMat;
	const float bb_enlarge = 1.8;
	const float bb_y = 0.8;

	for (auto &&face : prev_detection_results) {
        
        auto clippedRect = face.location & cv::Rect(0, 0, DMS_IMAGE_WIDTH, DMS_IMAGE_HEIGHT);
         
        cv::rectangle(frame, face.location,  cv::Scalar(255, 255, 0), 1);
		int facewidth = face.location.width;
		int faceheight = face.location.height;
		int facearea = facewidth * faceheight;

		if (dmsfacearea < facearea) {
			dmsfacearea = facearea;
			faceMat = frame(clippedRect);
			faceleft = face.location.x;
			facetop = face.location.y;

			cv::Rect facephone;
			int bb_width = clippedRect.width;
		    int bb_height = clippedRect.height;

		    int bb_center_x = clippedRect.x + bb_width / 2;
		    int bb_center_y = clippedRect.y + bb_height / 2;

		    int max_of_sizes = std::max(bb_width, bb_height);

		    int bb_new_width = bb_enlarge * max_of_sizes;
		    int bb_new_height = bb_enlarge * max_of_sizes;

		    facephone.x = RELU( bb_center_x - bb_new_width / 2);
		    facephone.y = RELU( bb_center_y - (float)bb_y * bb_new_height / 2);
			int rb_x = MAXWIDTH(facephone.x + bb_new_width);
			int rb_y = MAXHEIGHT(facephone.y + bb_new_height);
		    facephone.width = rb_x - facephone.x;
		    facephone.height = rb_y - facephone.y;
			auto facephoneRect = facephone & cv::Rect(0, 0, DMS_IMAGE_WIDTH, DMS_IMAGE_HEIGHT);
			facePhoneMat = frame(facephoneRect);
			//cv::rectangle(frame, facephoneRect,  cv::Scalar(255, 0, 255), 1);
		}
        
    }
*/
//	if (faceMat.empty()) 
//	{	
//			return 0;
//	}

	inferPhoneSmoke(frame, dmsstatus);

	return 1;
/*	int width = faceMat.cols;
	int height = faceMat.rows;

	mHeadPose->enqueue(faceMat);
	mLandmark->enqueue(faceMat);
	
	mHeadPose->submitRequest();
	mLandmark->submitRequest();

	mHeadPose->wait();
	mLandmark->wait();
*/
	/*std::cout << "Head pose results: yaw, pitch, roll = "
                                  << (*mHeadPose)[0].angle_y << ";"
                                  << (*mHeadPose)[0].angle_p << ";"
                                  << (*mHeadPose)[0].angle_r << std::endl;
	*/
/*
	angle[0] = (*mHeadPose)[0].angle_y;
	angle[1] = (*mHeadPose)[0].angle_p;
	angle[2] = (*mHeadPose)[0].angle_r;
	
	cv::Point3f center(faceleft + width / 2, facetop + height / 2, 0);
	mHeadPose->drawAxes(frame, center, (*mHeadPose)[0], 50);

	if(fabs(angle[0]) >= headpose_threshold || fabs(angle[1]) >= headpose_threshold) {
		dmsstatus[3].status = 1;
		++dmsstatus[3].count;
		dmsstatus[4].status = 0;
		dmsstatus[4].count = 0;
		dmsstatus[5].status = 0;
		dmsstatus[5].count = 0;
		return 0;
	} else {
		dmsstatus[3].status = 0;
		dmsstatus[3].count = 0;
	}

	std::vector<cv::Point> righteye;
	std::vector<cv::Point> lefteye;
	std::vector<cv::Point> mouth;
	cv::Point p;
    auto normed_landmarks = (*mLandmark)[0];
    auto n_lm = normed_landmarks.size();
    for (auto i_lm = 0UL; i_lm < n_lm / 2; ++i_lm) {
        float normed_x = normed_landmarks[2 * i_lm];
        float normed_y = normed_landmarks[2 * i_lm + 1];

        int x_lm = faceleft + width * normed_x;
        int y_lm = facetop + height * normed_y;

		if (i_lm == 0 || i_lm == 1) {
			p.x = x_lm;
			p.y = y_lm;
			righteye.push_back(p);
		}
		if (i_lm == 2 || i_lm == 3) {
			p.x = x_lm;
			p.y = y_lm;
			lefteye.push_back(p);
		}

		if (i_lm == 8 || i_lm == 9 || i_lm == 10 || i_lm == 11) {
			p.x = x_lm;
			p.y = y_lm;
			mouth.push_back(p);
		}
        // Drawing facial landmarks on the frame
        cv::circle(frame, cv::Point(x_lm, y_lm), 1 + static_cast<int>(0.012 * width), cv::Scalar(255, 255, 255), -1);
    }
    
	
	//Compute Mouth aspect ration for mouth
    double mouthAr = computeMAR(mouth);     
	if (mouthAr >= mouth_threshold) {
		dmsstatus[4].status = 1;
		++dmsstatus[4].count;
	} else {
		dmsstatus[4].status = 0;
		dmsstatus[4].count = 0;
	}

	cv::Rect righteyeRoi;
	int eyewidth = (int)cv::norm(cv::Mat(righteye[0]), cv::Mat(righteye[1]));
	righteyeRoi.x = RELU(righteye[1].x - eyewidth * EYE_SCALE_X);
	righteyeRoi.y = RELU(righteye[1].y - eyewidth * EYE_SCALE_Y);
	int rb_x = MAXWIDTH(righteye[0].x + eyewidth * EYE_SCALE_X);
	int rb_y = MAXHEIGHT(righteye[0].y + eyewidth * EYE_SCALE_Y);
	righteyeRoi.width = rb_x - righteyeRoi.x;
	righteyeRoi.height = rb_y - righteyeRoi.y;
	cv::Mat temprighteyeframe(eyedetectframe, righteyeRoi);
	righteyeFrame = temprighteyeframe;	
	cv::rectangle(frame, righteyeRoi, cv::Scalar(0, 255, 255));

	cv::Rect lefteyeRoi;
	eyewidth = (int)cv::norm(cv::Mat(lefteye[0]), cv::Mat(lefteye[1]));
	lefteyeRoi.x = RELU(lefteye[0].x - eyewidth * EYE_SCALE_X);
	lefteyeRoi.y = RELU(lefteye[0].y - eyewidth * EYE_SCALE_Y);
	rb_x = MAXWIDTH(lefteye[1].x + eyewidth * EYE_SCALE_X);
	rb_y = MAXHEIGHT(lefteye[1].y + eyewidth * EYE_SCALE_Y);
	lefteyeRoi.width = rb_x - lefteyeRoi.x;
	lefteyeRoi.height = rb_y - lefteyeRoi.y;
	cv::Mat templefteyeframe(eyedetectframe, lefteyeRoi);
	lefteyeFrame = templefteyeframe;	
	cv::rectangle(frame, lefteyeRoi, cv::Scalar(0, 255, 255));

//	std::cout << "mouthAr : " << mouthAr << "    mouth size : " << mouth.size() << std::endl;

	return 1;
*/
}

void DsmOpenVino::initFaceSSD(std::string facemodel)
{
	//PluginDispatcher dispatcher({OPENVINOLIBPATH, ""});
    	//InferencePlugin plugin(dispatcher.getSuitablePlugin(TargetDevice::eGPU));
        InferencePlugin plugin = PluginDispatcher({OPENVINOLIBPATH, ""}).getPluginByDevice("GPU");
	CNNNetReader netReader;
        /** Read network model **/
        netReader.ReadNetwork(facemodel.c_str());
        /** Set batch size to 1 **/
        netReader.getNetwork().setBatchSize(1);
        /** Extract model name and load it's weights **/
        std::string binFileName = fileNameNoExt(facemodel.c_str()) + ".bin";
        netReader.ReadWeights(binFileName);
        /** Read labels (if any)**/
        std::string labelFileName = fileNameNoExt(facemodel.c_str()) + ".labels";
        std::ifstream inputFile(labelFileName);
        std::copy(std::istream_iterator<std::string>(inputFile),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(mLabels));
        // -----------------------------------------------------------------------------------------------------

        /** SSD-based network should have one input and one output **/
        // --------------------------- Configure input & output ---------------------------------------------
        // --------------------------- Prepare input blobs -----------------------------------------------------
        InputsDataMap inputInfo(netReader.getNetwork().getInputsInfo());
        if (inputInfo.size() != 1) {
            std::cout << "Error::This demo accepts networks having only one input" << std::endl;
        }
        InputInfo::Ptr& input = inputInfo.begin()->second;
        mInputName = inputInfo.begin()->first;
        input->setPrecision(Precision::U8);     
        input->getInputData()->setLayout(Layout::NCHW);
       
        // --------------------------- Prepare output blobs -----------------------------------------------------
        OutputsDataMap outputInfo(netReader.getNetwork().getOutputsInfo());
        if (outputInfo.size() != 1) {
            std::cout << "Error::This demo accepts networks having only one output" << std::endl;
        }
        DataPtr& output = outputInfo.begin()->second;
       	mOutputName = outputInfo.begin()->first;
        const int num_classes = netReader.getNetwork().getLayerByName(mOutputName.c_str())->GetParamAsInt("num_classes");
		std::cout << "num_class :" << num_classes << " labelsize :" << mLabels.size() << std::endl;
	
        if (mLabels.size() != num_classes) {
            if (mLabels.size() == (num_classes - 1))  // if network assumes default "background" class, having no label
                mLabels.insert(mLabels.begin(), "fake");
            else
                mLabels.clear();
        }
        const SizeVector outputDims = output->getTensorDesc().getDims();
        mMaxProposalCount = outputDims[2];
        mObjectSize = outputDims[3];
	std::cout << "mObjectSize = " << mObjectSize << std::endl;
        if (mObjectSize != 7) {
            throw std::logic_error("Output should have 7 as a last dimension");
        }
        if (outputDims.size() != 4) {
            throw std::logic_error("Incorrect output dimensions for SSD");
        }
        output->setPrecision(Precision::FP32);
        output->setLayout(Layout::NCHW);
        // -----------------------------------------------------------------------------------------------------  
        // --------------------------- Loading model to the plugin ------------------------------------------
        ExecutableNetwork network = plugin.LoadNetwork(netReader.getNetwork(), {});
        // -----------------------------------------------------------------------------------------------------

        // --------------------------- Create infer request -------------------------------------------------
        InferRequest::Ptr async_infer_request_next = network.CreateInferRequestPtr();
        mAsync_infer_request_curr = network.CreateInferRequestPtr();
        // -----------------------------------------------------------------------------------------------------
		std::cout << "Loading Face Phone Smoke Detection model to the "<< FLAGS_DEVICE << " plugin" << std::endl;
}

int DsmOpenVino::inferPhoneSmoke(cv::Mat frame, basestatus *dmsstatus)
{
	int isphone = 0;
	if (frame.empty()) 
	{
		return isphone;
	}
	const float FLAGS_t = 0.8;
	const size_t width  = (size_t)frame.cols;
    const size_t height = (size_t)frame.rows;
	frameToBlob(frame, mAsync_infer_request_curr, mInputName);
	mAsync_infer_request_curr->StartAsync();
	if (OK == mAsync_infer_request_curr->Wait(IInferRequest::WaitMode::RESULT_READY)) {
        // ---------------------------Process output blobs--------------------------------------------------
        // Processing results of the CURRENT request
        const float *detections = mAsync_infer_request_curr->GetBlob(mOutputName)->buffer().as<PrecisionTrait<Precision::FP32>::value_type*>();
        for (int i = 0; i < mMaxProposalCount; i++) {
            float image_id = detections[i * mObjectSize + 0];
            int label = static_cast<int>(detections[i * mObjectSize + 1]);
            float confidence = detections[i * mObjectSize + 2];
            float xmin = detections[i * mObjectSize + 3] * width;
            float ymin = detections[i * mObjectSize + 4] * height;
            float xmax = detections[i * mObjectSize + 5] * width;
            float ymax = detections[i * mObjectSize + 6] * height;

            std::cout << "Only " << i << " proposals found" << "   maxnum = " << mMaxProposalCount << std::endl;
            std::cout << "image_id " << image_id << "   label = " << label << " confidence = " << confidence << std::endl;
            if (image_id < 0) {
             //   std::cout << "Only " << i << " proposals found" << std::endl;
                return isphone;
            }
  
            if (confidence > FLAGS_t) {
				if(label == SMOKE_DETECTION_OUT) {
					dmsstatus[2].status = 1;
					++dmsstatus[2].count;
					dmsstatus[1].status = 0;
					dmsstatus[1].count = 0;
					isphone = 1;
					std::cout << "SMOKE**************" << std::endl;
					cv::rectangle(frame, cv::Point2f(xmin, ymin), cv::Point2f(xmax, ymax), cv::Scalar(255, 0, 255));
				}
				else if(label == PHONE_DETECTION_OUT) {
					std::cout << "PHONE==========" << std::endl;
					dmsstatus[1].status = 1;
					++dmsstatus[1].count;
					dmsstatus[2].status = 0;
					dmsstatus[2].count = 0;
					isphone = 1;
					cv::rectangle(frame, cv::Point2f(xmin, ymin), cv::Point2f(xmax, ymax), cv::Scalar(0, 255, 255));
				} else {
					isphone = 0;				
				}
            }
        }
    }


    return isphone;
}

int DsmOpenVino::getFaceSSD(cv::Mat frame, cv::Mat& faceFrame, int *left, int *top, basestatus *dmsstatus)
{
	int isFindFace = 0;
	std::vector<cv::Mat> faces;
	if (frame.empty()) 
	{
		return isFindFace;
	}
	const float FLAGS_t = 0.5;
	const size_t width  = (size_t)frame.cols;
    const size_t height = (size_t)frame.rows;
	frameToBlob(frame, mAsync_infer_request_curr, mInputName);
	mAsync_infer_request_curr->StartAsync();
	int dmsfacearea = 0;
	int dmsfaceindex = 0;
	if (OK == mAsync_infer_request_curr->Wait(IInferRequest::WaitMode::RESULT_READY)) {
        // ---------------------------Process output blobs--------------------------------------------------
        // Processing results of the CURRENT request
        const float *detections = mAsync_infer_request_curr->GetBlob(mOutputName)->buffer().as<PrecisionTrait<Precision::FP32>::value_type*>();
        for (int i = 0; i < mMaxProposalCount; i++) {
            float image_id = detections[i * mObjectSize + 0];
            int label = static_cast<int>(detections[i * mObjectSize + 1]);
            float confidence = detections[i * mObjectSize + 2];
            float xmin = detections[i * mObjectSize + 3] * width;
            float ymin = detections[i * mObjectSize + 4] * height;
            float xmax = detections[i * mObjectSize + 5] * width;
            float ymax = detections[i * mObjectSize + 6] * height;

            if (image_id < 0) {
               // std::cout << "Only " << i << " proposals found" << std::endl;
                return isFindFace;
            }
  
            if (confidence > FLAGS_t) {

				if (label == FACE_DETECTION_OUT) {
					continue;				
				}
                /** Drawing only objects when > confidence_threshold probability **/
              /*  std::ostringstream conf;
                conf << ":" << std::fixed << std::setprecision(3) << confidence;
                cv::putText(frame,
                            (label < mLabels.size() ? mLabels[label] : std::string("label #") + std::to_string(label))
                            + conf.str(),
                            cv::Point2f(xmin, ymin - 5), cv::FONT_HERSHEY_COMPLEX_SMALL, 1,
                            cv::Scalar(0, 0, 255));*/
//                cv::rectangle(frame, cv::Point2f(xmin, ymin), cv::Point2f(xmax, ymax), cv::Scalar(0, 255, 0));
				
				
				if (label == FACE_DETECTION_OUT) {
					//faces.push_back(faceframe);
					isFindFace = 1;
					int facewidth = xmax-xmin;
					int faceheight = ymax - ymin;
					int facearea = facewidth * faceheight;
					if (dmsfacearea < facearea) {
						dmsfacearea = facearea;
					//	cv::Rect faceRoi(xmin, ymin, xmax-xmin, ymax-ymin);
						int facewidth = xmax-xmin;
						cv::Rect scalefaceRoi;
						scalefaceRoi.x = RELU(xmin - facewidth * FACE_SCALE_X);
						scalefaceRoi.y = RELU(ymin - faceheight * FACE_SCALE_Y);
						int rb_x = MAXWIDTH(xmin + facewidth * ( 1 + FACE_SCALE_X));
						int rb_y = MAXHEIGHT(ymin + faceheight * ( 1 + FACE_SCALE_Y));
						scalefaceRoi.width = rb_x - scalefaceRoi.x;
						scalefaceRoi.height = rb_y - scalefaceRoi.y;
						cv::Mat tempfaceframe(frame, scalefaceRoi);
						faceFrame = tempfaceframe;
						*left = scalefaceRoi.x;
						*top = scalefaceRoi.y;
						//cv::rectangle(frame, scalefaceRoi, cv::Scalar(255, 0, 0));
					}
				}
				else if(label == SMOKE_DETECTION_OUT) {
					dmsstatus[2].status = 1;
					++dmsstatus[2].count;
					dmsstatus[1].status = 0;
					dmsstatus[1].count = 0;
				}
				else if(label == PHONE_DETECTION_OUT) {
					dmsstatus[1].status = 1;
					++dmsstatus[1].count;
					dmsstatus[2].status = 0;
					dmsstatus[2].count = 0;
				} else {}
            }
        }
    }


    return isFindFace;

}
