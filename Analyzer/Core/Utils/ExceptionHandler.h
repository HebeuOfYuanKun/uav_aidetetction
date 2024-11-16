// ExceptionHandler.h

#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <iostream>
#include <string>
#include <exception>
#include <filesystem>
#include <onnxruntime_cxx_api.h>

namespace AVSAnalyzer {
    class ExceptionHandler {
    public:
        // ���������쳣�ľ�̬����
        static void Handle(const std::exception& e);

    private:
        // �����ļ�ϵͳ����
        static void HandleFilesystemError(const std::filesystem::filesystem_error& e);

        // ���� ONNX Runtime �ض����쳣
        static void HandleOrtException(const Ort::Exception& e);

        // ����ͨ���쳣
        static void HandleGeneralException(const std::exception& e);
    };
}
#endif // EXCEPTIONHANDLER_H
