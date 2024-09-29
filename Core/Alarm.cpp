//#include "Alarm.h"
//#include "Config.h"
//#include "Utils/Log.h"
//#include "Utils/Common.h"
//#include <json/json.h>
//#include "Utils/Request.h"
//#include <iostream>
//#include <filesystem>
//#include <opencv2/opencv.hpp>
//#include <opencv2/imgcodecs/legacy/constants_c.h>
//extern "C" {
//#include "libswscale/swscale.h"
//#include <libavutil/imgutils.h>
//#include <libswresample/swresample.h>
//}
//
//#pragma warning(disable: 4996)
//
//#ifdef WIN32
//    #ifndef _DEBUG
//        #include <turbojpeg.h>// windows+release环境下，图像的jpg压缩和解压缩均使用turboJpeg库实现，其余环境则使用opencv库
//    #endif
//#endif
//
//
//namespace AVSAnalyzer {
//    AlarmImage::AlarmImage(int width, int height, int channels) {
//        mWidth = width;
//        mHeight = height;
//        mChannels = channels;
//
//        //mJpgBuf = (unsigned char*)malloc(alarm_image_jpg_buf_max_size);
//    }
//    AlarmImage::~AlarmImage() {
//        //if (mJpgBuf) {
//        //    free(mJpgBuf);
//        //    mJpgBuf = nullptr;
//        //}
//    }
//
//    void AlarmImage::setBuf(unsigned char* buf, int size) {
//        if (size >0 && size <= alarm_image_jpg_buf_max_size) {
//            memcpy(mJpgBuf, buf, size);
//            mJpgBufSize = size;
//        }
//        else {
//            LOGE("AlarmImage::setBuf size=%d over max", size);
//            mJpgBufSize = -1;
//        }
//
//    }
//
//    unsigned char* AlarmImage::getBuf() {
//        return mJpgBuf;
//    }
//    int AlarmImage::getSize() {
//        return mJpgBufSize;
//    }
//    int AlarmImage::getWidth() {
//        return mWidth;
//    }
//    int AlarmImage::getHeight() {
//        return mHeight;
//    }
//    int AlarmImage::getChannels() {
//        return mChannels;
//    }
//
//    Alarm::Alarm(int height, int width, int fps, int64_t happenTimestamp, int happenImageIndex, const char* controlCode) {
//        LOGI("");
//
//        this->height = height;
//        this->width = width;
//        this->fps = fps;
//        this->happenTimestamp = happenTimestamp;
//        this->happenImageIndex = happenImageIndex;
//        this->controlCode = controlCode;
//    }
//    Alarm::~Alarm() {
//        LOGI("");
//
//        for (size_t i = 0; i < this->images.size(); i++)
//        {
//            AlarmImage* image = this->images[i];
//            delete image;
//            image = nullptr;
//        }
//        images.clear();
//
//    }
//    //turboJpeg库讲bgr图像压缩jpg
//    bool bgr2Jpg_by_turboJpeg(int height, int width, int channels, unsigned char* in_bgr, unsigned char*& out_jpg, unsigned long* out_jpg_size) {
//
//#ifdef WIN32
//#ifndef _DEBUG
//
//        tjhandle handle = tjInitCompress();
//        if (nullptr == handle) {
//            return false;
//        }
//
//        //pixel_format : TJPF::TJPF_BGR or other
//        const int JPEG_QUALITY = 75;
//        int pixel_format = TJPF::TJPF_BGR;
//        int pitch = tjPixelSize[pixel_format] * width;
//        int ret = tjCompress2(handle, in_bgr, width, pitch, height, pixel_format,
//            &out_jpg, out_jpg_size, TJSAMP_444, JPEG_QUALITY, TJFLAG_FASTDCT);
//
//        tjDestroy(handle);
//
//        if (ret != 0) {
//            return false;
//        }
//        return true;
//
//#endif // !_DEBUG
//#endif //WIN32
//
//        return false;
//    }
//
//    bool bgr2Jpg(int height, int width, int channels, unsigned char* in_bgr, AlarmImage* out_image) {
//
//        unsigned char* out_jpg_buf = nullptr;
//        unsigned long  out_jpg_size = 0;
//
//        bool res = bgr2Jpg_by_turboJpeg(height, width, channels, in_bgr, out_jpg_buf, &out_jpg_size);
//
//        if (res) {//使用turboJpeg库进行图像压缩成功
//            if (out_jpg_size > 0 && out_jpg_buf != nullptr) {
//
//                out_image->setBuf(out_jpg_buf, out_jpg_size);
//                
//                free(out_jpg_buf);
//                out_jpg_buf = nullptr;
//                return true;
//            }
//            else {
//                return false;
//            }
//        
//        }
//        else {//使用opencv库进行图像压缩
//            //TODO 新升级OpenCV-4.7.0 不支持下面方式的bgr压缩jpg，会引发错误，所以不要使用Debug版本运行
//
//            LOGE("Alarm.cpp 新升级OpenCV-4.7.0 不支持下面方式的bgr压缩jpg，会引发错误，所以不要使用Debug版本运行");
//           
//            cv::Mat in_bgr_cvmat(height, width, CV_8UC3, in_bgr);
//            std::vector<int> JPEG_QUALITY = { 75 };
//            std::vector<uchar> out_jpg;
//            cv::imencode(".jpg", in_bgr_cvmat, out_jpg, JPEG_QUALITY);
//            out_image->setBuf(out_jpg.data(), out_jpg.size());
//            out_jpg.clear();
//
//            return true;
//        
//        }
//    }
//    bool jpg2Bgr_by_opencv(AlarmImage* in_image, unsigned char*& out_bgr) {
//
//        unsigned char* jpg_buf = in_image->getBuf();
//        int jpg_size = in_image->getSize();
//        int height = in_image->getHeight();
//        int width = in_image->getWidth();
//        int channels = in_image->getChannels();
//
//        int bgrSize = width * height * channels;
//
//        std::vector<uchar> jpgData(jpg_size);
//        memcpy(jpgData.data(), jpg_buf, jpg_size);
//        cv::Mat out_bgr_image = cv::imdecode(jpgData, CV_LOAD_IMAGE_UNCHANGED);
//        memcpy(out_bgr, out_bgr_image.data, bgrSize);
//        return true;
//    }
//    bool jpg2Bgr_by_turboJpeg(AlarmImage* in_image, unsigned char*& out_bgr) {
//
//#ifdef WIN32
//#ifndef _DEBUG
//        unsigned char* jpg_buf = in_image->getBuf();
//        unsigned long jpg_size = in_image->getSize();
//        int height = in_image->getHeight();
//        int width = in_image->getWidth();
//        int channels = in_image->getChannels();
//
//        tjhandle handle = tjInitDecompress();
//        if (nullptr == handle) {
//            return false;
//        }
//
//        int subsamp, cs;
//        int ret = tjDecompressHeader3(handle, jpg_buf, jpg_size, &width, &height, &subsamp, &cs);
//        if (cs == TJCS_GRAY) channels = 1;
//        else channels = 3;
//
//        int pf = TJCS_RGB;
//        int ps = tjPixelSize[pf];
//
//        ret = tjDecompress2(handle, jpg_buf, jpg_size, out_bgr, width, width * channels, height, TJPF_BGR, TJFLAG_NOREALLOC);
//
//        tjDestroy(handle);
//
//        if (ret != 0) {
//            return false;
//        }
//        return true;
//#endif // !_DEBUG
//#endif // !WIN32
//
//        return false;
//    }
//    
//    bool jpg2Bgr(AlarmImage* in_image, unsigned char*& out_bgr) {
//        
//        int jpg_size = in_image->getSize();
//        int height = in_image->getHeight();
//        int width = in_image->getWidth();
//        int channels = in_image->getChannels();
//
//        if (jpg_size <= 0 || height <= 0 || width <= 0) {
//            return false;
//        }
//
//        if (jpg2Bgr_by_turboJpeg(in_image, out_bgr)) {
//            return true;
//        }
//        else {
//            return jpg2Bgr_by_opencv(in_image, out_bgr);
//        }
//
//    }
//
//
//    GenerateAlarmVideo::GenerateAlarmVideo(Config* config, Alarm* alarm) :
//        mConfig(config), mAlarm(alarm)
//    {
//        LOGI("");
//        /*
//#define AV_LOG_QUIET    -8	 保持沉默，不输出
//#define AV_LOG_PANIC     0	 确实出了问题，即将崩溃。
//#define AV_LOG_FATAL     8	 有些地方出了问题，并且不可能修复
//#define AV_LOG_ERROR    16	 有些地方出了问题，不能毫无损失地恢复。
//#define AV_LOG_WARNING  24	 有些东西看起来不太对，有可能出问题
//#define AV_LOG_INFO     32	 标准信息。
//#define AV_LOG_VERBOSE  40	 详细的信息。
//#define AV_LOG_DEBUG    48	 调试信息，只对libav*开发者有用的东西。
//#define AV_LOG_TRACE    56	 非常冗长的调试信息，对libav*开发非常有用。
//*/
//
//        av_log_set_level(AV_LOG_ERROR);
//
//    }
//
//    GenerateAlarmVideo::~GenerateAlarmVideo()
//    {
//        LOGI("");
//        destoryCodecCtx();
//
//    }
//
//    bool GenerateAlarmVideo::initCodecCtx(const char* url) {
//
//        if (avformat_alloc_output_context2(&mFmtCtx, NULL, "mp4", url) < 0) {
//            LOGE("avformat_alloc_output_context2 error");
//            return false;
//        }
//
//        // 初始化视频编码器 start
//        AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
//        if (!videoCodec) {
//            LOGE("avcodec_find_decoder error");
//            return false;
//        }
//        mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
//        if (!mVideoCodecCtx) {
//            LOGE("avcodec_alloc_context3 error");
//            return false;
//        }
//        int bit_rate = 100000;
//
//        // CBR：Constant BitRate - 固定比特率
//    //    mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
//    //    mVideoCodecCtx->bit_rate = bit_rate;
//    //    mVideoCodecCtx->rc_min_rate = bit_rate;
//    //    mVideoCodecCtx->rc_max_rate = bit_rate;
//    //    mVideoCodecCtx->bit_rate_tolerance = bit_rate;
//
//        //VBR
//        mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
//        mVideoCodecCtx->rc_min_rate = bit_rate / 2;
//        mVideoCodecCtx->rc_max_rate = bit_rate / 2 + bit_rate;
//        mVideoCodecCtx->bit_rate = bit_rate;
//
//        //ABR：Average Bitrate - 平均码率
//    //    mVideoCodecCtx->bit_rate = bit_rate;
//
//        mVideoCodecCtx->codec_id = videoCodec->id;
//        mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;// 不支持AV_PIX_FMT_BGR24直接进行编码
//        mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
//        mVideoCodecCtx->width = mAlarm->width;
//        mVideoCodecCtx->height = mAlarm->height;
//        mVideoCodecCtx->time_base = { 1,mAlarm->fps };
//        mVideoCodecCtx->framerate = { mAlarm->fps, 1 };
//        mVideoCodecCtx->gop_size = mAlarm->fps;
//        mVideoCodecCtx->max_b_frames = 5;
//        mVideoCodecCtx->thread_count = 1;
//
//        //mVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;  //全局参数
//
//        unsigned char sps_pps[] = { 0x00 ,0x00 ,0x01,0x67,0x42,0x00 ,0x2a ,0x96 ,0x35 ,0x40 ,0xf0 ,0x04 ,
//                            0x4f ,0xcb ,0x37 ,0x01 ,0x01 ,0x01 ,0x40 ,0x00 ,0x01 ,0xc2 ,0x00 ,0x00 ,0x57 ,
//                            0xe4 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x68 ,0xce ,0x3c ,0x80, 0x00 };
//
//        mVideoCodecCtx->extradata_size = sizeof(sps_pps);
//        mVideoCodecCtx->extradata = (uint8_t*)av_mallocz(mVideoCodecCtx->extradata_size);
//        memcpy(mVideoCodecCtx->extradata, sps_pps, mVideoCodecCtx->extradata_size);
//
//
//        AVDictionary* video_codec_options = NULL;
//        av_dict_set(&video_codec_options, "profile", "main", 0);
//        //av_dict_set(&video_codec_options, "profile", "high", 0);
//        av_dict_set(&video_codec_options, "preset", "superfast", 0);
//        //av_dict_set(&video_codec_options, "tune", "fastdecode", 0);
//
//        if (avcodec_open2(mVideoCodecCtx, videoCodec, &video_codec_options) < 0) {
//            LOGE("avcodec_open2 error");
//            return false;
//        }
//
//        mVideoStream = avformat_new_stream(mFmtCtx, videoCodec);
//        if (!mVideoStream) {
//            LOGE("avformat_new_stream error");
//            return false;
//        }
//        mVideoStream->id = mFmtCtx->nb_streams - 1;
//        // stream的time_base参数非常重要，它表示将现实中的一秒钟分为多少个时间基, 在下面调用avformat_write_header时自动完成
//        avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);
//        mVideoIndex = mVideoStream->id;
//        // 初始化视频编码器 end
//
//
//
//        av_dump_format(mFmtCtx, 0, url, 1);
//
//        // open output url
//        if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
//            if (avio_open(&mFmtCtx->pb, url, AVIO_FLAG_WRITE) < 0) {
//                LOGE("avio_open error url=%s", url);
//                return false;
//            }
//        }
//
//
//        AVDictionary* fmt_options = NULL;
//        //av_dict_set(&fmt_options, "bufsize", "1024", 0);
//        //av_dict_set(&fmt_options, "muxdelay", "0.1", 0);
//        //av_dict_set(&fmt_options, "tune", "zerolatency", 0);
//
//        mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;
//
//        if (avformat_write_header(mFmtCtx, &fmt_options) < 0) { // 调用该函数会将所有stream的time_base，自动设置一个值，通常是1/90000或1/1000，这表示一秒钟表示的时间基长度
//            LOGE("avformat_write_header error");
//            return false;
//        }
//
//        return true;
//    }
//    void GenerateAlarmVideo::destoryCodecCtx() {
//
//        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
//
//        if (mFmtCtx) {
//            // 推流需要释放start
//            if (mFmtCtx && !(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
//                avio_close(mFmtCtx->pb);
//            }
//            // 推流需要释放end
//
//
//
//            avformat_free_context(mFmtCtx);
//            mFmtCtx = NULL;
//        }
//
//        if (mVideoCodecCtx) {
//            if (mVideoCodecCtx->extradata) {
//                av_free(mVideoCodecCtx->extradata);
//                mVideoCodecCtx->extradata = NULL;
//            }
//
//            avcodec_close(mVideoCodecCtx);
//            avcodec_free_context(&mVideoCodecCtx);
//            mVideoCodecCtx = NULL;
//            mVideoIndex = -1;
//        }
//
//
//    }
//    // bgr24转yuv420p
//
//    unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {
//
//        if (x > max_val) {
//            return max_val;
//        }
//        else if (x < min_val) {
//            return min_val;
//        }
//        else {
//            return x;
//        }
//    }
//
//    bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf) {
//
//        unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
//        memset(yuvBuf, 0, w * h * 3 / 2);
//        ptrY = yuvBuf;
//        ptrU = yuvBuf + w * h;
//        ptrV = ptrU + (w * h * 1 / 4);
//        unsigned char y, u, v, r, g, b;
//
//        for (int j = 0; j < h; ++j) {
//
//            ptrRGB = bgrBuf + w * j * 3;
//            for (int i = 0; i < w; i++) {
//
//                b = *(ptrRGB++);
//                g = *(ptrRGB++);
//                r = *(ptrRGB++);
//
//
//                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
//                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
//                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
//                *(ptrY++) = clipValue(y, 0, 255);
//                if (j % 2 == 0 && i % 2 == 0) {
//                    *(ptrU++) = clipValue(u, 0, 255);
//                }
//                else {
//                    if (i % 2 == 0) {
//                        *(ptrV++) = clipValue(v, 0, 255);
//                    }
//                }
//            }
//        }
//        return true;
//    }
//
//
//
//    bool GenerateAlarmVideo::genAlarmVideo() {
//        if (!mAlarm) {
//            return false;
//        }
//
//        std::string rootDir = mConfig->rootDir;
//
//        // C++创建文件夹 https://pythonjishu.com/cgnqifmjqqrgjnj/
//
//        std::filesystem::path path(rootDir);
//        try {
//            if (!std::filesystem::exists(path)) {
//                std::filesystem::create_directory(path);
//            }
//        }
//        catch (std::filesystem::filesystem_error& e) {
//            std::cout << e.what() << std::endl;
//        }
//
//        std::string filename = getCurFormatTimeStr(mConfig->videoFileNameFormat.data());
//
//        std::string video_path = mAlarm->controlCode + "-" + filename +".mp4";//视频相对路径
//        std::string image_path = mAlarm->controlCode + "-" + filename +".jpg";//图片相对路径
//
//        std::string video_absolute_path = rootDir + "/" + video_path;
//        std::string image_absolute_path = rootDir + "/" + image_path;
//
//        if (!initCodecCtx(video_absolute_path.data())) {
//            return false;
//        }
//
//        int width = mAlarm->width;
//        int height = mAlarm->height;
//
//        AVFrame* frame_yuv420p = av_frame_alloc();
//        frame_yuv420p->format = mVideoCodecCtx->pix_fmt;
//        frame_yuv420p->width = width;
//        frame_yuv420p->height = height;
//
//        int frame_yuv420p_buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
//        uint8_t* frame_yuv420p_buff = (uint8_t*)av_malloc(frame_yuv420p_buff_size);
//        av_image_fill_arrays(frame_yuv420p->data, frame_yuv420p->linesize,
//            frame_yuv420p_buff,
//            AV_PIX_FMT_YUV420P,
//            width, height, 1);
//
//
//        AVPacket* pkt = av_packet_alloc();// 编码后的视频帧
//        int64_t  frameCount = 1;
//
//        int ret = -1;
//        int receive_packet_count = -1;
//
//        int out_bgrSize = width * height * 3;
//        unsigned char* out_bgr = (unsigned char*)malloc(out_bgrSize);//创建堆内存
//
//        for (size_t i = 0; i < mAlarm->images.size(); i++)
//        {
//            AlarmImage* image = mAlarm->images[i];
//            //LOGI("----%d,%d,%d,%d----", i, image->getWidth(), image->getHeight(), image->getSize());
//
//            if (jpg2Bgr(image, out_bgr)) {
//                //解压缩成功
//                if (i == mAlarm->happenImageIndex) {
//                    //封面图
//                    cv::Mat happenImage_cvmat(height, width, CV_8UC3, out_bgr);
//                    cv::imwrite(image_absolute_path, happenImage_cvmat);
//                }
//
//                 // frame_bgr 转  frame_yuv420p
//                bgr24ToYuv420p(out_bgr, width, height, frame_yuv420p_buff);
//
//                frame_yuv420p->pts = frame_yuv420p->pkt_dts = av_rescale_q_rnd(frameCount,
//                    mVideoCodecCtx->time_base,
//                    mVideoStream->time_base,
//                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//
//                frame_yuv420p->pkt_duration = av_rescale_q_rnd(1,
//                    mVideoCodecCtx->time_base,
//                    mVideoStream->time_base,
//                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//
//                frame_yuv420p->pkt_pos = frameCount;
//
//
//                ret = avcodec_send_frame(mVideoCodecCtx, frame_yuv420p);
//                if (ret >= 0) {
//                    receive_packet_count = 0;
//                    while (true) {
//                        ret = avcodec_receive_packet(mVideoCodecCtx, pkt);
//                        if (ret >= 0) {
//
//                            //LOGI("encode 1 frame spend：%lld(ms),frameCount=%lld, encodeSuccessCount = %lld, frameQSize=%d,ret=%d", 
//                            //    (t2 - t1), frameCount, encodeSuccessCount, frameQSize, ret);
//
//                            pkt->stream_index = mVideoIndex;
//
//                            pkt->pos = frameCount;
//                            pkt->duration = frame_yuv420p->pkt_duration;
//
//
//                            int wframe = av_write_frame(mFmtCtx, pkt);
//                            if (wframe < 0) {
//                                LOGE("writePkt : wframe=%d", wframe);
//                            }
//                            ++receive_packet_count;
//
//
//                            if (receive_packet_count > 1) {
//                                LOGI("avcodec_receive_packet success: receive_packet_count=%d", receive_packet_count);
//                            }
//                        }
//                        else {
//                            if (0 == receive_packet_count) {
//                                LOGE("avcodec_receive_packet error : ret=%d", ret);
//                            }
//
//                            break;
//                        }
//                    }
//
//                }
//                else {
//                    LOGE("avcodec_send_frame error : ret=%d", ret);
//                }
//                frameCount++;
//
//
//
//                //std::string imageName = mAlarm->videoDir + "\\" + std::to_string(getCurTimestamp()) + "_" + std::to_string(Common_GetRandom())+"_" + std::to_string(i) + ".jpg";
//                //Common_SaveCompressImage(image, imageName);
//                //bool s = false;
//                //bool s = Common_SaveBgr(image->getHeight(), image->getWidth(), image->getChannels(),
//                //    bgr, imageName);
//                //printf("%s,s=%d\n", imageName.data(),s);
//
//
//            }
//            else {
//                LOGE("jpg2Bgr error");
//            }
//            delete image;
//            image = nullptr;
//
//        }
//
//        mAlarm->images.clear();
//
//        free(out_bgr);
//        out_bgr = nullptr;
//
//        av_write_trailer(mFmtCtx);//写文件尾
//
//        av_packet_unref(pkt);
//        pkt = nullptr;
//
//
//        av_free(frame_yuv420p_buff);
//        frame_yuv420p_buff = nullptr;
//
//        av_frame_free(&frame_yuv420p);
//        //av_frame_unref(frame_yuv420p);
//        frame_yuv420p = nullptr;
//
//        //上传报警信息start
//        std::string url = mConfig->adminHost + "/api/postAddAlarm";
//        Json::Value param;
//        param["control_code"] = mAlarm->controlCode;
//        param["desc"] = filename;
//        param["video_path"] = video_path;
//        param["image_path"] = image_path;
//
//        std::string data = param.toStyledString();
//        Request request;
//        std::string response;
//        bool res = request.post(url.data(), data.data(), response);
//        //上传报警信息end
//
//        return true;
//
//    }
//
//
//}
#include "Alarm.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include <json/json.h>
#include "Utils/Request.h"
#include "Frame.h"
#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

