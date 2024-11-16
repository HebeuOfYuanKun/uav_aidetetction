#include "AlgorithmTensorRT.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/TensorRTEngine.h"
#include <opencv2/opencv.hpp>
#include <opencv2/cudaimgproc.hpp>


namespace AVSAnalyzer {

 


    AlgorithmTensorRT::AlgorithmTensorRT(Config* config) :Algorithm(config)
    {
        std::string modelPath = config->tensorRt;
        //const std::string engineFilePath = "model/yolov8n.fp16.1.1.engine";
        LOGI("加载的模型地址=%s", modelPath.data());

        // Specify our GPU inference configuration options
        Options options;
        // Specify what precision to use for inference
        // FP16 is approximately twice as fast as FP32.
        options.precision = Precision::FP16;
        // If using INT8 precision, must specify path to directory containing calibration data.
        options.calibrationDataDirectoryPath = "";
        // If the model does not support dynamic batch size, then the below two parameters must be set to 1.
        // Specify the batch size to optimize for.
        options.optBatchSize = 1;
        // Specify the maximum batch size we plan on running.
        options.maxBatchSize = 1;

        mEngine = new TensorRTEngine(options);

        // Define our preprocessing code
        // The default Engine::build method will normalize values between [0.f, 1.f]
        // Setting the normalize flag to false will leave values between [0.f, 255.f] (some converted models may require this).

        // For our YoloV8 model, we need the values to be normalized between [0.f, 1.f] so we use the following params
        std::array<float, 3> subVals{ 0.f, 0.f, 0.f };
        std::array<float, 3> divVals{ 1.f, 1.f, 1.f };
        bool normalize = true;
        // Note, we could have also used the default values.

        // If the model requires values to be normalized between [-1.f, 1.f], use the following params:
        //    subVals = {0.5f, 0.5f, 0.5f};
        //    divVals = {0.5f, 0.5f, 0.5f};
        //    normalize = true;

        bool succ = mEngine->setTRTEngineParams(modelPath, subVals, divVals, normalize);
        if (!succ) {
            throw std::runtime_error("Unable to setTRTEngineParams.");
        }

        // Load the TensorRT engine file from disk
        succ = mEngine->loadNetwork();
        if (!succ) {
            throw std::runtime_error("Unable to load TRT engine.");
        }


    }

    AlgorithmTensorRT::~AlgorithmTensorRT()
    {
        LOGI("");
        if (mEngine) {
            delete mEngine;
            mEngine = nullptr;
        }


    }
    bool AlgorithmTensorRT::objectDetect( cv::Mat& image, std::vector<DetectObject>& detects) {


        /// 读取输入图像（可根据需求修改）
        cv::Mat letterbox_img = letterbox(image); // 进行字母框处理，确保图像符合模型输入要求

        // 获取处理后的图像高度
        int letterbox_img_h = letterbox_img.size[0];

        // 获取模型的输入维度
        const auto& inputDims = mEngine->getInputDims();
        int input_height = inputDims[0].d[1]; // 获取输入高度
        int input_width = inputDims[0].d[2];  // 获取输入宽度
        // 计算缩放比例
        float scale_h = static_cast<float>(letterbox_img.rows) / input_height;
        float scale_w = static_cast<float>(letterbox_img.cols) / input_width;
       
        // 上传图像到GPU内存
        cv::cuda::GpuMat img;
        img.upload(letterbox_img);

        // 将图像转换为RGB格式
        // 
        cv::cuda::cvtColor(img, img, cv::COLOR_BGR2RGB);
       
        // 填充输入向量以便后续推理
        std::vector<std::vector<cv::cuda::GpuMat>> inputs;

        // 设定批量大小
        size_t batchSize = 1;

        for (const auto& inputDim : inputDims) {
            std::vector<cv::cuda::GpuMat> input;
            for (size_t j = 0; j < batchSize; ++j) {
                // 自适应调整图像大小以匹配模型输入
                auto resized = TensorRTEngine::resizeKeepAspectRatioPadRightBottom(img, inputDim.d[1], inputDim.d[2]);
                input.emplace_back(std::move(resized));
            }
            inputs.emplace_back(std::move(input));
        }
        // 获取输出形状
        const auto& outputDims = mEngine->getOutputDims();

        std::vector<std::vector<std::vector<float>>> featureVectors;
        if (mEngine->runInference(inputs, featureVectors)) {

                int batchSize = outputDims[0].d[0];
                int rows = outputDims[0].d[1]; // 8400
                int dimensions = outputDims[0].d[2]; // 84
                ///开始处理结果数据
                int size = featureVectors[0][0].size();
                float* data = featureVectors[0][0].data();


                // -------- Step 8. Postprocess the result --------
                cv::Mat output_buffer(dimensions, rows, CV_32F, data);
                cv::transpose(output_buffer, output_buffer); //[8400,84]
                float score_threshold = 0.25;//分类过滤阈值
                float nms_threshold = 0.5;
                std::vector<int> class_ids;
                std::vector<float> class_scores;
                std::vector<cv::Rect> boxes;

            
                int score_index_start = 4;
                /*int score_index_end = score_index_start + class_count;*/

                // Figure out the bbox, class_id and class_score
                for (int i = 0; i < output_buffer.rows; i++) {
                    //cv::Mat classes_scores = output_buffer.row(i).colRange(4, 84);
                    cv::Mat classes_scores = output_buffer.row(i).colRange(score_index_start, dimensions);
                    cv::Point class_id;
                    double maxClassScore;
                    cv::minMaxLoc(classes_scores, 0, &maxClassScore, 0, &class_id);

                    if (maxClassScore > score_threshold) {
                        class_scores.push_back(maxClassScore);
                        class_ids.push_back(class_id.x);
                        float cx = output_buffer.at<float>(i, 0);
                        float cy = output_buffer.at<float>(i, 1);
                        float w = output_buffer.at<float>(i, 2);
                        float h = output_buffer.at<float>(i, 3);

                        int left = int((cx - 0.5 * w) * scale_h);
                        int top = int((cy - 0.5 * h) * scale_w);
                        int width = int(w * scale_h);
                        int height = int(h * scale_w);

                        boxes.push_back(cv::Rect(left, top, width, height));
                    }
                }
                //NMS
                std::vector<int> indices;
                cv::dnn::NMSBoxes(boxes, class_scores, score_threshold, nms_threshold, indices);

                // -------- Visualize the detection results -----------
                for (size_t i = 0; i < indices.size(); i++) {
                    int index = indices[i];
                    int class_id = class_ids[index];

                    float class_score = class_scores[index];
                   
                    //printf("index=%d,class_score=%.4f,class_name=%s\n", index, class_score, class_name.data());

                    DetectObject detect;
                    detect.score = class_score;
                    detect.class_id = class_id;
                    detect.x1 = boxes[index].tl().x;
                    detect.y1 = boxes[index].tl().y;
                    detect.x2 = detect.x1 + boxes[index].width;
                    detect.y2 = detect.y1 + boxes[index].height;

                    detects.push_back(detect);
                }

                return true;
            }

       

        return false;
    }
}