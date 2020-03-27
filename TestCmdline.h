#ifndef __TEST_CMDLINE_H__
#define __TEST_CMDLINE_H__

#include <thread>
#include <mutex>
#include <chrono>             // std::chrono::seconds
#include <condition_variable>   // NOLINT

#include "cmdline/CmdLineFactory.h"

#include "pp/FootballPPTester.h"

namespace NS_test_miniled {
	class TestCmdMiniLed;
};

namespace NS_test_cmdline {

class FootballReceiver_Impl1;
class FootballService_Impl1;

class TestCmdline {
public:

#define TestPanelTool_TEST 1

#if TestPanelTool_TEST
		static int cmdline_test1(void *ctx, int argc, char *const*argv);
		static int cmdline_test2(void *ctx,int argc, char *const*argv);
		static int cmdline_test3(void *ctx,int argc, char *const*argv);
#endif

	CL_DEF_FUNC(date);
	CL_DEF_FUNC(rquit);

	CL_DEF_FUNC(fpp);
	CL_DEF_FUNC(re);
	CL_DEF_FUNC(set);
	CL_DEF_FUNC(n);
	CL_DEF_FUNC(p);
	CL_DEF_FUNC(c);
	CL_DEF_FUNC(vpp);
	CL_DEF_FUNC(bl);
	CL_DEF_FUNC(pr);

	CL_DEF_FUNC(pp);
	CL_DEF_FUNC(cct);

	enum {
		INIT_RUN_SERVICE = 0x01,
		INIT_FOR_CLIENT = 0x02,
		INIT_FOR_RECEIVER = 0x04,
	};
	TestCmdline(uint32_t flags);
	~TestCmdline();

	DECLARE_on_empty_cmd(on_empty_cmd_);
	DECLARE_on_intercept_command(on_intercept_command_);

	int isReceiverRegistered();
	
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

	FootballService_Impl1 *mFootballService_Impl1 = nullptr;
	FootballReceiver_Impl1 *mNativeFootballReceiver = nullptr;


	enum {
		COMMAND_EXIT = 0x01,
	};
	int setPendingCommandFlags(uint32_t flags);
	int checkPendingCommandFlags(uint32_t flags);
	int waitPendingCommandFlags(uint32_t flags, long timeout);
	uint32_t mPendingCommandFlags = 0;
	std::mutex PendingCommandFlags_mutex_;
	std::condition_variable PendingCommandFlags_cv_;


//
	int test_FootballPPVk_test2(int argc, char *const*argv);
	int test_FootballPPGles_test1(int argc, char *const*argv);
	int test_ANativeServiceFootballProxy(int argc, char *const*argv);
	int test_FootballService_Impl1(int argc, char *const*argv);
	int test_ndk_extend_1(int argc, char *const*argv);

	static int _main(int argc, char **argv);
};

};

#endif

