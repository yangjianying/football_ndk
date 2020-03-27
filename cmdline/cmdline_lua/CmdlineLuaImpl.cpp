
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
void CmdlineLuaImpl::add_on_empty_cmd(PF_on_empty_cmd cb, void *ctx) {
}
void CmdlineLuaImpl::add_on_intercept_command(PF_on_intercept_command, void *ctx) {
}
void CmdlineLuaImpl::on_empty_cmd_i() {
}
int CmdlineLuaImpl::add(
	const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) {

	return 0;
}
int CmdlineLuaImpl::loop() {

	return 0;
}
int CmdlineLuaImpl::check_command_matched(const char * cmd, const char *matched_) {
	return 0;
}
int CmdlineLuaImpl::postCommand(const char * cmd) {
	return -1;
}
int CmdlineLuaImpl::runCommand(const char * cmd) {

	return 0;
}

/*static*/ int CmdlineLuaImpl::test(int argc, char* argv[]) {
	return 0;
}


};



