#include "Analyzer.h"
#include "Algorithm.h"
#include <json/json.h>
#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/Request.h"
#include "Utils/Base64.h"
#include "Utils/CalcuIOU.h"
#include <fstream>

namespace AVSAnalyzer {

    Analyzer::Analyzer(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(control)
    {
  
    }

    Analyzer::~Analyzer()
    {


    }

    bool Analyzer::handleVideoFrame(int64_t frameCount, cv::Mat& image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore) {

        if (mControl->modelCode == "onnxruntime_yolo8") {
            //v3.43新增，基于onnxruntime的yolo8检测

            if (mScheduler->algorithm_onnx_yolo8_mtx.try_lock()) {
                if (mScheduler->algorithm_onnx_yolo8) {
                    happenDetects.clear();
                    happen = false;
                    happenScore = 0;
                    mScheduler->algorithm_onnx_yolo8->objectDetect(image, happenDetects);
                }
                mScheduler->algorithm_onnx_yolo8_mtx.unlock();
            }
            if(!happenDetects.empty()){

                //cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 0, 255), 4, 8);//绘制多边形
                int x1, y1, x2, y2;
                int matchCount = 0;
                for (int i = 0; i < happenDetects.size(); i++)
                {
                    x1 = happenDetects[i].x1;
                    y1 = happenDetects[i].y1;
                    x2 = happenDetects[i].x2;
                    y2 = happenDetects[i].y2;

                    std::vector<double> object_d;
                    object_d.push_back(x1);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y2);

                    object_d.push_back(x1);
                    object_d.push_back(y2);


                    double iou = CalcuPolygonIOU(mControl->recognitionRegion_d, object_d);

                    if (iou >= 0.5) {
                        int class_id = happenDetects[i].class_id;
                        std::string class_name;
                        std::string grade;
                        if (class_id < mControl->objects_v1_len) {
                            class_name = mControl->objects_v1[class_id*2];
                            grade = mControl->objects_v1[class_id*2+1];
                        }
                        else {
                            LOGE("class error,class_id=%d,objects_v1_len=%d", class_id, mControl->objects_v1_len);
                        }

                        happenDetects[i].class_name = class_name;
                        happenDetects[i].grade = grade;
                        float class_score = happenDetects[i].score;
                        if (mControl->objects_v1[class_id*2+1]>"0") {
                            mControl->category = class_name;
                            ++matchCount;
                        }
                    }
                }
                if (matchCount > 0) {//匹配数据大于0，则认为发生了报警事件
                    happen = true;
                    happenScore = 1.0;
                }

            }

            return true;
        }
        else if (mControl->modelCode == "tensorrt_yolo8") {
            //v3.43新增，基于tensorrt的yolo8检测

            if (mScheduler->algorithm_tr_yolo8_mtx.try_lock()) {
                if (mScheduler->algorithm_tr_yolo8) {
                    happenDetects.clear();
                    happen = false;
                    happenScore = 0;
                    mScheduler->algorithm_tr_yolo8->objectDetect(image, happenDetects);
                }
                mScheduler->algorithm_tr_yolo8_mtx.unlock();
            }
            if (!happenDetects.empty()) {

                //cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 0, 255), 4, 8);//绘制多边形
                int x1, y1, x2, y2;
                int matchCount = 0;
                for (int i = 0; i < happenDetects.size(); i++)
                {
                    x1 = happenDetects[i].x1;
                    y1 = happenDetects[i].y1;
                    x2 = happenDetects[i].x2;
                    y2 = happenDetects[i].y2;

                    std::vector<double> object_d;
                    object_d.push_back(x1);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y2);

                    object_d.push_back(x1);
                    object_d.push_back(y2);


                    double iou = CalcuPolygonIOU(mControl->recognitionRegion_d, object_d);

                    if (iou >= 0.5) {
                        int class_id = happenDetects[i].class_id;
                        std::string class_name;
                        std::string grade;
                        if (class_id < mControl->objects_v1_len) {
                            class_name = mControl->objects_v1[class_id*2];
                            grade = mControl->objects_v1[class_id * 2 + 1];
                        }
                        else {
                            LOGE("class error,class_id=%d,objects_v1_len=%d", class_id, mControl->objects_v1_len);
                        }

                        happenDetects[i].class_name = class_name;
                        happenDetects[i].grade = grade;
                        float class_score = happenDetects[i].score;
                        if (mControl->objects_v1[class_id*2+1]>"0") {
                            mControl->category = class_name;
                            ++matchCount;
                        }
                    }
                }
                if (matchCount > 0) {//匹配数据大于0，则认为发生了报警事件
                    happen = true;
                    happenScore = 1.0;
                }

            }

            return true;
        }
        else if (mControl->modelCode == "openvino_yolo8_face") {
            //v3.43新增，基于openvino的yolo8-face，仅用于检测人脸
            //if (mscheduler->algorithm_ov_yolo8_face_mtx.try_lock()) {
            //    if (mscheduler->algorithm_ov_yolo8_face) {
            //        happendetects.clear();
            //        happen = false;
            //        happenscore = 0;
            //        mscheduler->algorithm_ov_yolo8_face->objectdetect(image, happendetects);
            //    }
            //    mscheduler->algorithm_ov_yolo8_face_mtx.unlock();

            //    if (!happendetects.empty()) {
            //        上传人脸数据

            //        uint64_t curtimestamp = getcurtimestamp();
            //        uint64_t spendms = curtimestamp - ovfacelasttimestamp;
            //        if (spendms > 1000 * 5) {
            //            ovfacelasttimestamp = getcurtimestamp();

            //            首先将包含人脸的整张图片写入文件

            //            std::string prifixdir = mscheduler->getconfig()->uploaddir + "/staffrecord";

            //            std::filesystem::path __prifixdir_path1(prifixdir);
            //            try {
            //                if (!std::filesystem::exists(__prifixdir_path1)) {
            //                    std::filesystem::create_directory(__prifixdir_path1);
            //                }
            //            }
            //            catch (std::filesystem::filesystem_error& e) {
            //                std::cout << "handlevideoframe() create_directory1 error:" << e.what() << std::endl;
            //                return false;
            //            }

            //            prifixdir = prifixdir + "/" + mcontrol->code;

            //            std::filesystem::path __prifixdir_path2(prifixdir);
            //            try {
            //                if (!std::filesystem::exists(__prifixdir_path2)) {
            //                    std::filesystem::create_directory(__prifixdir_path2);
            //                }
            //            }
            //            catch (std::filesystem::filesystem_error& e) {
            //                std::cout << "handlevideoframe() create_directory2 error:" << e.what() << std::endl;
            //                return false;
            //            }

            //            std::string ymdhms_rd = getcurformattimestr("%y%m%d%h%m%s") + "_" + std::to_string(getrandomint());
            //            prifixdir = prifixdir + "/" + ymdhms_rd;
            //            std::filesystem::path __prifixdir_path3(prifixdir);
            //            try {
            //                if (!std::filesystem::exists(__prifixdir_path3)) {
            //                    std::filesystem::create_directory(__prifixdir_path3);
            //                }
            //            }
            //            catch (std::filesystem::filesystem_error& e) {
            //                std::cout << "handlevideoframe() create_directory3 error:" << e.what() << std::endl;
            //                return false;
            //            }

            //            std::string image_path = "staffrecord/" + mcontrol->code + "/" + ymdhms_rd + "/main.jpg";//图片相对路径
            //            std::string image_path_abs = mscheduler->getconfig()->uploaddir + "/" + image_path;

            //            cv::imwrite(image_path_abs, image);


            //            本次检测发生了事件，裁剪所有的人脸子图片
            //            std::string url = mscheduler->getconfig()->adminhost + "/staffrecord/postadd";
            //            json::value param;
            //            param["controlcode"] = mcontrol->code;

            //            param["starttimestamp"] = mcontrol->starttimestamp;
            //            param["checkfps"] = mcontrol->checkfps;
            //            param["videowidth"] = mcontrol->videowidth;
            //            param["videoheight"] = mcontrol->videoheight;
            //            param["videochannel"] = mcontrol->videochannel;
            //            param["videoindex"] = mcontrol->videoindex;
            //            param["videofps"] = mcontrol->videofps;

            //            param["framecount"] = framecount;


            //            json::value param_detects;
            //            json::value param_detects_item;
            //            for (int i = 0; i < happendetects.size(); i++)
            //            {
            //                param_detects_item["x1"] = happendetects[i].x1;
            //                param_detects_item["y1"] = happendetects[i].y1;
            //                param_detects_item["x2"] = happendetects[i].x2;
            //                param_detects_item["y2"] = happendetects[i].y2;
            //                param_detects_item["score"] = happendetects[i].score;
            //                param_detects_item["class_id"] = happendetects[i].class_id;
            //                param_detects_item["class_name"] = happendetects[i].class_name;

            //                param_detects.append(param_detects_item);
            //            }

            //            param["imagepath"] = image_path;
            //            param["detects"] = param_detects;

            //            std::string data = param.tostyledstring();
            //            request request;
            //            std::string response;
            //            request.post(url.data(), data.data(), response);

            //            logi("\n \t request:%s \n \t response:%s",
            //                url.data(),
            //                response.data());

            //        }



            //    }


            //    return true;
            //}
        }
        else if (mControl->modelCode == "API") {
            //v3.40新增，调用API类型的算法服务
            return this->postImage2Server(frameCount, image,happenDetects,happen,happenScore);

        }
        else if (mControl->modelCode == "DLIB_FACE") {
            //v3.40新增，基于dlib的人脸检测
            //v4.41移除，因为v3.41需要在rk3588等硬件编译，如果不去除还需要编译dlib，比较麻烦
            LOGE("该算法仅在v3.40版本支持：%s", mControl->algorithmCode.data());
            return false;
        }
        else if (mControl->modelCode == "CNNLSTM") {
            //v3.42新增，基于连续帧进行行为分析
            if (mCnnLstmCurFrameCount >= mCnnLstmThresh) {
                happenDetects.clear();
                happen = false;
                happenScore = 0;

                 //上传待分析信息start
                std::string url = "http://127.0.0.1:9710/algorithm";
                Json::Value param;
                param["control_code"] = mControl->code;
                std::string data = param.toStyledString();
                Request request;
                std::string response;

                bool ret = request.post(url.data(), data.data(), response);
                if (ret) {
                    ret = false;

                    Json::CharReaderBuilder builder;
                    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                    Json::Value root;
                    JSONCPP_STRING errs;

                    if (reader->parse(response.data(), response.data() + response.size(), &root, &errs) && errs.empty()) {
                        int code = root["code"].asInt();
                        std::string msg = root["msg"].asCString();
                        if (1000 == code) {
                            Json::Value info = root["info"];
                            int class_index = info["class_index"].asInt();
                            std::string class_name = info["class_name"].asCString();
                            float class_score = info["class_score"].asFloat();

                            //LOGI("class_name=%s,class_score=%.4f", class_name.data(), class_score);

                            if (class_score > 0.6) {

                                int image_width = image.rows;
                                int image_height = image.cols;

                                DetectObject detect;
                                detect.x1 = image_width/4;
                                detect.y1 = image_height/4;
                                detect.x2 = detect.x1 + 100;
                                detect.y2 = detect.y1 + 100;
                                detect.score = class_score;
                                detect.class_name = class_name;

                                happenDetects.push_back(detect);

                                happen = true;
                                happenScore = 1.0;

                            }

                            ret = true;
                        }
                    }
                }


                //上传待分析信息end



                mCnnLstmCurFrameCount = 0;
            }
            else {
                //继续积累视频数据
                std::string filepath = "D:\\Project\\BXC_VideoAnalyzer_v3\\data\\cnnlstmdata\\" +std::to_string(mCnnLstmCurFrameCount)+".jpg";
                std::ifstream f(filepath);
                
                if (f.good()) {
                    removeFile(filepath);
                }
                cv::imwrite(filepath, image);

                mCnnLstmCurFrameCount++;
            }

            return true;
        }
        else {
            LOGE("不支持的算法：%s",mControl->algorithmCode.data());
        }

