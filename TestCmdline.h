#ifndef __TEST_CMDLINE_H__
#define __TEST_CMDLINE_H__

#include "CmdLineFactory.h"

#include "FootballPPTester.h"

namespace NS_test_miniled {
	class TestCmdMiniLed;
};

namespace NS_test_cmdline {

class TestCmdline {
public:

#define TestPanelTool_TEST 1

#if TestPanelTool_TEST
		static int cmdline_test1(void *ctx, int argc, char *const*argv);
		static int cmdline_test2(void *ctx,int argc, char *const*argv);
		static int cmdline_test3(void *ctx,int argc, char *const*argv);
#endif

	CL_DEF_FUNC(date);

	CL_DEF_FUNC(fp);
	CL_DEF_FUNC(re);
	CL_DEF_FUNC(set);
	CL_DEF_FUNC(n);
	CL_DEF_FUNC(p);
	CL_DEF_FUNC(c);
	CL_DEF_FUNC(vd);
	CL_DEF_FUNC(bl);
	CL_DEF_FUNC(pr);

	TestCmdline(uint32_t flags);
	~TestCmdline();

	DECLARE_EMPTY_CB(onEmptyCmd);
	
	int isValid() { return mValid; }
	void initCmd();
	int runCommand(const char * cmd);
	void loop();

	uint32_t mInitFlags = 0;
	int mValid = 0;
	::android::Cmdline * mCmdline = nullptr;

	NS_test_miniled::TestCmdMiniLed *mTestCmdMiniLed = nullptr;

	void checkFootballPPTester_created();
	football::FootballPPTester * mFootballPPTester = nullptr;

};

};

#endif

