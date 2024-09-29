#include "AlgorithmTensorRT.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/TensorRTEngine.h"
#include <opencv2/opencv.hpp>
#include <opencv2/cudaimgproc.hpp>


namespace AVSAnalyzer {

    /*const std::vector<std::string> trt_class_names = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush" };*/
    /*const std::vector<std::string> trt_class_names = {
        "pedestrian","people","bicycle","car","van","truck","tricycle","awning-tricycle","bus","motor"
    };*/
    const std::vector<std::string> trt_class_names = {
        "pedestrian","people","bicycle","car","van","truck","tricycle","awning-tricycle","bus","motor"
        ,"person","hat","fire","smoke"
    };


    AlgorithmTensorRT::AlgorithmTensorRT(Config* config):Algorithm(config)
    {

        //const std::string engineFilePath = "model/yolov8n.fp16.1.1.engine";
        LOGI("加载的模型地址=%s", mConfig->yolov8TensorRTWeight.data());

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

        bool succ = mEngine->setTRTEngineParams(mConfig->yolov8TensorRTWeight, subVals, divVals, normalize);
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
    bool AlgorithmTensorRT::objectDetect(int height, int width, cv::Mat& image, std::vector<DetectObject>& detects, std::vector<int>& count_category) {
    

        // Read the input image
        // TODO: You will need to read the input image required for your model
        //const std::string inputImage = "bus.jpg";
        //auto cpuImg = cv::imread(inputImage);
        //if (cpuImg.empty()) {
        //    throw std::runtime_error("Unable to read image at path: " + inputImage);
        //}
        //cpuImg = image;
        float scale = std::max(height,width) / 1280.0;


        // Upload the image GPU memory
        cv::cuda::GpuMat img;
        img.upload(image);

        // The model expects RGB input
        cv::cuda::cvtColor(img, img, cv::COLOR_BGR2RGB);

        // In the following section we populate the input vectors to later pass for inference
        const auto& inputDims = mEngine->getInputDims();
        std::vector<std::vector<cv::cuda::GpuMat>> inputs;

        // Let's use a batch size which matches that which we set the Options.optBatchSize option
        size_t batchSize = 1;// options.optBatchSize;

        // TODO:
        // For the sake of the demo, we will be feeding the same image to all the inputs
        // You should populate your inputs appropriately.
        for (const auto& inputDim : inputDims) { // For each of the model inputs...
            std::vector<cv::cuda::GpuMat> input;
            for (size_t j = 0; j < batchSize; ++j) { // For each element we want to add to the batch...
                // TODO:
                // You can choose to resize by scaling, adding padding, or a combination of the two in order to maintain the aspect ratio
                // You can use the Engine::resizeKeepAspectRatioPadRightBottom to resize to a square while maintain the aspect ratio (adds padding where necessary to achieve this).
                //printf("%d----%d",inputDim.d[1], inputDim.d[2]);
                auto resized = TensorRTEngine::resizeKeepAspectRatioPadRightBottom(img, inputDim.d[1], inputDim.d[2]);
                // You could also perform a resize operation without maintaining aspect ratio with the use of padding by using the following instead:
    //            cv::cuda::resize(img, resized, cv::Size(inputDim.d[2], inputDim.d[1])); // TRT dims are (height, width) whereas OpenCV is (width, height)
                input.emplace_back(std::move(resized));
            }
            inputs.emplace_back(std::move(input));
        }


        std::vector<std::vector<std::vector<float>>> featureVectors;
        //LOGE("%d",int(mEngine->runInference(inputs, featureVectors)));
        if (mEngine->runInference(inputs, featureVectors)) {
        

            int size = 0;
            if (featureVectors.size() > 0) {
                if (featureVectors[0].size() > 0) {
                    size = featureVectors[0][0].size();
                }

            }
          //LOGE("size================%d", size);
            if (2448000 == size) {//705600
                
                ///开始处理结果数据
                int size = featureVectors[0][0].size();
                float* data = featureVectors[0][0].data();
                int dimensions = trt_class_names.size()+4;
                int rows = 136000;

                // -------- Step 8. Postprocess the result --------
                cv::Mat output_buffer(dimensions, rows, CV_32F, data);
                cv::transpose(output_buffer, output_buffer); //[8400,84]
                float score_threshold = 0.25;//分类过滤阈值
                float nms_threshold = 0.5;
                std::vector<int> class_ids;
                std::vector<float> class_scores;
                std::vector<cv::Rect> boxes;

                int class_count = trt_class_names.size();
                int score_index_start = 4;
                int score_index_end = score_index_start + class_count;

                // Figure out the bbox, class_id and class_score
                for (int i = 0; i < output_buffer.rows; i++) {
                    //cv::Mat classes_scores = output_buffer.row(i).colRange(4, 84);
                    cv::Mat classes_scores = output_buffer.row(i).colRange(score_index_start, score_index_end);
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

                        int left = int((cx - 0.5 * w) * scale);
                        int top = int((cy - 0.5 * h) * scale);
                        int width = int(w * scale);
                        int height = int(h * scale);

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
                    std::string class_name = trt_class_names[class_id];
                    count_category[class_id]++;
                    //printf("index=%d,class_score=%.4f,class_name=%s\n", index, class_score, class_name.data());

                    DetectObject detect;
                    detect.score = class_score;
                    detect.class_name = class_name;
                    detect.x1 = boxes[index].tl().x;
                    detect.y1 = boxes[index].tl().y;
                    detect.x2 = detect.x1 + boxes[index].width;
                    detect.y2 = detect.y1 + boxes[index].height;

                    detects.push_back(detect);
                }

                return true;
            }
            
        }

        return false;
    }
}