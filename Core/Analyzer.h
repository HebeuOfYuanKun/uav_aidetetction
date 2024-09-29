#ifndef ANALYZER_ANALYZER_H
#define ANALYZER_ANALYZER_H

#include <string>
#include <vector>

namespace AVSAnalyzer {
	struct Control;
	class Config;
	class Scheduler;
	class Algorithm;
	struct DetectObject;

	class Analyzer
	{
	public:
		explicit Analyzer(Scheduler* scheduler, Control* control);
		~Analyzer();
		bool handleVideoFrame(int64_t frameCount, unsigned char* bgr, bool& happen, float& happenScore, int& count_gap, int& gap, std::string& warnning);
		/*public:
			bool handleVideoFrame(int64_t frameCount, unsigned char* bgr, bool& happen, float& happenScore, int& count_gap, int& gap);*/

	private:
		Scheduler* mScheduler;
		Control* mControl;
		std::vector<DetectObject> mDetects;
	};
}
#endif //ANALYZER_ANALYZER_H

