
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CmdlineLuaImpl.h"



extern "C" {
	extern int luac_main_(int argc, char* argv[]);
	extern int lua_main_(int argc, char* argv[]);
};

namespace android {

CmdlineLuaImpl::CmdlineLuaImpl() {

}
CmdlineLuaImpl::~CmdlineLuaImpl() {

}
	// prompt is set by CONFIG_SYS_PROMPT macro !!!
void CmdlineLuaImpl::setPrompt(const char *prompt) {

}
void CmdlineLuaImpl::addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) {

}
void CmdlineLuaImpl::onEmptyCmd() {

}
int CmdlineLuaImpl::add(
	const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) {

	return 0;
}
int CmdlineLuaImpl::loop() {

	return 0;
}
int CmdlineLuaImpl::runCommand(const char * cmd) {

	return 0;
}

/*static*/ int CmdlineLuaImpl::test(int argc, char* argv[]) {
	return 0;
}


};



