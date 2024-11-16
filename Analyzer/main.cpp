#include "Core/Config.h"
#include "Core/Scheduler.h"
#include "Core/Server.h"
#include "Core/Utils/Version.h" 
#include "Core/Algorithm.h"

using namespace AVSAnalyzer;

int main(int argc, char** argv)
{
#ifdef WIN32
	srand(time(NULL));//时间初始化
#endif // WIN32

	const char* file = NULL;

	for (int i = 1; i < argc; i += 2)
	{
		if (argv[i][0] != '-')
		{
			printf("parameter error:%s\n", argv[i]);
			return -1;
		}
		switch (argv[i][1])
		{
			case 'h': {
				//打印help信息
				printf("-h 打印参数配置信息并退出\n");
				printf("-f 配置文件    如：-f config.json \n");
				system("pause\n"); 
				exit(0); 
				return -1;
			}
			case 'f': {
				file = argv[i + 1];
				break;
			}
			default: {
				printf("set parameter error:%s\n", argv[i]);
				return -1;

			}
		}
	}
	
	if (file == NULL) {
		printf("failed to read config file\n");
		return -1;
	}
	Config config(file);
	if (!config.mState) {
		printf("failed to read config file: %s\n", file);
		return -1;
	}

	printf("Analyzer %s \n", PROJECT_VERSION);
	//config.show();
	printf("无人机智慧识别模块 %s \n", PROJECT_VERSION);
	config.show();
	printf("\n");
	printf("请注意! 上面打印的配置参数中，有涉及路径的参数，一定要在config.json修改成自己的路径，否则程序一定会报错的，如果不知道config.json各个参数代表什么意思，请参考README.md\n");
	printf("\n");
	printf("%s 发布于2024.9.29.\n", PROJECT_VERSION);

	printf("\n");
	printf("\n");

	Scheduler scheduler(&config);
	if (!scheduler.initAlgorithm()) {
		return -1;
	}
	Server server;
	server.start(&scheduler);
	scheduler.loop();

	return 0;
}