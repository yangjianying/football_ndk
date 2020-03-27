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



#undef __CLASS__
#define __CLASS__ "FootballConfig"

extern "C" {
	extern int luac_main_(int argc, char* argv[]);
	extern int lua_main_(int argc, char* argv[]);
};

#undef UNUSED_
#define UNUSED_(x) ((void)x)

namespace football {

/*static*/ const char *FootballConfig::cycling_service_name = "cycling_service1";

/*static*/ void FootballConfig::print_version() {
	DLOGD("version: %03d.%03d.%03d, build at %s %s \r\n",
		__FOOTBALL_VERSION_MAJOR(FOOTBALL_VERSION), 
		__FOOTBALL_VERSION_MINOR(FOOTBALL_VERSION),
		__FOOTBALL_VERSION_develop(FOOTBALL_VERSION),
		__DATE__, __TIME__);
}


};


