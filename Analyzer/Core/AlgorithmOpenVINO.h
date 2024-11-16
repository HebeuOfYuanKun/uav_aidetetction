#ifndef ANALYZER_ALGORITHMOPENVINOYOLO8_H
#define ANALYZER_ALGORITHMOPENVINOYOLO8_H

#include <string>
#include <vector>
#include "Algorithm.h"
#include <openvino/openvino.hpp> //openvino header file
#include <opencv2/opencv.hpp>    //opencv header file

namespace AVSAnalyzer {
	class Config;

	class AlgorithmOpenVINO : public Algorithm
	{
	public:
		AlgorithmOpenVINO(Config* config);
		virtual ~AlgorithmOpenVINO();
	public:
		virtual bool objectDetect(cv::Mat& image, std::vector<DetectObject>& detects);
	private:
		ov::Core* core = nullptr;
		ov::CompiledModel compiled_model;
		ov::InferRequest  infer_request;

	};


}
#endif //ANALYZER_ALGORITHMOPENVINOYOLO8_H

