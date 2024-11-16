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
        // 处理所有异常的静态方法
        static void Handle(const std::exception& e);

    private:
        // 处理文件系统错误
        static void HandleFilesystemError(const std::filesystem::filesystem_error& e);

        // 处理 ONNX Runtime 特定的异常
        static void HandleOrtException(const Ort::Exception& e);

        // 处理通用异常
        static void HandleGeneralException(const std::exception& e);
    };
}
#endif // EXCEPTIONHANDLER_H