        return false;

    }
    bool Analyzer::postImage2Server(int64_t frameCount, cv::Mat& image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore) {

        Config* config = mScheduler->getConfig();
        int height = mControl->videoHeight;
        int width = mControl->videoWidth;

        std::vector<int> JPEG_QUALITY = { 75 };
        std::vector<uchar> jpg;
        cv::imencode(".jpg", image, jpg, JPEG_QUALITY);
        int JPGBufSize = jpg.size();


        if (JPGBufSize > 0) {
            Base64 base64;
            std::string imageBase64;
            base64.encode(jpg.data(), JPGBufSize, imageBase64);

            std::string response;
            Json::Value param;
            param["image_base64"] = imageBase64;

            param["code"] = mControl->code;//布控编号
            //param["alarmObject"] = mControl->alarmObject;
            param["objects"] = mControl->objects;
            param["recognitionRegion"] = mControl->recognitionRegion;
            param["min_interval"] = mControl->minInterval;

            std::string data = param.toStyledString();
            //int64_t t1 = Common::getCurTimestamp();
            Request request;
            if (request.post(config->algorithmApiUrl.data(), data.data(), response)) {
                Json::CharReaderBuilder builder;
                const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

                Json::Value root;
                JSONCPP_STRING errs;

                if (reader->parse(response.data(), response.data() + std::strlen(response.data()),
                    &root, &errs) && errs.empty()) {

                    if (root["code"].isInt() && root["msg"].isString()) {
                        int code = root["code"].asInt();
                        std::string msg = root["msg"].asCString();

                        if (1000 == code) {
                            happenDetects.clear();
                            happen = false;
                            happenScore = 0.0;

                            Json::Value result = root["result"];
                            if (result["happen"].isBool() && result["happenScore"].isDouble()) {
                                happen = result["happen"].asBool();
                                happenScore = result["happenScore"].asFloat();

                                Json::Value result_detects = result["detects"];
                                for (auto i : result_detects) {

                                    int x1 = i["x1"].asInt();
                                    int y1 = i["y1"].asInt();
                                    int x2 = i["x2"].asInt();
                                    int y2 = i["y2"].asInt();
                                    float class_score = i["class_score"].asFloat();
                                    std::string class_name = i["class_name"].asString();

                                    DetectObject detect;
                                    detect.x1 = x1;
                                    detect.y1 = y1;
                                    detect.x2 = x2;
                                    detect.y2 = y2;
                                    detect.class_name = class_name;
                                    detect.score = class_score;

                                    happenDetects.push_back(detect);
                                }
                            }
                        }
                        else {
                            LOGE("code=%d,msg=%s", code, msg.data());
                        }

                    }
                    else {
                        LOGE("incorrect return parameter format");
                    }
                }
            }
            else {
                happenDetects.clear();
                happen = false;
                happenScore = 0.0;
            }
            //int64_t t2 = Common::getCurTimestamp();
        
        }

        return true;
    }

}