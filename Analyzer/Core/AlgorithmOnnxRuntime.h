#ifndef ANALYZER_ALGORITHMONNXYOLO8_H
#define ANALYZER_ALGORITHMONNXYOLO8_H

#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include "Algorithm.h"
#include <onnxruntime_cxx_api.h>
#include <cuda_runtime.h>




namespace AVSAnalyzer {
	class Config;
	class OnnxRuntimeEngine
	{
	public:
		explicit OnnxRuntimeEngine(std::string modelPath, std::string device);
		~OnnxRuntimeEngine();
	public:
		bool runInference(cv::Mat& image, std::vector<DetectObject>& detects);

	private:
		std::string mModelPath;
		Ort::Env mEnv{ nullptr };
		Ort::SessionOptions mSessionOptions{ nullptr };
		Ort::Session mSession{ nullptr };

	};
	class AlgorithmOnnxRuntime : public Algorithm
	{
	public:
		AlgorithmOnnxRuntime(Config* config);
		virtual ~AlgorithmOnnxRuntime();
	public:
		virtual bool objectDetect(cv::Mat& image, std::vector<DetectObject>& detects);
	private:
		OnnxRuntimeEngine* mEngine;
	};
}
#endif //ANALYZER_ALGORITHMONNXYOLO8_H

