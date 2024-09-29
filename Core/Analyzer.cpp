#include "Analyzer.h"
#include "Algorithm.h"
#include <opencv2/opencv.hpp>
#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/CalcuIOU.h"

namespace AVSAnalyzer {
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }
    /*const std::vector<std::string> trt_class_names = {
        "pedestrian","people","bicycle","car","van","truck","tricycle","awning-tricycle","bus","motor"
    };*/
    const std::vector<std::string> trt_class_names = {
        "pedestrian","people","bicycle","car","van","truck","tricycle","awning-tricycle","bus","motor"
        ,"person","hat","fire","smoke"
    };
    Analyzer::Analyzer(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(control)
    {

    }

    Analyzer::~Analyzer()
    {
        mDetects.clear();

    }
    std::string cv_count = "";
    bool Analyzer::handleVideoFrame(int64_t frameCount, unsigned char* bgr, bool& happen, float& happenScore, int& count_gap, int& gap, std::string& warnning ) {
        bool isNormal = false;//该参数的意义是，判断本次执行，是否真正调用了算法。用于统计checkfps

        happen = false;
        happenScore = 0.0;
   

        cv::Mat image(mControl->videoHeight, mControl->videoWidth, CV_8UC3, bgr);

        Algorithm* algorihtm = mScheduler->gainAlgorithm();
        count_gap++;
        int flag = 0;
        std::vector<int> count_category(trt_class_names.size(), 0);
        
        std::vector<std::string> category = split(mControl->objectCode, ',');
       
        if (algorihtm) {

            
            if (count_gap == gap) {
                
                cv_count = "";
                count_gap = 1;
                flag = 1;
                //printf("%s",algorihtm);
                algorihtm->objectDetect(mControl->videoHeight, mControl->videoWidth, image, mDetects, count_category);
            }
            
            if (count_gap % 5 == 0) {
                mDetects.clear();               
            }
           
            //try {
            //    mScheduler->giveBackAlgorithm(algorihtm);
            //}
            //catch (const std::exception& e) {
            //    // 处理异常情况，例如输出错误消息或执行其他操作
            //    std::cerr << "Exception occurred: " << e.what() << std::endl;
            //    // 可以选择采取适当的措施来处理异常，例如重新尝试、回滚操作或抛出新的异常
            //}
            mScheduler->giveBackAlgorithm(algorihtm);
            isNormal = true;
        }
        if (mControl->checkFps > 0) {
            
            int x1, y1, x2, y2;
            int matchCount = 0;
            for (int i = 0; i < count_category.size(); i++) {
                if ( count_category[i] != 0 ){
                    cv_count = cv_count + trt_class_names[i] + ":" + std::to_string(count_category[i])+" ";
                }
            }
            for (int i = 0; i < mDetects.size(); i++)
            {
                x1 = mDetects[i].x1;
                y1 = mDetects[i].y1;
                x2 = mDetects[i].x2;
                y2 = mDetects[i].y2;

                std::vector<double> object_d;
                object_d.push_back(x1);
                object_d.push_back(y1);

                object_d.push_back(x2);
                object_d.push_back(y1);

                object_d.push_back(x2);
                object_d.push_back(y2);

                object_d.push_back(x1);
                object_d.push_back(y2);


                //double iou = CalcuPolygonIOU(mControl->recognitionRegion_d, object_d);

                //if (iou >= mControl->overlapThresh) {
                std::string class_name = mDetects[i].class_name;
                float       class_score = mDetects[i].score;
                //cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0, 255, 0), 2, cv::LINE_8, 0);
                std::string text = class_name + ":" + std::to_string(class_score);
                //cv::putText(image, text, cv::Point(x1 + 5, y1 + 15), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
                //LOGI("--------------%s", mControl->objectCode);
                auto it = std::find(category.begin(), category.end(), class_name);
                if (it != category.end()) {
                    if (class_score >= mControl->classThresh) {
                        ++matchCount;
                        warnning = class_name;
                    }                 
                    //printf("%d", matchCount);
                    cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0,0, 255), 2, cv::LINE_8, 0);
                    cv::putText(image, text, cv::Point(x1 + 5, y1 + 15), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
                    //cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0, 255, 0), 2, cv::LINE_8, 0);
                }
                else
                {
                    cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0, 255, 0), 2, cv::LINE_8, 0);
                    cv::putText(image, text, cv::Point(x1 + 5, y1 + 15), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
                }
            }

            if (matchCount > 0) {//匹配数据大于0，则认为发生了报警事件
                happen = true;
                happenScore = 1.0;
            }
            //count_gap = 0;
        }

        //cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 0, 255), 4, 8);//绘制多边形

        std::stringstream fps_stream;
        fps_stream << std::setprecision(4) << mControl->checkFps;
        std::string fps_title = "fps:" + fps_stream.str();
        cv::putText(image, fps_title, cv::Point(20, 80), cv::FONT_HERSHEY_COMPLEX, mControl->videoWidth / 1000, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        cv::putText(image, cv_count, cv::Point(200, 80), cv::FONT_HERSHEY_COMPLEX, mControl->videoWidth / 1000, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);

        return isNormal;

    }

}