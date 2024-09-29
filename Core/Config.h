#ifndef ANALYZER_CONFIG_H
#define ANALYZER_CONFIG_H

#include <string>
#include <vector>

namespace AVSAnalyzer {
	class Config
	{
	public:
		Config(const char* file);
		~Config();
	public:

		bool mState = false;
		void show();
		//void getAlgorithmHost(std::string &host);
	public:
		const char* file = NULL;

		std::string host{};//主机IP地址 192.168.1.4
		std::string adminHost{};//后台管理服务地址 http://192.168.1.4:9001
		int adminPort;// 后台管理服务端口 9001
		int analyzerPort;// 分析服务端口 9002
		int mediaHttpPort;// 80
		int mediaRtspPort;// 554

		std::string rootDir{};
		std::string videoFileNameFormat{};
		int  workerConcurrency = 0;// 支持的分析视频最大路数
		bool supportHardwareVideoDecode = false;
		bool supportHardwareVideoEncode = false;

		std::string yolov8OpenVINOWeight{};//yolov8 OpenVINO 模型地址
		std::string yolov8TensorRTWeight{};//yolov8 TensorRT 模型地址
		std::string algorithmEngine{};//选用推理引擎
		std::string algorithmDevice{};//选用推理设备

	};
}
#endif //ANALYZER_CONFIG_H