#pragma warning(disable: 4996)

namespace AVSAnalyzer {


    Alarm::Alarm(int height, int width, int fps, int64_t happenTimestamp, int happenImageIndex, const char* controlCode, std::string warnning) {
        LOGI("");

        this->height = height;
        this->width = width;
        this->fps = fps;
        this->happenTimestamp = happenTimestamp;
        this->happenImageIndex = happenImageIndex;
        this->controlCode = controlCode;
        this->warnning = warnning;
    }
    Alarm::~Alarm() {
        LOGI("");

        for (size_t i = 0; i < this->frames.size(); i++)
        {
            Frame* frame = this->frames[i];
            delete frame;
            frame = nullptr;
        }
        frames.clear();

    }
    GenerateAlarmVideo::GenerateAlarmVideo(Config* config, Alarm* alarm) :
        mConfig(config), mAlarm(alarm)
    {
        LOGI("");
        /*
#define AV_LOG_QUIET    -8	 保持沉默，不输出
#define AV_LOG_PANIC     0	 确实出了问题，即将崩溃。
#define AV_LOG_FATAL     8	 有些地方出了问题，并且不可能修复
#define AV_LOG_ERROR    16	 有些地方出了问题，不能毫无损失地恢复。
#define AV_LOG_WARNING  24	 有些东西看起来不太对，有可能出问题
#define AV_LOG_INFO     32	 标准信息。
#define AV_LOG_VERBOSE  40	 详细的信息。
#define AV_LOG_DEBUG    48	 调试信息，只对libav*开发者有用的东西。
#define AV_LOG_TRACE    56	 非常冗长的调试信息，对libav*开发非常有用。
*/

        av_log_set_level(AV_LOG_ERROR);

    }

