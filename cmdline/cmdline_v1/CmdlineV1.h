#ifndef __CMDLINE_V1_H__
#define __CMDLINE_V1_H__

#if 0
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#endif

#include "CmdLine.h"

#define CMDLINE_TOOL_VERSION "v1.0.0"
/*
v1.0.1
support multi-instances

v1.0.1
frankie, should refactor, currently only one instance can be created in one process !!!
*/

namespace android {

class CmdlineV1: public Cmdline {

public:
	CmdlineV1();
	virtual ~CmdlineV1() override ;
	virtual void setPrompt(const char *prompt) override ;
	virtual void addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) override;
	virtual void onEmptyCmd() override;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const*), void *ctx) override ;
	virtual int loop() override ;
	virtual int runCommand(const char * cmd) override;

	void *impl1 = nullptr;
	int internalAddon = 0;

private:
	void addInternalCmd();

	static int sCmdlineV1Num;
};

//
class CmdlineV1Combined
	//: public RefBase
{
public:
	CmdlineV1Combined();
	~CmdlineV1Combined();
	void setPrompt(const char *prompt);
	int add(CmdlineV1 *cl);
	int loop();
};

};


#endif

