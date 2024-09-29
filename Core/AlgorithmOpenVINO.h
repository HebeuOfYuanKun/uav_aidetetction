#ifndef ANALYZER_ALGORITHMOPENVINO_H
#define ANALYZER_ALGORITHMOPENVINO_H

#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <openvino/openvino.hpp> //openvino header file
#include "Algorithm.h"

namespace AVSAnalyzer {
	class Config;

	class AlgorithmOpenVINO : public Algorithm
	{
	public:
		AlgorithmOpenVINO(Config* config);
		virtual ~AlgorithmOpenVINO();
	public:
		virtual bool objectDetect(int height, int width, cv::Mat& image, std::vector<DetectObject>& detects, std::vector<int>& count_category);
	private:

		ov::Core* core;
		ov::CompiledModel compiled_model;
		ov::InferRequest  infer_request;
	};


}
#endif //ANALYZER_ALGORITHMOPENVINO_H

