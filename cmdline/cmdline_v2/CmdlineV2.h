#ifndef __CMDLINE_V2_H__
#define __CMDLINE_V2_H__

#if 0
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#endif

#include <stdlib.h>

#include "CmdLine.h"

#define CMDLINE_TOOL_VERSION "v1.0.0"

namespace android {

class cli_impl;

class CmdlineV2: public Cmdline {

public:
	friend class cli_impl;

	CmdlineV2();
	virtual ~CmdlineV2() override;
		// prompt is set by CONFIG_SYS_PROMPT macro !!!
	virtual void setPrompt(const char *prompt) override ;
	virtual void addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) override;
	virtual void onEmptyCmd() override;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) override ;
	virtual int loop() override ;
	virtual int runCommand(const char * cmd) override;

	void *impl1 = nullptr;
	void *menu_impl = nullptr;
	int internalAddon = 0;

	PF_empty_cmd_cb mEmptyCmdCallback = nullptr;
	void *mEmptyCmdCallback_ctx = nullptr;

private:
	void addInternalCmd();
	int cli_cmd_process_(int flag, int argc, char * const argv[],
					   int *repeatable, uint64_t *ticks);
	void cli_cmd_empty_();

};


};


#endif

