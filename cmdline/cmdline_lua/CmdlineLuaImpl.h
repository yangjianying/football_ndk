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
	virtual void addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) override;
	virtual void onEmptyCmd() override;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) override ;
	virtual int loop() override ;
	virtual int runCommand(const char * cmd) override;

	int internalAddon = 0;

	PF_empty_cmd_cb mEmptyCmdCallback = nullptr;
	void *mEmptyCmdCallback_ctx = nullptr;


	static int test(int argc, char* argv[]);

};


};


#endif

