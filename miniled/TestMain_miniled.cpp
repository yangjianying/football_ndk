
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/resource.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include <sys/mman.h>

//#include<linux/fb.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "TestCmdMiniLed.h"

#undef JNI_FR_DEBUG_H_
#if 1  // must @ last
#undef config_fr_DEBUG_EN
#undef config_fr_DEBUG_TO_CONSOLE
#define config_fr_DEBUG_EN 1  //  (1)
#define config_fr_DEBUG_TO_CONSOLE 1  //  1 //  0
#include "cycling_utils/cy_debug.h"
#endif
#undef LOG_TAG
#define LOG_TAG "TestMain_miniled"


#if 1
#undef UNUSED_
#define UNUSED_(x) ((void)x)


int main(int argc, char **argv) {
	UNUSED_(argc);
	UNUSED_(argv);
	//FrLOGV(LOG_TAG, "%s,%d", __func__, __LINE__);

    static const struct option longOptions[] = {
        { "help",               no_argument,        NULL, 'h' },
        { "verbose",            no_argument,        NULL, 'v' },
        { "capi",               no_argument,  NULL, 'c' },
        { "qdcm",               no_argument,  NULL, 'q' },
        { "dummy",               required_argument,  NULL, 'd' },
        { NULL,                 0,                  NULL, 0 }
    };

	int capi = 0;
	int qdcm_enable = 0;
    while (true) {
        int optionIndex = 0;
        int ic = getopt_long(argc, argv, "", longOptions, &optionIndex);
        if (ic == -1) {
            break;
        }
		switch (ic) {
		case 'h': {
			break;
		}
		case 'v': {
			break;
		}
		case 'c': {
			capi = 1;
			break;
		}
		case 'q': {
			qdcm_enable = 1;
			break;
		}
		}
    }

	{
		uint32_t initFlags = 0;
		if (qdcm_enable) {
			initFlags |= 0x8000;
		}
		if (capi) {
			initFlags |= 0x4000;
		}
		::android::sp<::NS_test_miniled::TestCmdMiniLed> _test =
			new ::NS_test_miniled::TestCmdMiniLed(initFlags);
		if (_test->isValid()) {
			_test->loop();
		}
		else {
			FrLOGV(LOG_TAG, "%s,%d not valid!", __func__, __LINE__);
		}
	}


	return 0;
}
#endif

