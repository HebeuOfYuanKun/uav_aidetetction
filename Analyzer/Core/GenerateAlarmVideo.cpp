#include "GenerateAlarmVideo.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include <json/json.h>
#include "Utils/Request.h"
#include "Frame.h"
#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

#pragma warning(disable: 4996)

namespace AVSAnalyzer {


    Alarm::Alarm(int height, int width, int fps, std::string category, int64_t happenTimestamp, int happenImageIndex, const char* controlCode) {
        //LOGI("");

        this->height = height;
        this->width = width;
        this->fps = fps;
        this->category = category;
        this->happenTimestamp = happenTimestamp;
        this->happenImageIndex = happenImageIndex;
        this->controlCode = controlCode;
    }
    Alarm::~Alarm() {
        //LOGI("");

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
        //LOGI("");
        av_log_set_level(AV_LOG_ERROR);

    }

    GenerateAlarmVideo::~GenerateAlarmVideo()
    {
        //LOGI("");
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
        // C++ 17创建文件夹 https://pythonjishu.com/cgnqifmjqqrgjnj/


        if (!mAlarm) {
            return false;
        }

        std::string prifixDir = mConfig->uploadDir + "/alarm";

        std::filesystem::path __prifixDir_path1(prifixDir);
        try {
            if (!std::filesystem::exists(__prifixDir_path1)) {
                std::filesystem::create_directory(__prifixDir_path1);
            }
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cout <<"genAlarmVideo() create_directory1 error:" << e.what() << std::endl;
            return false;
        }

        prifixDir = prifixDir +"/" + mAlarm->controlCode;

        std::filesystem::path __prifixDir_path2(prifixDir);
        try {
            if (!std::filesystem::exists(__prifixDir_path2)) {
                std::filesystem::create_directory(__prifixDir_path2);
            }
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cout << "genAlarmVideo() create_directory2 error:" << e.what() << std::endl;
            return false;
        }
        std::string ymdhms_rd = getCurFormatTimeStr("%Y%m%d%H%M%S") + "_" + std::to_string(getRandomInt());
        prifixDir = prifixDir + "/" + ymdhms_rd;
        std::filesystem::path __prifixDir_path3(prifixDir);
        try {
            if (!std::filesystem::exists(__prifixDir_path3)) {
                std::filesystem::create_directory(__prifixDir_path3);
            }
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cout << "genAlarmVideo() create_directory3 error:" << e.what() << std::endl;
            return false;
        }



        std::string video_path = "alarm/"+ mAlarm->controlCode +"/" + ymdhms_rd + "/main.mp4";//视频相对路径
        std::string image_path = "alarm/" + mAlarm->controlCode + "/" + ymdhms_rd + "/main.jpg";//图片相对路径

        std::string video_path_abs = mConfig->uploadDir + "/" + video_path;
        std::string image_path_abs = mConfig->uploadDir + "/" + image_path;

        if (!initCodecCtx(video_path_abs.data())) {
            std::cout << "genAlarmVideo() initCodecCtx error" << std::endl;
            return false;
        }

        int width = mAlarm->width;
        int height = mAlarm->height;

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
                cv::imwrite(image_path_abs, happenImage_cvmat);
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
                            //LOGE("avcodec_receive_packet error : ret=%d", ret);
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
        std::string url = mConfig->adminHost + "/service/business/alarm/add";
        Json::Value param;
        param["controlCode"] = mAlarm->controlCode;
        param["desc"] = "";
        param["category"] = mAlarm->category;
        param["videoPath"] = video_path;
        param["imagePath"] = image_path;
        
        std::string data = param.toStyledString();
        std::cout << data;
        Request request;
        //request.setHeader("Expect", "");  // 移除 Expect 头
        //request.setHeader("Content-Type", "application/json");  // 设置 Content-Type 为 application/json
        std::string response;
        bool res = request.post(url.data(), data.data(), response);

        LOGI("\n \t request:%s \n \t response:%s",
            url.data(),
            response.data());


        return true;

    }


}
