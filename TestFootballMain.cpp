#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//#include<linux/fb.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <sched.h>

#include "FootballConfig.h"

static const char* kTAG = "football-main";
#include "android_logcat_.h"

#include "ndk_extend/NativeHooApiLoader.h"

#include "TestCmdline.h"

#include "miniled/TestCmdMiniLed.h"

#include "StbImage_.h"

extern "C" {
	extern int luac_main_(int argc, char* argv[]);
	extern int lua_main_(int argc, char* argv[]);
};

#undef UNUSED_
#define UNUSED_(x) ((void)x)


static void print_version() {
	fprintf(stderr, "version: %03d.%03d.%03d \r\n",
		VERSION_MAJOR(FOOTBALL_VERSION), 
		VERSION_MINOR(FOOTBALL_VERSION),
		VERSION_develop(FOOTBALL_VERSION));
}
int main(int argc, char **argv) {
	UNUSED_(argc); UNUSED_(argv);

	///return lua_main_(argc, argv);

	fprintf(stderr, "Hello,world! \r\n");
	print_version();

	::android::ndk::extend::android_ndk_extend_initialize();

    static const struct option longOptions[] = {
        { "help",               no_argument,        NULL, 'h' },
        { "version",            no_argument,        NULL, 'v' },
        { "verbose",            no_argument,        NULL, 'b' },
        { "daemon",             no_argument,  		NULL, 'm' },
        { "dummy",              no_argument,  		NULL, 'd' },
        { NULL,                 0,                  NULL, 0 }
    };
	int run_for_daemon = 0;
	while (true) {
		int optionIndex = 0;
		int ic = getopt_long(argc, argv, "", longOptions, &optionIndex);
		if (ic == -1) {
			break;
		}
		switch (ic) {
		case 'h': {
			printf("no option		run for cmdline debug tool \r\n");
			printf("--help			print this message \r\n");
			printf("--verbose		verbose log \r\n");
			printf("--daemon		run for daemon \r\n");
			printf("--dummy 		dummy \r\n");
			return 0;
		}
		case 'v': {
			print_version();
			return 0;
		}
		case 'b': {
			break;
		}
		case 'm': {
			run_for_daemon = 1;
			break;
		}
		}
	}

//////////////////////////////////////////////////////////////////
	if (run_for_daemon) {

		daemon(0, 0) ;

		{
			uint32_t initFlags = 0x00;
			NS_test_cmdline::TestCmdline *_test = new NS_test_cmdline::TestCmdline(initFlags);
			if (_test->isValid()) {
				_test->initCmd();
				_test->runCommand("vd 1 0");
				_test->loop();
			}
			else {
				fprintf(stderr, "%s,%d not valid!", __func__, __LINE__);
			}
			delete _test;
		}

		#if 0
		{
			uint64_t running_tick = 0; // ~ 584,942,417,355 years to overflow
			while(1) {
				LOGW("%s running_tick %" PRIu64 "", __func__, running_tick++);
				usleep(1*1000*1000);
			}
		}
		#endif

		::android::ndk::extend::android_ndk_extend_uninitialize();
		return 0;
	}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


#if 1  // test cmdline	
	{
		uint32_t initFlags = 0x00;
		NS_test_cmdline::TestCmdline *_test = new NS_test_cmdline::TestCmdline(initFlags);
		if (_test->isValid()) {
			_test->initCmd();
			_test->loop();
		}
		else {
			fprintf(stderr, "%s,%d TestCmdline not valid!", __func__, __LINE__);
		}
		delete _test;
	}
#endif

#if 0  // test miniled spi device
	{
		uint32_t initFlags = 0x00;
		NS_test_miniled::TestCmdMiniLed *_test = new NS_test_miniled::TestCmdMiniLed(initFlags);
		if (_test->isValid()) {
			_test->initCmd();
			_test->loop();
		}
		else {
			fprintf(stderr, "%s,%d not valid!", __func__, __LINE__);
		}
		delete _test;

	}
#endif

#if 0
	football::StbImage_::test1();
#endif


	::android::ndk::extend::android_ndk_extend_uninitialize();
	return 0;
}