    GenerateAlarmVideo::~GenerateAlarmVideo()
    {
        LOGI("");
        destoryCodecCtx();

    }

    bool GenerateAlarmVideo::initCodecCtx(const char* url) {

        if (avformat_alloc_output_context2(&mFmtCtx, NULL, "mp4", url) < 0) {
            LOGE("avformat_alloc_output_context2 error");
            return false;
        }

        // 初始化视频编码器 start
        const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            LOGE("avcodec_find_decoder error");
            return false;
        }
        mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
        if (!mVideoCodecCtx) {
            LOGE("avcodec_alloc_context3 error");
            return false;
        }
        int bit_rate = 100000;

        // CBR：Constant BitRate - 固定比特率
    //    mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    //    mVideoCodecCtx->bit_rate = bit_rate;
    //    mVideoCodecCtx->rc_min_rate = bit_rate;
    //    mVideoCodecCtx->rc_max_rate = bit_rate;
    //    mVideoCodecCtx->bit_rate_tolerance = bit_rate;

        //VBR
        mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
        mVideoCodecCtx->rc_min_rate = bit_rate / 2;
        mVideoCodecCtx->rc_max_rate = bit_rate / 2 + bit_rate;
        mVideoCodecCtx->bit_rate = bit_rate;

        //ABR：Average Bitrate - 平均码率
    //    mVideoCodecCtx->bit_rate = bit_rate;

