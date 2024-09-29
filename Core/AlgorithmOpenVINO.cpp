#include "AlgorithmOpenVINO.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"

namespace AVSAnalyzer {

    //using namespace cv;
    //using namespace dnn;

    std::vector<cv::Scalar> colors = {
    cv::Scalar(0, 0, 255) ,
    cv::Scalar(0, 255, 0) ,
    cv::Scalar(255, 0, 0) ,
    cv::Scalar(255, 100, 50) ,
    cv::Scalar(50, 100, 255) ,
    cv::Scalar(255, 50, 100)
    };

    /*const std::vector<std::string> class_names = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush" };*/
    const std::vector<std::string> class_names = {
        "pedestrian","people","bicycle","car","van","truck","tricycle","awning-tricycle","bus","motor"
    };


    cv::Mat letterbox(const cv::Mat& source)
    {
        int col = source.cols;
        int row = source.rows;
        int _max = MAX(col, row);
        cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
        source.copyTo(result(cv::Rect(0, 0, col, row)));
        return result;
    }


    AlgorithmOpenVINO::AlgorithmOpenVINO(Config* config):Algorithm(config){
        LOGI("加载的模型地址=%s", mConfig->yolov8OpenVINOWeight.data());

        // -------- Step 1. Initialize OpenVINO Runtime Core --------
        core = new ov::Core;

        // -------- Step 2. Compile the Model --------
        //std::shared_ptr<ov::Model> model = core->read_model("yolov8n.xml");

        //compiled_model = core->compile_model("yolov8n.xml", "GPU");
        compiled_model = core->compile_model(mConfig->yolov8OpenVINOWeight, mConfig->algorithmDevice);

        // -------- Step 3. Create an Inference Request --------
        infer_request = compiled_model.create_infer_request();
    }

    AlgorithmOpenVINO::~AlgorithmOpenVINO()
    {
        LOGI("");
        delete core;
        core = nullptr;

    }

    bool AlgorithmOpenVINO::objectDetect(int height, int width, cv::Mat& image, std::vector<DetectObject>& detects, std::vector<int>& count_category){

        //int64_t t1 = getCurTime();
        //cv::Mat image(height, width, CV_8UC3, bgr);

        // Preprocess the image
        cv::Mat letterbox_img = letterbox(image);
        float scale = letterbox_img.size[0] / 640.0;
        cv::Mat blob = cv::dnn::blobFromImage(letterbox_img, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(), true);

        // -------- Step 5. Feed the blob into the input node of the Model -------
        // Get input port for model with one input
        auto input_port = compiled_model.input();


        // Create tensor from external memory
        ov::Tensor input_tensor(input_port.get_element_type(), input_port.get_shape(), blob.ptr(0));
        // Set input tensor for model with one input
        infer_request.set_input_tensor(input_tensor);

        // -------- Step 6. Start inference --------
        infer_request.infer();

        // -------- Step 7. Get the inference result --------
        auto output = infer_request.get_output_tensor(0);
        auto output_shape = output.get_shape();


        int rows = output_shape[2];        //8400
        int dimensions = output_shape[1];  //84: box[cx, cy, w, h]+80 classes scores

        // -------- Step 8. Postprocess the result --------
        float* data = output.data<float>();
        cv::Mat output_buffer(output_shape[1], output_shape[2], CV_32F, data);
        cv::transpose(output_buffer, output_buffer); //[8400,84]
        float score_threshold = 0.25;
        float nms_threshold = 0.5;
        std::vector<int> class_ids;
        std::vector<float> class_scores;
        std::vector<cv::Rect> boxes;

        // Figure out the bbox, class_id and class_score
        for (int i = 0; i < output_buffer.rows; i++) {
            cv::Mat classes_scores = output_buffer.row(i).colRange(4, output_shape[1]);
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
        //if (indices.size() > 0) {
        //    detects.clear();
        //}

        for (size_t i = 0; i < indices.size(); i++) {
            int index = indices[i];
            int class_id = class_ids[index];

            /*
            cv::rectangle(img, boxes[index], colors[class_id % 6], 2, 8);
            std::string label = class_names[class_id] + ":" + std::to_string(class_scores[index]).substr(0, 4);

            cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, 0);
            cv::Rect textBox(boxes[index].tl().x, boxes[index].tl().y - 15, textSize.width, textSize.height + 5);

            cv::rectangle(img, textBox, colors[class_id % 6], cv::FILLED);
            cv::putText(img, label, cv::Point(boxes[index].tl().x, boxes[index].tl().y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
            */

            DetectObject detect;
            detect.score = class_scores[index];
            detect.class_name = class_names[class_id];
            detect.x1 = boxes[index].tl().x;
            detect.y1 = boxes[index].tl().y;

            detect.x2 = detect.x1 + boxes[index].width;
            detect.y2 = detect.y1 + boxes[index].height;

            detects.push_back(detect);

        }

        //int64_t t2 = getCurTime();

        //LOGI("objectDetect spend %lld(ms)", (t2 - t1));
        return true;
    }

}