
#include "cmdline_v1/CmdlineV1.h"
#include "cmdline_v2/CmdlineV2.h"
#include "cmdline_v2/CmdlineV3.h"
#include "cmdline_lua/CmdlineLuaImpl.h"

#include "CmdLineFactory.h"

namespace android {

/*static*/ const char *Cmdline::kCmd_quit = "quit";
/*static*/ const char *Cmdline::kCmd_q_ = "q";
/*static*/ const char *Cmdline::kCmd_clversion = "clversion";
/*static*/ const char *Cmdline::kCmd_help = "help";
/*static*/ const char *Cmdline::kCmd_h_ = "h";

namespace cmdline {
namespace factory {

Cmdline *makeCmdline(int ver, int opt) {
	if (ver == CMDLINE_V3) {
		return new CmdlineV3(opt);
	}
	else if(ver == CMDLINE_V2) {
		return new CmdlineV2(opt);
	}
	else if(ver == CMDLINE_LUA) {
		//return new CmdlineLuaImpl();
	}
	return nullptr;
}


};
};
};

