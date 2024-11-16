#ifndef ANALYZER_ALGORITHMTENSORRT_H
#define ANALYZER_ALGORITHMTENSORRT_H

#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include "Algorithm.h"

namespace AVSAnalyzer {
	class Config;
	class TensorRTEngine;

	class AlgorithmTensorRT : public Algorithm
	{
	public:
		AlgorithmTensorRT(Config* config);
		virtual ~AlgorithmTensorRT();
	public:
		virtual bool objectDetect(cv::Mat& image, std::vector<DetectObject>& detects);
	private:
		TensorRTEngine* mEngine;
	};

}
#endif //ANALYZER_ALGORITHMTENSORRT_H

