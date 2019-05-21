#include "DsmGPU.h"

#define FLAGS_NUM_REQ 1
#define FLAGS_NUM_CATEGORY 2

static std::string labels[] = {
                        "openeye",
                        "closeeye"
                        };


using namespace InferenceEngine;

DsmGPU::DsmGPU()
{
}

DsmGPU::~DsmGPU()
{
}

void DsmGPU::initModel()
{

	PluginDispatcher dispatcher({"/opt/intel/computer_vision_sdk/inference_engine/lib/ubuntu_16.04/intel64", ""});
	// PluginDispatcher dispatcher({"/home/yang/intel/openvino/deployment_tools/inference_engine/lib/intel64", ""});
    InferencePlugin plugin(dispatcher.getSuitablePlugin(TargetDevice::eGPU));


	InferenceEngine::CNNNetReader netReader;
    netReader.ReadNetwork("dsmmodel.xml");
	netReader.ReadWeights("dsmmodel.bin");

	CNNNetwork network = netReader.getNetwork();

	network.setBatchSize(1);
	
	input_info = network.getInputsInfo().begin()->second;
    input_name = network.getInputsInfo().begin()->first;


    input_info->setPrecision(Precision::U8);
    input_info->setLayout(Layout::NCHW);

	DataPtr output_info = network.getOutputsInfo().begin()->second;
    output_name = network.getOutputsInfo().begin()->first;
    output_info->setPrecision(Precision::FP32);


	executable_network = plugin.LoadNetwork(network, {});

	infer_request = executable_network.CreateInferRequest();

}

int DsmGPU::process(cv::Mat frame)
{
	Blob::Ptr input = infer_request.GetBlob(input_name);
    auto input_data = input->buffer().as<PrecisionTrait<Precision::U8>::value_type *>();

	cv::resize(frame, frame, cv::Size(input_info->getTensorDesc().getDims()[3], input_info->getTensorDesc().getDims()[2]));	
	size_t channels_number = input->getTensorDesc().getDims()[1];
    size_t image_size = input->getTensorDesc().getDims()[3] * input->getTensorDesc().getDims()[2];
//	std::cout << "channels_number =  "<< channels_number << " image_size = " << input_info->getTensorDesc().getDims()[3] << std::endl;

    for (size_t pid = 0; pid < image_size; ++pid) {
        for (size_t ch = 0; ch < channels_number; ++ch) {
           	input_data[ch * image_size + pid] = frame.at<cv::Vec3b>(pid)[ch];
      	}
  	}

	infer_request.Infer();
	
	Blob::Ptr output = infer_request.GetBlob(output_name);
    auto output_data = output->buffer().as<PrecisionTrait<Precision::FP32>::value_type*>();

    std::vector<unsigned> results;
    /*  This is to sort output probabilities and put it to results vector */
    TopResults(FLAGS_NUM_CATEGORY, *output, results);

	double maxResult = 0;
	double maxId = 0;
  //  std::cout << std::endl << "Top 2 results:" << std::endl << std::endl;
    for (size_t id = 0; id < FLAGS_NUM_CATEGORY; ++id) {
       	std::cout.precision(7);
        auto result = output_data[results[id]];
 //     	std::cout << std::left << std::fixed << result << " label #" << results[id] << std::endl;
/*		if (maxResult < result) {
			maxResult = result;
			maxId = results[id];
		}	*/
   	}

	if (results.size() == 0) {
		return 0;
	}
	return results[0];
}




