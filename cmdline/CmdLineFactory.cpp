
#include "cmdline_v1/CmdlineV1.h"
#include "cmdline_v2/CmdlineV2.h"
#include "cmdline_lua/CmdlineLuaImpl.h"

#include "CmdLineFactory.h"

namespace android {
namespace cmdline {
namespace factory {

Cmdline *makeCmdline() {
	return new CmdlineV2();
	//return new CmdlineLuaImpl();
}


};
};
};

