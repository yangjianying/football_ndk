#ifndef __CMDLINE_FACTORY_H__
#define __CMDLINE_FACTORY_H__


#include "CmdLine.h"

namespace android {

enum {
	CMDLINE_V1 = 0,
	CMDLINE_V2 = 1,
	CMDLINE_V3 = 2,
	CMDLINE_LUA = 10,
};

namespace cmdline {
namespace factory {

Cmdline *makeCmdline(int ver = CMDLINE_V2, int opt = CLI_OPT_console_reader);


};
};
};

#endif

