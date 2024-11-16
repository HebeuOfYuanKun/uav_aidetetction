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

                this->uploadDir = root["uploadDir"].asString();
                this->openVINO = root["openVINO"].asString();
                this->onnxRuntime = root["onnxRuntime"].asString();
                this->tensorRt = root["tensorRt"].asString();

                this->algorithmApiUrl = root["algorithmApiUrl"].asString();
                this->algorithmOpenVINODetectDevice = root["algorithmOpenVINODetectDevice"].asString();
                this->algorithmOnnxRuntimeDetectDevice = root["algorithmOnnxRuntimeDetectDevice"].asString();

                std::filesystem::path path(uploadDir);
                try {
                    if (!std::filesystem::exists(path)) {
                        std::filesystem::create_directory(path);
                    }

                    mState = true;
                }
                catch (std::filesystem::filesystem_error& e) {
                    std::cout << e.what() << std::endl;
                }

              
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

        printf("config.uploadDir=%s\n", uploadDir.data());
        printf("config.openVINO=%s\n", openVINO.data());
        printf("config.tensorRt=%s\n", tensorRt.data());
        printf("config.onnxRuntime=%s\n", onnxRuntime.data());

        printf("config.algorithmApiUrl=%s\n", algorithmApiUrl.data());
        printf("config.algorithmOpenVINOFaceDetectDevice=%s\n", algorithmOpenVINODetectDevice.data());

    }
}