        mVideoCodecCtx->codec_id = videoCodec->id;
        mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;// 不支持AV_PIX_FMT_BGR24直接进行编码
        mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        mVideoCodecCtx->width = mAlarm->width;
        mVideoCodecCtx->height = mAlarm->height;
        mVideoCodecCtx->time_base = { 1,mAlarm->fps };
        mVideoCodecCtx->framerate = { mAlarm->fps, 1 };
        mVideoCodecCtx->gop_size = mAlarm->fps;
        mVideoCodecCtx->max_b_frames = 5;
        mVideoCodecCtx->thread_count = 1;

        //mVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;  //全局参数

        unsigned char sps_pps[] = { 0x00 ,0x00 ,0x01,0x67,0x42,0x00 ,0x2a ,0x96 ,0x35 ,0x40 ,0xf0 ,0x04 ,
                            0x4f ,0xcb ,0x37 ,0x01 ,0x01 ,0x01 ,0x40 ,0x00 ,0x01 ,0xc2 ,0x00 ,0x00 ,0x57 ,
                            0xe4 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x68 ,0xce ,0x3c ,0x80, 0x00 };

        mVideoCodecCtx->extradata_size = sizeof(sps_pps);
        mVideoCodecCtx->extradata = (uint8_t*)av_mallocz(mVideoCodecCtx->extradata_size);
        memcpy(mVideoCodecCtx->extradata, sps_pps, mVideoCodecCtx->extradata_size);


