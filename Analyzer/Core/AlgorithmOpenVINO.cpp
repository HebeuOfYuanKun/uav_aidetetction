#include "AlgorithmOpenVINO.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"


namespace AVSAnalyzer {

    // AlgorithmOpenVINO 类的构造函数，接受配置、模型路径和设备类型
    AlgorithmOpenVINO::AlgorithmOpenVINO(Config* config) : Algorithm(config) {
        

        // 记录模型路径和设备类型以供调试
        LOGI("AlgorithmOpenVINO() modelPath=%s,device=%s", config->openVINO.data(), config->algorithmOpenVINODetectDevice.data());

        // 创建 OpenVINO 的核心对象
        this->core = new ov::Core;

        // 编译模型，指定模型路径和设备类型（如 CPU、GPU、VPU 等）
        this->compiled_model = this->core->compile_model(config->openVINO.data(), config->algorithmOpenVINODetectDevice.data());

        // 创建推理请求，准备进行推理操作
        this->infer_request = compiled_model.create_infer_request();
    }


	AlgorithmOpenVINO::~AlgorithmOpenVINO()
    {
        LOGI("");
    }

    bool AlgorithmOpenVINO::objectDetect(cv::Mat& image, std::vector<DetectObject>& detects){

        // -------- Step 4.Read a picture file and do the preprocess --------
        // Preprocess the image
        // Get input dimensions
        auto input_port = compiled_model.input();
        auto input_shape = input_port.get_shape();
        int input_h = input_shape[2]; // Height
        int input_w = input_shape[3]; // Width

        cv::Mat letterbox_img = letterbox(image);

        int letterbox_img_h = letterbox_img.size[0];
        
        cv::Mat blob = cv::dnn::blobFromImage(letterbox_img, 1.0 / 255.0, cv::Size(input_w, input_h), cv::Scalar(), true, false);

    
        // -------- Step 5. Feed the blob into the input node of the Model -------
        // Create tensor from external memory
        ov::Tensor input_tensor(input_port.get_element_type(), input_shape, blob.ptr(0));

        //std::cout << "start set_input_tensor" << std::endl;



        // Set input tensor for model with one input
        infer_request.set_input_tensor(input_tensor);
        //std::cout << "set_input_tensor success" << std::endl;

        // -------- Step 6. Start inference --------
        infer_request.infer();
        //std::cout << "infer success" << std::endl;

        // -------- Step 7. Get the inference result --------
        auto output = infer_request.get_output_tensor(0);
        auto output_shape = output.get_shape();
        //std::cout << "The shape of output tensor:" << output_shape << std::endl;
        int rows = output_shape[2];        //8400
        int dimensions = output_shape[1];  //84: box[cx, cy, w, h]+80 classes scores

        // -------- Step 8. Postprocess the result --------
        float* data = output.data<float>();
        cv::Mat output_buffer(output_shape[1], output_shape[2], CV_32F, data);
        cv::transpose(output_buffer, output_buffer); //[8400,84]
        float score_threshold = 0.1;
        float nms_threshold = 0.1;
        std::vector<int> class_ids;
        std::vector<float> class_scores;
        std::vector<cv::Rect> boxes;


        // Figure out the bbox, class_id and class_score
        for (int i = 0; i < output_buffer.rows; i++) {
            //cv::Mat classes_scores = output_buffer.row(i).colRange(4, 84);// 1-4对应x,y,w,h。 5-84对应80个分类
            cv::Mat classes_scores = output_buffer.row(i).colRange(4, dimensions);

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

                //std::cout << "i=" << i << ",cx=" << cx << ",cy=" << cy << ",w=" << w << ",h=" << h << std::endl;


                int x = int((cx - 0.5 * w) * (image.cols / static_cast<float>(input_w)));
                int y = int((cy - 0.5 * h) * (image.rows / static_cast<float>(input_h)));
                int width = int(w * (image.cols / static_cast<float>(input_w)));
                int height = int(h * (image.rows / static_cast<float>(input_h)));

                boxes.push_back(cv::Rect(x,y, width, height));
            }
        }
        //NMS
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, class_scores, score_threshold, nms_threshold, indices);


        if (indices.size() > 0) {
            detects.clear();
            // -------- Visualize the detection results -----------
            for (size_t i = 0; i < indices.size(); i++) {
                int index = indices[i];
                int class_id = class_ids[index];
                cv::Rect rect = boxes[index];
                int x = rect.x;
                int y = rect.y;
                int width = rect.width;
                int height = rect.height;

                DetectObject detect;
                detect.x1 = x;
                detect.y1 = y;
                detect.x2 = x + width;
                detect.y2 = y + height;
                detect.class_id = class_id;
                //detect.class_name = "face";
                detect.score = class_scores[index];

                detects.emplace_back(std::move(detect));

 
            }

            return true;
        }

        return false;
    }

}