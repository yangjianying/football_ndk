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



#include "TestCmdline.h"

static const char* kTAG = "footballd";
#include "android_logcat_.h"

#undef __CLASS__
#define __CLASS__ "footballd"

int main(int argc, char **argv) {
	return ::NS_test_cmdline::TestCmdline::_main(argc, argv);
	//return ::NS_test_miniled::TestCmdMiniLed::_main(argc, argv);
}


