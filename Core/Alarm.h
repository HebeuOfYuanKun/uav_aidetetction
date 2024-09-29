//#ifndef ANALYZER_ALARM_H
//#define ANALYZER_ALARM_H
//#include <vector>
//#include <queue>
//#include <mutex>
//extern "C" {
//#include "libavcodec/avcodec.h"
//#include "libavformat/avformat.h"
//}
//#define alarm_image_jpg_buf_max_size 800000
//
//namespace AVSAnalyzer {
//
//	class Config;
//
//	struct AlarmImage
//	{
//	public:
//		AlarmImage() = delete;
//		AlarmImage(int width, int height, int channels);
//		~AlarmImage();
//	public:
//		void setBuf(unsigned char* buf, int size);
//
//		bool  happen = false;// 是否发生事件
//		float happenScore = 0;// 发生事件的分数
//
//		unsigned char* getBuf();
//		int getSize();
//		int getWidth();
//		int getHeight();
//		int getChannels();
//	private:
//
//		unsigned char mJpgBuf[alarm_image_jpg_buf_max_size]; //图片经过jpg压缩后的数据
//		int mJpgBufSize = 0;								 //图片经过jpg压缩后的数据长度
//		int mWidth = 0;                //原图宽
//		int mHeight = 0;               //原图高
//		int mChannels = 0;             //原图通道长
//	};
//	struct Alarm
//	{
//	public:
//		Alarm() = delete;
//		Alarm(int height, int width, int fps, int64_t happenTimestamp,int happenImageIndex, const char* controlCode);
//		~Alarm();
//	public:
//		int width = 0;
//		int height = 0;
//		int fps = 0;
//		int64_t happenTimestamp = 0; //发生事件的时间戳（毫秒级）
//		int		happenImageIndex = 0;//封面图index
//		std::string controlCode;// 布控编号
//		std::vector<AlarmImage*> images;//组成报警视频的图片帧
//	};
//	//bgr图像压缩为jpg格式
//	bool bgr2Jpg(int height, int width, int channels, unsigned char* in_bgr, AlarmImage* out_image);
//	//jpg图像解压缩未bgr格式
//	bool jpg2Bgr(AlarmImage* in_image, unsigned char*& out_bgr);
//
//	class GenerateAlarmVideo
//	{
//	public:
//		GenerateAlarmVideo() = delete;
//		GenerateAlarmVideo(Config* config, Alarm* alarm);
//		~GenerateAlarmVideo();
//
//	public:
//		bool genAlarmVideo();
//	private:
//		Config* mConfig;
//		Alarm* mAlarm;
//		bool initCodecCtx(const char* url);
//		void destoryCodecCtx();
//
//		AVFormatContext* mFmtCtx = nullptr;
//		//视频帧
//		AVCodecContext* mVideoCodecCtx = nullptr;
//		AVStream* mVideoStream = nullptr;
//		int mVideoIndex = -1;
//	};
//
//}
//
//#endif //ANALYZER_ALARM_H
#ifndef ANALYZER_GENERATEALARMVIDEO_H
#define ANALYZER_GENERATEALARMVIDEO_H
#include <vector>
#include <queue>
#include <mutex>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

namespace AVSAnalyzer {

	class Config;
	struct Frame;

	struct Alarm
	{
	public:
		Alarm() = delete;
		Alarm(int height, int width, int fps, int64_t happenTimestamp, int happenImageIndex, const char* controlCode, std::string warnning);
		~Alarm();
	public:
		int width = 0;
		int height = 0;
		int fps = 0;
		int64_t happenTimestamp = 0; //发生事件的时间戳（毫秒级）
		int		happenImageIndex = 0;//封面图index
		std::string controlCode;// 布控编号
		std::vector<Frame*> frames;//组成报警视频的图片帧
		std::string warnning;//报警的种类
	};


	class GenerateAlarmVideo
	{
	public:
		GenerateAlarmVideo() = delete;
		GenerateAlarmVideo(Config* config, Alarm* alarm);
		~GenerateAlarmVideo();

	public:
		bool genAlarmVideo();
	private:
		Config* mConfig;
		Alarm* mAlarm;
		bool initCodecCtx(const char* url);
		void destoryCodecCtx();

		AVFormatContext* mFmtCtx = nullptr;
		//视频帧
		AVCodecContext* mVideoCodecCtx = nullptr;
		AVStream* mVideoStream = nullptr;
		int mVideoIndex = -1;
	};

}

#endif //ANALYZER_GENERATEALARMVIDEO_H