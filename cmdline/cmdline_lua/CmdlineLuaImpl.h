#ifndef __CMDLINE_LUAIMPL_H__
#define __CMDLINE_LUAIMPL_H__


#include <stdlib.h>

#include "CmdLine.h"

namespace android {

class CmdlineLuaImpl: public Cmdline {

public:

	CmdlineLuaImpl();
	virtual ~CmdlineLuaImpl() override;

		// prompt is set by CONFIG_SYS_PROMPT macro !!!
	virtual void setPrompt(const char *prompt) override ;
	virtual void add_on_empty_cmd(PF_on_empty_cmd cb, void *ctx) override;
	virtual void add_on_intercept_command(PF_on_intercept_command pf, void *ctx) override;
	virtual void on_empty_cmd_i() override;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) override ;
	virtual int loop() override ;
	virtual int check_command_matched(const char * cmd, const char *matched_) override;
	virtual int postCommand(const char * cmd) override;
	virtual int runCommand(const char * cmd) override;

	int internalAddon = 0;
	
	PF_on_empty_cmd mPF_on_empty_cmd = nullptr;
	void *mPF_on_empty_cmd_ctx = nullptr;


	static int test(int argc, char* argv[]);

};


};


#endif

