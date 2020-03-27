#ifndef __CMDLINE_V2_H__
#define __CMDLINE_V2_H__

#include <stdlib.h>

#include <string>

#include "CmdLine.h"

#define CMDLINE_TOOL_VERSION "v1.0.0"

namespace android {

class cli_impl;

class CmdlineV2: public Cmdline {

public:
	friend class cli_impl;

	CmdlineV2(int opt_ = CLI_OPT_console_reader);
	virtual ~CmdlineV2() override;
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

	void *impl1 = nullptr;
	void *menu_impl = nullptr;
	int internalAddon = 0;

	PF_on_empty_cmd mPF_on_empty_cmd = nullptr;
	void *mPF_on_empty_cmd_ctx = nullptr;

	std::string mPrompt = "";

private:
	void addInternalCmd();
	int cli_cmd_process_(int flag, int argc, char * const argv[],
					   int *repeatable, uint64_t *ticks);
	void cli_cmd_empty_();

};


};


#endif

