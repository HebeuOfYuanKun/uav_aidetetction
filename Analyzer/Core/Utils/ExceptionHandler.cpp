// ExceptionHandler.cpp

#include "ExceptionHandler.h"
namespace AVSAnalyzer {
    // ���������쳣�ľ�̬����
    void ExceptionHandler::Handle(const std::exception& e) {
        try {
            if (const auto* fs_error = dynamic_cast<const std::filesystem::filesystem_error*>(&e)) {
                HandleFilesystemError(*fs_error);
            }
            else if (const auto* ort_error = dynamic_cast<const Ort::Exception*>(&e)) {
                HandleOrtException(*ort_error);
            }
            else {
                HandleGeneralException(e);
            }
        }
        catch (const std::exception& inner_e) {
            std::cerr << "Exception while handling exception: " << inner_e.what() << std::endl;
        }
    }

    // �����ļ�ϵͳ����
    void ExceptionHandler::HandleFilesystemError(const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;

        if (e.code() == std::errc::no_such_file_or_directory) {
            std::cerr << "Error: File not found. Path: " << e.path1() << std::endl;
        }
        else if (e.code() == std::errc::permission_denied) {
            std::cerr << "Error: Permission denied. Path: " << e.path1() << std::endl;
        }
        else {
            std::cerr << "Error: Other filesystem issue. Path: " << e.path1() << std::endl;
        }
    }

    // ���� ONNX Runtime �ض����쳣
    void ExceptionHandler::HandleOrtException(const Ort::Exception& e) {
        OrtErrorCode ort_code = e.GetOrtErrorCode(); // ��ȡ ONNX Runtime �������
        std::cerr << "ONNX Runtime error: " << e.what() << " (OrtErrorCode: " << ort_code << ")" << std::endl;

        // ���� OrtErrorCode ����ͬ����
        switch (ort_code) {
        case ORT_INVALID_ARGUMENT:
            std::cerr << "Error: Invalid argument passed to ONNX Runtime function." << std::endl;
            break;
        case ORT_NO_SUCHFILE:
            std::cerr << "Error: Model file not found or path is incorrect." << std::endl;
            break;
        case ORT_ENGINE_ERROR:
            std::cerr << "Error: Runtime engine encountered an error." << std::endl;  
            break;
        default:
            std::cerr << "Error: Other ONNX Runtime error occurred." << std::endl;
            break;
        }
    }

    // ����ͨ���쳣
    void ExceptionHandler::HandleGeneralException(const std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
    }
}