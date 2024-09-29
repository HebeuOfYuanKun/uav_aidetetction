#include "Config.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <json/json.h>
#include "Utils/Log.h"
#include "Utils/Version.h"

namespace AVSAnalyzer {
    Config::Config(const char* file) :
        file(file)
    {

        std::ifstream ifs(file, std::ios::binary);

        if (!ifs.is_open()) {
            LOGE("open %s error", file);
            return;
        }
        else {
            Json::CharReaderBuilder builder;
            builder["collectComments"] = true;
            JSONCPP_STRING errs;
            Json::Value root;

            if (parseFromStream(builder, ifs, &root, &errs)) {
                this->host = root["host"].asString();
                this->adminPort = root["adminPort"].asInt();
                this->adminHost = "http://" + this->host + ":" + std::to_string(this->adminPort);
                this->analyzerPort = root["analyzerPort"].asInt();
                this->mediaHttpPort = root["mediaHttpPort"].asInt();
                this->mediaRtspPort = root["mediaRtspPort"].asInt();

                this->rootDir = root["rootDir"].asString();
                this->videoFileNameFormat = root["videoFileNameFormat"].asString();
                this->workerConcurrency = root["workerConcurrency"].asInt();
                this->supportHardwareVideoDecode = root["supportHardwareVideoDecode"].asBool();
                this->supportHardwareVideoEncode = root["supportHardwareVideoEncode"].asBool();

                this->yolov8OpenVINOWeight = root["yolov8OpenVINOWeight"].asString();
                this->yolov8TensorRTWeight = root["yolov8TensorRTWeight"].asString();
                this->algorithmEngine = root["algorithmEngine"].asString();
                this->algorithmDevice = root["algorithmDevice"].asString();


                std::filesystem::path path(rootDir);
                try {
                    if (!std::filesystem::exists(path)) {
                        std::filesystem::create_directory(path);
                    }
                }
                catch (std::filesystem::filesystem_error& e) {
                    std::cout << e.what() << std::endl;
                }

                mState = true;
            }
            else {
                LOGE("parse %s error", file);
            }
            ifs.close();
        }
    }

    Config::~Config()
    {

    }

    void Config::show() {

        printf("config.file=%s\n", file);
        printf("config.host=%s\n", host.data());
        printf("config.adminPort=%d\n", adminPort);
        printf("config.analyzerPort=%d\n", analyzerPort);
        printf("config.mediaHttpPort=%d\n", mediaHttpPort);
        printf("config.mediaRtspPort=%d\n", mediaRtspPort);

        printf("config.rootDir=%s\n", rootDir.data());
        printf("config.videoFileNameFormat=%s\n", videoFileNameFormat.data());
        printf("config.workerConcurrency=%d\n", workerConcurrency);
        printf("config.supportHardwareVideoDecode=%d\n", supportHardwareVideoDecode);
        printf("config.supportHardwareVideoEncode=%d\n", supportHardwareVideoEncode);

        printf("config.yolov8OpenVINOWeight=%s\n", yolov8OpenVINOWeight.data());
        printf("config.yolov8TensorRTWeight=%s\n", yolov8TensorRTWeight.data());
        printf("config.algorithmEngine=%s\n", algorithmEngine.data());
        printf("config.algorithmDevice=%s\n", algorithmDevice.data());

    }
}