        AVDictionary* video_codec_options = NULL;
        av_dict_set(&video_codec_options, "profile", "main", 0);
        //av_dict_set(&video_codec_options, "profile", "high", 0);
        av_dict_set(&video_codec_options, "preset", "superfast", 0);
        //av_dict_set(&video_codec_options, "tune", "fastdecode", 0);

        if (avcodec_open2(mVideoCodecCtx, videoCodec, &video_codec_options) < 0) {
            LOGE("avcodec_open2 error");
            return false;
        }

        mVideoStream = avformat_new_stream(mFmtCtx, videoCodec);
        if (!mVideoStream) {
            LOGE("avformat_new_stream error");
            return false;
        }
        mVideoStream->id = mFmtCtx->nb_streams - 1;
        // stream的time_base参数非常重要，它表示将现实中的一秒钟分为多少个时间基, 在下面调用avformat_write_header时自动完成
        avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);
        mVideoIndex = mVideoStream->id;
        // 初始化视频编码器 end



        av_dump_format(mFmtCtx, 0, url, 1);

        // open output url
        if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&mFmtCtx->pb, url, AVIO_FLAG_WRITE) < 0) {
                LOGE("avio_open error url=%s", url);
                return false;
            }
        }


        AVDictionary* fmt_options = NULL;
        //av_dict_set(&fmt_options, "bufsize", "1024", 0);
        //av_dict_set(&fmt_options, "muxdelay", "0.1", 0);
        //av_dict_set(&fmt_options, "tune", "zerolatency", 0);

        mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;

        if (avformat_write_header(mFmtCtx, &fmt_options) < 0) { // 调用该函数会将所有stream的time_base，自动设置一个值，通常是1/90000或1/1000，这表示一秒钟表示的时间基长度
            LOGE("avformat_write_header error");
            return false;
        }

        return true;
    }
    void GenerateAlarmVideo::destoryCodecCtx() {

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (mFmtCtx) {
            // 推流需要释放start
            if (mFmtCtx && !(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_close(mFmtCtx->pb);
            }
            // 推流需要释放end



            avformat_free_context(mFmtCtx);
            mFmtCtx = NULL;
        }

        if (mVideoCodecCtx) {
            if (mVideoCodecCtx->extradata) {
                av_free(mVideoCodecCtx->extradata);
                mVideoCodecCtx->extradata = NULL;
            }

            avcodec_close(mVideoCodecCtx);
            avcodec_free_context(&mVideoCodecCtx);
            mVideoCodecCtx = NULL;
            mVideoIndex = -1;
        }


    }
    // bgr24转yuv420p

    unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {

        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }

    bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf) {

        unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
        memset(yuvBuf, 0, w * h * 3 / 2);
        ptrY = yuvBuf;
        ptrU = yuvBuf + w * h;
        ptrV = ptrU + (w * h * 1 / 4);
        unsigned char y, u, v, r, g, b;

        for (int j = 0; j < h; ++j) {

            ptrRGB = bgrBuf + w * j * 3;
            for (int i = 0; i < w; i++) {

                b = *(ptrRGB++);
                g = *(ptrRGB++);
                r = *(ptrRGB++);


                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrY++) = clipValue(y, 0, 255);
                if (j % 2 == 0 && i % 2 == 0) {
                    *(ptrU++) = clipValue(u, 0, 255);
                }
                else {
                    if (i % 2 == 0) {
                        *(ptrV++) = clipValue(v, 0, 255);
                    }
                }
            }
        }
        return true;
    }



    bool GenerateAlarmVideo::genAlarmVideo() {
        if (!mAlarm) {
            return false;
        }

        std::string rootDir = mConfig->rootDir;
        // C++创建文件夹 https://pythonjishu.com/cgnqifmjqqrgjnj/

        std::filesystem::path path(rootDir);
        try {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directory(path);
            }
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cout << e.what() << std::endl;
        }

        std::string filename = getCurFormatTimeStr(mConfig->videoFileNameFormat.data());

        std::string video_path = mAlarm->controlCode + "-" + filename + ".mp4";//视频相对路径
        std::string image_path = mAlarm->controlCode + "-" + filename + ".jpg";//图片相对路径

        std::string video_absolute_path = rootDir + "/" + video_path;
        std::string image_absolute_path = rootDir + "/" + image_path;

        if (!initCodecCtx(video_absolute_path.data())) {
            return false;
        }

        int width = mAlarm->width;
        int height = mAlarm->height;
        std::string warnning= mAlarm->warnning;

        AVFrame* frame_yuv420p = av_frame_alloc();
        frame_yuv420p->format = mVideoCodecCtx->pix_fmt;
        frame_yuv420p->width = width;
        frame_yuv420p->height = height;

        int frame_yuv420p_buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
        uint8_t* frame_yuv420p_buff = (uint8_t*)av_malloc(frame_yuv420p_buff_size);
        av_image_fill_arrays(frame_yuv420p->data, frame_yuv420p->linesize,
            frame_yuv420p_buff,
            AV_PIX_FMT_YUV420P,
            width, height, 1);


        AVPacket* pkt = av_packet_alloc();// 编码后的视频帧
        int64_t  frameCount = 1;

        int ret = -1;
        int receive_packet_count = -1;

        for (size_t i = 0; i < mAlarm->frames.size(); i++)
        {
            Frame* frame = mAlarm->frames[i];
            //LOGI("----%d,%d,%d,%d----", i, image->getWidth(), image->getHeight(), image->getSize());


            //解压缩成功
            if (i == mAlarm->happenImageIndex) {
                //封面图
                cv::Mat happenImage_cvmat(height, width, CV_8UC3, frame->getBuf());
                cv::imwrite(image_absolute_path, happenImage_cvmat);
            }

            // frame_bgr 转  frame_yuv420p
            bgr24ToYuv420p(frame->getBuf(), width, height, frame_yuv420p_buff);

            frame_yuv420p->pts = frame_yuv420p->pkt_dts = av_rescale_q_rnd(frameCount,
                mVideoCodecCtx->time_base,
                mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_yuv420p->pkt_duration = av_rescale_q_rnd(1,
                mVideoCodecCtx->time_base,
                mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_yuv420p->pkt_pos = frameCount;


            ret = avcodec_send_frame(mVideoCodecCtx, frame_yuv420p);
            if (ret >= 0) {
                receive_packet_count = 0;
                while (true) {
                    ret = avcodec_receive_packet(mVideoCodecCtx, pkt);
                    if (ret >= 0) {

                        //LOGI("encode 1 frame spend：%lld(ms),frameCount=%lld, encodeSuccessCount = %lld, frameQSize=%d,ret=%d", 
                        //    (t2 - t1), frameCount, encodeSuccessCount, frameQSize, ret);

                        pkt->stream_index = mVideoIndex;

                        pkt->pos = frameCount;
                        pkt->duration = frame_yuv420p->pkt_duration;


                        int wframe = av_write_frame(mFmtCtx, pkt);
                        if (wframe < 0) {
                            LOGE("writePkt : wframe=%d", wframe);
                        }
                        ++receive_packet_count;


                        if (receive_packet_count > 1) {
                            LOGI("avcodec_receive_packet success: receive_packet_count=%d", receive_packet_count);
                        }
                    }
                    else {
                        if (0 == receive_packet_count) {
                            LOGE("avcodec_receive_packet error : ret=%d", ret);
                        }

                        break;
                    }
                }

            }
            else {
                LOGE("avcodec_send_frame error : ret=%d", ret);
            }
            frameCount++;



            //std::string imageName = mAlarm->videoDir + "\\" + std::to_string(getCurTimestamp()) + "_" + std::to_string(Common_GetRandom())+"_" + std::to_string(i) + ".jpg";
            //Common_SaveCompressImage(image, imageName);
            //bool s = false;
            //bool s = Common_SaveBgr(image->getHeight(), image->getWidth(), image->getChannels(),
            //    bgr, imageName);
            //printf("%s,s=%d\n", imageName.data(),s);



            delete frame;
            frame = nullptr;

        }

        mAlarm->frames.clear();

        av_write_trailer(mFmtCtx);//写文件尾

        av_packet_unref(pkt);
        pkt = nullptr;


        av_free(frame_yuv420p_buff);
        frame_yuv420p_buff = nullptr;

        av_frame_free(&frame_yuv420p);
        //av_frame_unref(frame_yuv420p);
        frame_yuv420p = nullptr;

        //上传报警信息start
        std::string url = mConfig->adminHost + "/api/postAddAlarm";
        Json::Value param;
        param["control_code"] = mAlarm->controlCode;
        param["desc"] = filename;
        param["video_path"] = video_path;
        param["image_path"] = image_path;
        param["category"] = warnning;
        std::string data = param.toStyledString();
        Request request;
        std::string response;
        bool res = request.post(url.data(), data.data(), response);
        LOGI("%d", int(res));
        LOGI("%s", response);
        //上传报警信息end

        return true;

    }


}
