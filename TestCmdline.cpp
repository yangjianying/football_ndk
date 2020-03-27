#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/resource.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()


#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>


#if 1	// drm
#include <errno.h>
#endif

#include "FootballConfig.h"

static const char* kTAG = "TestCmdline";
#include "android_logcat_.h"

#include "utils/foot_utils.h"
#include "utils/football_debugger.h"

#include "cmdline/CmdLineUtils.h"
#include "cmdline/cmdline_lua/CmdlineLuaImpl.h"

#include "miniled/TestCmdMiniLed.h"


#include "pp/FootballPPTester.h"

#include "ndk_extend/NativeHooApi_Loader.h"
#include "ndk_extend/NativeServiceFootball_Loader.h"
#include "ndk_extend/TestService_Loader.h"
#include "ndk_extend/NativeFootballReceiver_Loader.h"


#include "impl/FootballPPGles.h"
#include "impl/FootballPPVk.h"

#include "sys_api/FootballSysApi.h"

#include "TestCmdline.h"

#undef __CLASS__
#define __CLASS__ "TestCmdline"

#undef UNUSED_
#define UNUSED_(x) ((void)x)


using namespace football;

namespace NS_test_cmdline {

#if 1  // a test

class FootballReceiver_Impl1: public NativeFootballReceiver {
public:
	ANativeFootballReceiver *receiver_ = nullptr;
/* here , how to detect the server is still alive !!! 
 * */
	ANativeServiceFootballProxy *proxy_ = nullptr;
	NativeServiceFootball *p_ = nullptr;

	bool registered_ = false;
	FootballReceiver_Impl1() {
		void *ibinder_ = nullptr;  // here ibinder_ is just one opaque object !!!
		ANativeFootballReceiver_create(this, &receiver_);
		ANativeFootballReceiver_getBinder(receiver_, &ibinder_);
		DLOGD("%s,  receiver_ receiver_ = %p !\r\n", __func__, receiver_);
		if (receiver_ != nullptr) {
			if(ANativeServiceFootballProxy_create(football::FootballConfig::cycling_service_name, &proxy_) == 0) {
				if (ANativeServiceFootballProxy_isValid(proxy_)) {
					p_ = ANativeServiceFootballProxy_get(proxy_);
/* here, how to detect the registered receiver_ is still used by server side !!! */					
					uint32_t ret = p_->register_receiver(0, receiver_);
				
					DLOGD("%s,  register_receiver ret = %d !\r\n", __func__, ret);
					registered_ = true;
				}
				else {
					DLOGD("%s,  ANativeServiceFootball_Proxy_isValid failed !\r\n", __func__);
				}
			}
		}
	}
	~FootballReceiver_Impl1() { 
		if (registered_) {
			registered_ = false;
			p_->unregister_receiver(0);
		}
		if (receiver_ != nullptr) {
			receiver_ = nullptr;
			ANativeFootballReceiver_destroy(receiver_);
		}
		if (proxy_ != nullptr) {
			ANativeServiceFootballProxy_destroy(proxy_);
			proxy_ = nullptr; p_ = nullptr;
		}
	}
	virtual int32_t on_receiver(const std::vector<std::string>& input_, std::vector<std::string>& output_) {
		int32_t r = 0;
		for(std::vector<std::string>::const_iterator iter = input_.cbegin();iter != input_.cend();iter++) {
			//DLOGD("iter->c_str():%s \r\n", iter->c_str());
			//DLOGD("%s\r\n", iter->c_str());
			fprintf(stderr, "%s\r\n", iter->c_str());
		}
		//output_.push_back("hello,world!");
		//output_.push_back("hello,world!");
		//output_.push_back("hello,world!");
		return r;
	}
	
};

static void __DeathRecipientImpl1(void *ctx, ANativeFootballReceiverProxy *proxy_);
class FootballService_Impl1: public NativeServiceFootball {
public:
	TestCmdline *pTestCmdline = nullptr;
	ANativeServiceFootball *service_ = nullptr;
	bool added = false;
	long statistic_count = 0;

	//
	std::mutex mProxy_receiver_lock;
	ANativeFootballReceiverProxy *mProxy_receiver_ = nullptr;
	NativeFootballReceiver *mReceiver_ = nullptr;
	class RemoteLOG_cb_impl: public ::football::FootballDebugHandler::RemoteLOG_cb {
	public:
		FootballService_Impl1 *service_impl = nullptr;
		RemoteLOG_cb_impl(FootballService_Impl1 *service):service_impl(service) {}
		virtual void on_log(char *log_str) {
			service_impl->on_log(log_str);
		}
	};
	RemoteLOG_cb_impl *mRemoteLOG_cb_impl = nullptr;

	FootballService_Impl1(TestCmdline *cmdline_): pTestCmdline(cmdline_) {
		DLOGD("%s,%d ... \r\n", __func__, __LINE__);
		if (ANativeServiceFootball_create(this, &service_) == 0) {
			if(ANativeServiceFootball_addService(service_, football::FootballConfig::cycling_service_name) >= 0) {
				added = true;
				DLOGD("%s, add service success !!! ", __func__);
			} else {
				DLOGD("%s, add service failed !!! ", __func__);
			}
		}
		mRemoteLOG_cb_impl = new RemoteLOG_cb_impl(this);
		DLOGD("%s,%d done \r\n", __func__, __LINE__);
	}
	virtual ~ FootballService_Impl1() {
		DLOGD("%s,%d ...\r\n", __func__, __LINE__);

		//
		::football::FootballDebugHandler::Get_FootballDebugHandler()->set_RemoteLOG_cb(nullptr);
		
		if (added) {
			added = false;
			ANativeServiceFootball_removeService(service_);
		}
		ANativeServiceFootball_destroy(service_);
		DLOGD("%s,%d done \r\n", __func__, __LINE__);
	}
	virtual void dummy() {
		statistic_count++;
		DLOGD( "FootballService_Impl1::%s  statistic_count:%ld \r\n", __func__, statistic_count);
		return ;
	}
	virtual int dummy2() {
		DLOGD( "FootballService_Impl1::%s \r\n", __func__);
		return 0;
	}
	virtual int dummy3(int a, int b) {
		DLOGD( "FootballService_Impl1::%s \r\n", __func__);
		return 0;
	}
	virtual int32_t cmdline_transact(const std::vector<std::string>& input_, std::vector<std::string>& output_) {
		int32_t r = 0;
		for(std::vector<std::string>::const_iterator iter = input_.cbegin();iter != input_.cend();iter++) {
			if (pTestCmdline != nullptr && pTestCmdline->mCmdline != nullptr) {
				pTestCmdline->mCmdline->postCommand(iter->c_str());
			}
		}
		output_.push_back("hello,world!");
		return r;
	}

/** redirect the log to here !!! */

	void on_log(char *log_str) {
		std::unique_lock<std::mutex> worker_lock(mProxy_receiver_lock);
		if (mReceiver_ != nullptr) {
			std::vector<std::string> input_;
			std::vector<std::string> output_;
			input_.push_back(std::string(log_str));
			int32_t r = mReceiver_->on_receiver(input_, output_);
		}
	}

	virtual int32_t register_receiver(int type_, void * binder_) {
		
		// only i known binder_ is a binder proxy object of NativeFootballReceiver interface .
		// when not using it, should call ANativeFootballReceiver_Proxy_destroy to release the proxy object !!!
		DLOGD( "%s,%d, type_:%d \r\n", __func__, __LINE__, type_);

		{
			std::unique_lock<std::mutex> worker_lock(mProxy_receiver_lock);
			if (mProxy_receiver_ != nullptr) {
				ANativeFootballReceiverProxy_destroy(mProxy_receiver_);
				mProxy_receiver_ = nullptr;
				mReceiver_ = nullptr;
			}

			mProxy_receiver_ = static_cast<ANativeFootballReceiverProxy *>(binder_);
			ANativeFootballReceiverProxy_setDeathRecipient(mProxy_receiver_, __DeathRecipientImpl1, this);

			mReceiver_ = ANativeFootballReceiverProxy_get(mProxy_receiver_);

		}
		{
			std::unique_lock<std::mutex> worker_lock(mProxy_receiver_lock);

			std::vector<std::string> input_;
			std::vector<std::string> output_;
			input_.push_back(std::string("That brown fox quickly jumps over the lazy dog !!!"));
			input_.push_back(std::string("That brown fox quickly jumps over the lazy dog !!!"));
			input_.push_back(std::string("That brown fox quickly jumps over the lazy dog !!!"));

			int32_t r = mReceiver_->on_receiver(input_, output_);

			#if 0  // here must not using DLOGD, will result dead lock !!!
			DLOGD( "r=%d output_.size=%d \r\n", r, (int)output_.size());
			for(std::vector<std::string>::const_iterator iter = output_.cbegin();iter != output_.cend();iter++) {
				DLOGD( "iter->c_str():%s \r\n", iter->c_str());
			}
			#endif
			printf( "r=%d output_.size=%d \r\n", r, (int)output_.size());
			for(std::vector<std::string>::const_iterator iter = output_.cbegin();iter != output_.cend();iter++) {
				printf( "iter->c_str():%s \r\n", iter->c_str());
			}

		}
		::football::FootballDebugHandler::Get_FootballDebugHandler()->set_RemoteLOG_cb(mRemoteLOG_cb_impl);
		printf("%s,%d \r\n", __func__, __LINE__);

		return 0;
	}

	virtual int32_t unregister_receiver(int type_) {
		{
			std::unique_lock<std::mutex> worker_lock(mProxy_receiver_lock);
			#if 0  // here must not using DLOGD, will result dead lock !!!
			DLOGD( "%s, type_:%d \r\n", __func__, type_);
			#endif
			printf( "%s, type_:%d \r\n", __func__, type_);
			if (mProxy_receiver_ != nullptr) {
				ANativeFootballReceiverProxy_destroy(mProxy_receiver_);
				mProxy_receiver_ = nullptr;
				mReceiver_ = nullptr;
			}
		}
		::football::FootballDebugHandler::Get_FootballDebugHandler()->set_RemoteLOG_cb(nullptr);
		return 0;
	}
	void DeathRecipient__(ANativeFootballReceiverProxy *proxy_) {
		{
			std::unique_lock<std::mutex> worker_lock(mProxy_receiver_lock);
			#if 0  // here must not using DLOGD, will result dead lock !!!
			DLOGD( "%s,%d proxy_: %p mProxy_receiver_:%p \r\n", __func__, __LINE__, proxy_, mProxy_receiver_);
			#endif
			printf( "%s,%d proxy_: %p mProxy_receiver_:%p \r\n", __func__, __LINE__, proxy_, mProxy_receiver_);
			if (mProxy_receiver_ != nullptr) {
				ANativeFootballReceiverProxy_destroy(mProxy_receiver_);
				mProxy_receiver_ = nullptr;
				mReceiver_ = nullptr;
			}
		}
		::football::FootballDebugHandler::Get_FootballDebugHandler()->set_RemoteLOG_cb(nullptr);
	}
};

static void __DeathRecipientImpl1(void *ctx, ANativeFootballReceiverProxy *proxy_) {
	FootballService_Impl1 *impl = static_cast<FootballService_Impl1 *>(ctx);
	impl->DeathRecipient__(proxy_);
}

#endif

IMPL_on_empty_cmd(TestCmdline, TestCmdline, on_empty_cmd_);
IMPL_on_intercept_command(TestCmdline, TestCmdline, on_intercept_command_);

TestCmdline::TestCmdline(uint32_t flags):
	mInitFlags(flags)
	, mCmdline(::android::cmdline::factory::makeCmdline(
		((INIT_RUN_SERVICE & mInitFlags) || (INIT_FOR_CLIENT & mInitFlags)) ? ::android::CMDLINE_V3 : 
				::android::CMDLINE_V2, 
		INIT_RUN_SERVICE & mInitFlags ? ::android::CLI_OPT_console_none 
			: ::android::CLI_OPT_console_reader))
	{
	DLOGD( "%s, mInitFlags:%08x \r\n", __func__, mInitFlags);
	
	NativeHooApi_INITIALIZE();
	NativeFootballReceiver_INITIALIZE();
	NativeServiceFootball_INITIALIZE();
	TestService_INITIALIZE();
	
	ANativeProcessState_startThreadPool();

	if ((INIT_RUN_SERVICE & mInitFlags) == 0) {
		ADD_on_intercept_command(mCmdline, TestCmdline, on_intercept_command_, this);
	}
	ADD_on_empty_cmd(mCmdline, TestCmdline, on_empty_cmd_, this);

	if ((INIT_FOR_CLIENT & mInitFlags) == 0) {
		uint32_t TestCmdMiniLed_flags = 0x00;
		mTestCmdMiniLed = new NS_test_miniled::TestCmdMiniLed(TestCmdMiniLed_flags);
	}

	mValid = 1;

	//mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);

	if (INIT_RUN_SERVICE & mInitFlags) {
		::football::FootballDebugHandler::Get_FootballDebugHandler()->setOutputFlags(
					::football::FootballDebugHandler::OUT_REMOTE, ::football::FootballDebugHandler::OUT_REMOTE);

		mFootballService_Impl1 = new FootballService_Impl1(this);
	}
	if (INIT_FOR_RECEIVER & mInitFlags) {
		mNativeFootballReceiver = new FootballReceiver_Impl1();
	}

}
TestCmdline::~TestCmdline() {
	if (mNativeFootballReceiver != nullptr) {
		delete mNativeFootballReceiver;
		mNativeFootballReceiver = nullptr;
	}
	if (mFootballService_Impl1 != nullptr) {
		delete mFootballService_Impl1;
		mFootballService_Impl1 = nullptr;
	}

	if (mFootballPPTester != nullptr) {
		delete mFootballPPTester;
	}
	if (mTestCmdMiniLed != nullptr) {
		delete mTestCmdMiniLed;
	}
	delete mCmdline;
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
void TestCmdline::on_empty_cmd_() {
	//DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
int TestCmdline::on_intercept_command_(const char *cmd, int flag) {
	//DLOGD( "%s,%d \r\n", __func__, __LINE__);
	{
		if (mCmdline->check_command_matched(cmd, ::android::Cmdline::kCmd_quit)
			|| mCmdline->check_command_matched(cmd, ::android::Cmdline::kCmd_q_)) {
			mCmdline->postCommand(cmd);
			return 0;
		}
		else if (mCmdline->check_command_matched(cmd, ::android::Cmdline::kCmd_help)
			|| mCmdline->check_command_matched(cmd, ::android::Cmdline::kCmd_h_)) {
			mCmdline->postCommand(cmd);
			return 0;
		}

	}
	{
		if (INIT_FOR_RECEIVER & mInitFlags) {
			DLOGD("log window, do nothing !!! \r\n");
			return 0;
		}
	}
	static int i = 0;
	i++;
	ANativeServiceFootballProxy *proxy_ = nullptr;

	if(ANativeServiceFootballProxy_create(football::FootballConfig::cycling_service_name, &proxy_) == 0) {
		if (ANativeServiceFootballProxy_isValid(proxy_)) {
			NativeServiceFootball *p_ = ANativeServiceFootballProxy_get(proxy_);
			std::vector<std::string> input_;
			std::vector<std::string> output_;
			input_.push_back(std::string(cmd));

			int32_t r = p_->cmdline_transact(input_, output_);

			DLOGD( "r=%d output_.size=%d output_[0]:%s \r\n",
				r, (int)output_.size(), output_.size() > 0 ? output_[0].c_str() : "null");
		}
		else {
			DLOGD( "%s,  ANativeServiceFootball_Proxy_isValid failed ! i = %d \r\n", __func__, i);
		}
		ANativeServiceFootballProxy_destroy(proxy_);
	}
	else {
		DLOGD( "%s,  ANativeServiceFootball_Proxy_create failed ! i = %d \r\n", __func__, i);
	}
	return 0;
}
int TestCmdline::isReceiverRegistered() {
	if (mNativeFootballReceiver != nullptr) {
		return mNativeFootballReceiver->registered_ ? 1 : 0;
	}
	return -1;
}

void TestCmdline::checkFootballPPTester_created() {
	if (mFootballPPTester != nullptr && mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_FILE) {
		delete mFootballPPTester;
		mFootballPPTester = nullptr;
	}

	if (mFootballPPTester == nullptr) {
		DLOGD( "%s! \r\n", __func__);
		mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);
	}
}

void TestCmdline::initCmd() {
	// init command line
	//DLOGD( "TestCmdline::%s \r\n", __func__);

	mCmdline->setPrompt("football>>");

#if TestPanelTool_TEST
	mCmdline->add("test1", "test1", cmdline_test1, this);
	mCmdline->add("test2", "test2", cmdline_test2, this);
	mCmdline->add("test3", "test3", cmdline_test3, this);
#endif

	CL_ADD_FUNC(mCmdline, date);
	if (INIT_RUN_SERVICE & mInitFlags) {
		CL_ADD_FUNC(mCmdline, rquit);
		CL_ADD_FUNC_f(mCmdline, rq, this, CL_SFUNC_NS(NS_test_cmdline::TestCmdline, rquit));
	}

	if ((INIT_FOR_RECEIVER & mInitFlags) == 0) {
		CL_ADD_FUNC(mCmdline, fpp);
		CL_ADD_FUNC(mCmdline, re);
		CL_ADD_FUNC(mCmdline, set);
		CL_ADD_FUNC(mCmdline, n);
		CL_ADD_FUNC(mCmdline, p);
		CL_ADD_FUNC(mCmdline, c);
		CL_ADD_FUNC(mCmdline, vpp);
		//CL_ADD_FUNC(mCmdline, bl);
		CL_ADD_FUNC(mCmdline, pr);

		CL_ADD_FUNC(mCmdline, pp);
		CL_ADD_FUNC(mCmdline, cct);
	}

	if (mTestCmdMiniLed != nullptr) {
		CL_ADD_FUNC_f(mCmdline, init, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, init));
		CL_ADD_FUNC_f(mCmdline, w, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, w));
		CL_ADD_FUNC_f(mCmdline, r, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, r));
		CL_ADD_FUNC_f(mCmdline, bl, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, bl));
		CL_ADD_FUNC_f(mCmdline, vsync, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, vsync));
	}

	// date(0, nullptr);
}
int TestCmdline::runCommand(const char * cmd) {
	return mCmdline->runCommand(cmd);
}
void TestCmdline::loop() {

#if 0
	// run init sequence
	runCommand("vd 10 50");

#endif

	// then enter cmdline parser !!!
	mCmdline->loop();
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, date);
int TestCmdline::date(int argc, char *const *argv) { UNUSED_(argc); UNUSED_(argv);
	DLOGD( "build at %s %s \r\n", __DATE__, __TIME__);
	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, rquit);
int TestCmdline::rquit(int argc, char *const *argv) { UNUSED_(argc); UNUSED_(argv);
	DLOGD( "rquit ... \r\n");
	DLOGD( "build at %s %s \r\n", __DATE__, __TIME__);
	return -10001;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, fpp);
int TestCmdline::fpp(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	int num_of_test = -1;
	int pp_type_ = FootballPPFactory::PP_CPU;  // default cpu
	int have_pp_type = 0;
	int have_delete = 0;
	
	optarg = NULL; optind = 0; opterr = 0; optopt = 0;
	static struct option long_options[] = {{0,  0,  0,  0 }};
	while (true) {
		int optionIndex = 0;
		int ic = getopt_long(argc, argv, "hp:c:d", long_options, &optionIndex);
		if (ic == -1) {
			break;
		}
		switch (ic) {
		case 'h': {
			DLOGD("no option /toggle the PP tester \r\n");
			DLOGD("-p 0/1/2 /specify the PP type \r\n");
			DLOGD("-c times /specify cycle test times \r\n");
			DLOGD("-d /delete the pp tester anyway \r\n");
			return 0;
		}
		case 'p': {
			if (cycling::_c__atoi(optarg, &pp_type_) < 0) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			if (pp_type_ < 0 || pp_type_ > 2) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			have_pp_type = 1;
			break;
		}
		case 'c': {
			if (cycling::_c__atoi(optarg, &num_of_test) < 0) {
				DLOGD( "num_of_file invalid! \r\n");
				return 0;
			}
			if (num_of_test < 0 || num_of_test > 50000) {
				DLOGD( "num_of_file invalid! \r\n");
				return 0;
			}
			break;
		}
		case 'd': {
			have_delete = 1;
			}
			break;
		default: {
			DLOGD( "input invalid! \r\n");
			return 0;
		}
		}
	}
	if (have_pp_type) {
		// have pp_type, but not have num_of_test, only re-init the pptester with specified pp type !!!
		if (num_of_test < 0) {
			num_of_test = 1;
		}
	}
	DLOGD( "pp_type_:%s \r\n", football::FootballPPFactory::getPPTypeDesc(pp_type_).c_str());
	DLOGD( "num_of_test:%d \r\n", num_of_test);
	
	if (num_of_test < 0) {  // default toggle tester !!!
		if (mFootballPPTester != nullptr) {
			DLOGD( "*** cleanup tester! \r\n");
			delete mFootballPPTester;
			mFootballPPTester = nullptr;
		} else {
			DLOGD( "*** create tester! \r\n");
			mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, pp_type_, 0, football::FootballPPTester::SOURCE_FILE);
		}
	}
	else if (num_of_test > 0) {  // other value , do delete/create cycle with specific times !!!
		for(int i=0;i<num_of_test;i++) {
			DLOGD( "######################################### num_of_test = %d \r\n", i);
			if (mFootballPPTester != nullptr) {
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
			mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, pp_type_, 0, football::FootballPPTester::SOURCE_FILE);
			if (have_delete) {
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
		}
	}
	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, re);
int TestCmdline::re(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();
	if (mFootballPPTester != nullptr) {
		mFootballPPTester->directoryRescan();
	}
	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, set);
int TestCmdline::set(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();
	if (argc == 1) {
		if (mFootballPPTester != nullptr) {
			DLOGD( "%s \r\n", mFootballPPTester->directoryGet());
		}
	} else if (argc == 2) {
		std::string dir = argv[1];
		if (mFootballPPTester != nullptr) {
			mFootballPPTester->directorySet(dir.c_str());
		}
	}

	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, n);
int TestCmdline::n(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();

	int num_of_file = 1;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &num_of_file) < 0) {
			DLOGD( "num_of_file invalid! \r\n");
			return 0;
		}
		if (num_of_file <= 0 || num_of_file > 50000) {
			DLOGD( "num_of_file invalid! \r\n");
			return 0;
		}
	} else if(argc > 2) {
		DLOGD( "input invalid! \r\n");
		return 0;
	}

	if (mFootballPPTester != nullptr) {
		mFootballPPTester->process_next_file(num_of_file);
	}

	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, p);
int TestCmdline::p(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();
	
	int num_of_file = 1;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &num_of_file) < 0) {
			DLOGD( "num_of_file invalid! \r\n");
			return 0;
		}
		if (num_of_file <= 0 || num_of_file > 50000) {
			DLOGD( "num_of_file invalid! \r\n");
			return 0;
		}
	} else if(argc > 2) {
		DLOGD( "input invalid! \r\n");
		return 0;
	}

	if (mFootballPPTester != nullptr) {
		mFootballPPTester->process_prev_file(num_of_file);
	}

	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, c);
int TestCmdline::c(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();

	int num_of_times = 1;
	int have_algo = 1;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &have_algo) < 0) {
			DLOGD( "have_algo invalid! \r\n");
			return 0;
		}
		if (have_algo < 0 || have_algo > 50000) {
			DLOGD( "have_algo invalid! \r\n");
			return 0;
		}
	} else if(argc == 3) {
		if (cycling::_c__atoi(argv[1], &num_of_times) < 0) {
			DLOGD( "num_of_times invalid! \r\n");
			return 0;
		}
		if (num_of_times <= 0 || num_of_times > 90000) {
			DLOGD( "num_of_times invalid! \r\n");
			return 0;
		}
		//
		if (cycling::_c__atoi(argv[2], &have_algo) < 0) {
			DLOGD( "have_algo invalid! \r\n");
			return 0;
		}
		if (have_algo < 0 || have_algo > 50000) {
			DLOGD( "have_algo invalid! \r\n");
			return 0;
		}
	} else if(argc > 3) {
		DLOGD( "input invalid! \r\n");
		return 0;
	}

	if (mFootballPPTester != nullptr) {
		mFootballPPTester->process_current_file(num_of_times, have_algo);
	}

	return 0;
}

CL_SFUNC_IMPL(TestCmdline, TestCmdline, vpp);
int TestCmdline::vpp(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);

	int num_of_times = -1;
	
	int pp_type_ = FootballPPFactory::PP_VK;  // default vk
	int have_pp_type = 0;

	long render_wait_ms_times = 100;
	int have_wait_time = 0;

	int have_delete = 0;
	
	optarg = NULL; optind = 0; opterr = 0; optopt = 0;
	static struct option long_options[] = {{0,  0,  0,  0 }};
	while (true) {
		int optionIndex = 0;
		int ic = getopt_long(argc, argv, "hp:c:t:d", long_options, &optionIndex);
		if (ic == -1) {
			break;
		}
		switch (ic) {
		case 'h': {
			DLOGD("no option /toggle the PP tester \r\n");
			DLOGD("-p 0/1/2 /specify the PP type \r\n");
			DLOGD("-c times /specify cycle test times \r\n");
			DLOGD("-t wait_time /specify wait time \r\n");
			DLOGD("-d /delete the pp tester anyway \r\n");
			return 0;
		}
		case 'p': {
			if (cycling::_c__atoi(optarg, &pp_type_) < 0) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			if (pp_type_ < 0 || pp_type_ > 2) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			have_pp_type = 1;
			break;
		}
		case 'c': {
			if (cycling::_c__atoi(optarg, &num_of_times) < 0) {
				DLOGD( "num_of_file invalid! \r\n");
				return 0;
			}
			if (num_of_times <= 0 || num_of_times > 10000000) {
				DLOGD( "num_of_file invalid! \r\n");
				return 0;
			}
			break;
		}
		case 't': {
			if (cycling::_c__atol(optarg, &render_wait_ms_times) < 0) {
				DLOGD( "render_wait_ms_times invalid! \r\n");
				return 0;
			}
			if (render_wait_ms_times < 0 || render_wait_ms_times > 100000*1000) {
				DLOGD( "render_wait_ms_times invalid! \r\n");
				return 0;
			}
			have_wait_time = 1;
			break;
		}
		case 'd': {
			have_delete = 1;
			break;
		}

		default: {
			DLOGD( "input invalid! \r\n");
			return 0;
		}
		}
	}
	if (have_pp_type) {
		// have pp_type, but not have num_of_test, only re-init the pptester with specified pp type !!!s
		if (num_of_times < 0) {
			num_of_times = 1;
		}
	}
	DLOGD( "pp_type_:%s \r\n", football::FootballPPFactory::getPPTypeDesc(pp_type_).c_str());
	DLOGD( "num_of_times:%d \r\n", num_of_times);
	DLOGD( "render_wait_ms_times: %ld \r\n", render_wait_ms_times);

	if (num_of_times > 0) {
		for(int i=0;i<num_of_times;i++) {
			DLOGD( "######################################### num_of_time = %d \r\n", i);
			if (mFootballPPTester != nullptr 
				//&& mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_DISPLAY
				) {
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
			if (mFootballPPTester == nullptr) {
				mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, pp_type_, 0, FootballPPTester::SOURCE_DISPLAY);
			}

			if (have_wait_time) {
				if (mFootballPPTester != nullptr) {
					mFootballPPTester->virtual_display_source_setup(football::FootballPPTester::VD_OP_OPEN);
					if (render_wait_ms_times > 0) {
						usleep(render_wait_ms_times*1000);
					}
					else if (render_wait_ms_times == 0) {
						DLOGD( "******************************************************** \r\n");
						DLOGD( "******************* wait forever !!!");
						DLOGD( "******************************************************** \r\n");
						while(checkPendingCommandFlags(COMMAND_EXIT) == 0) { usleep(1000); }
					}
					mFootballPPTester->virtual_display_source_setup(football::FootballPPTester::VD_OP_CLOSE);

					if (have_delete) {
						delete mFootballPPTester;
						mFootballPPTester = nullptr;
					}
				}
			}
		}
		return 0;
	}

	if (have_delete) {
		if (mFootballPPTester != nullptr 
			//&& mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_DISPLAY
			) {
			delete mFootballPPTester;
			mFootballPPTester = nullptr;
		}
		return 0;
	}

	// only toggle current pp tester 's source',
	// if current source is not display, create a new one with default VK pp type !!
	if (mFootballPPTester != nullptr && mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_DISPLAY) {
		delete mFootballPPTester;
		mFootballPPTester = nullptr;
	}
	if (mFootballPPTester == nullptr) {
		mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, pp_type_, 0, FootballPPTester::SOURCE_DISPLAY);
		mFootballPPTester->virtual_display_source_setup(football::FootballPPTester::VD_OP_TOGGLE);
	}

	return 0;
}


CL_SFUNC_IMPL(TestCmdline, TestCmdline, bl);
int TestCmdline::bl(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	return 0;
}

CL_SFUNC_IMPL(TestCmdline, TestCmdline, pr);
int TestCmdline::pr(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	checkFootballPPTester_created();

	if (mFootballPPTester != nullptr) {
		mFootballPPTester->print();
	}

	return 0;
}

/*
pp -p 0/1/2 -u 0/1/2/3 -s 0/1
*/
CL_SFUNC_IMPL(TestCmdline, TestCmdline, pp);
int TestCmdline::pp(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);

	int pp_type_ = -1;
	int session_type_ = -1;
	int source_type_ = -1;
	int sink_type_ = FootballPPTester::SINK_SURFACE;
	int have_delete = 0;
	
	optarg = NULL; optind = 0; opterr = 0; optopt = 0;
	static struct option long_options[] = {{0,  0,  0,  0 }};
	while (true) {
		int optionIndex = 0;
		int ic = getopt_long(argc, argv, "hp:u:s:k:dw:f:m:t:", long_options, &optionIndex);
		if (ic == -1) {
			break;
		}
		switch (ic) {
		case 'h': {
			DLOGD("-h print this message \r\n");
			DLOGD("-p 0/1/2 /specify the PP type \r\n");
			DLOGD("    0 cpu\r\n");
			DLOGD("    1 opengles\r\n");
			DLOGD("    2 vulkan\r\n");
			DLOGD("-u 0~ /specify the session type \r\n");
			DLOGD("    0~ vary for specific PP type\r\n");
			DLOGD("-s 0/1 /specify the pp source \r\n");
			DLOGD("    0 file source\r\n");
			DLOGD("    1 virtual display source\r\n");
			DLOGD("-k 0/1 /specify the pp sink \r\n");
			DLOGD("    0 to surface\r\n");
			DLOGD("    1 to image reader\r\n");
			DLOGD("-w 0/1 /open or close the debug window \r\n");
			DLOGD("-f f0 f1 /set BHE factor0, factor1 \r\n");
			DLOGD("-m 0/1 /switch between full or half screen \r\n");
			DLOGD("-t f0 f1 /set BHE tuning0, tuning1 \r\n");
			DLOGD("-d delete tester \r\n");
			return 0;
		}
		case 'p': {
			if (cycling::_c__atoi(optarg, &pp_type_) < 0) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			if (pp_type_ < 0 || pp_type_ > 2) {
				DLOGD( "pp_type_ invalid! \r\n");
				return 0;
			}
			break;
		}
		case 'u': {
			if (cycling::_c__atoi(optarg, &session_type_) < 0) {
				DLOGD( "session_type_ invalid! \r\n");
				return 0;
			}
			if (session_type_ < 0) {
				DLOGD( "session_type_ invalid! \r\n");
				return 0;
			}
			break;
		}
		case 's': {
			if (cycling::_c__atoi(optarg, &source_type_) < 0) {
				DLOGD( "source_type_ invalid! \r\n");
				return 0;
			}
			if (source_type_ < 0 || source_type_ > 1) {
				DLOGD( "source_type_ invalid! \r\n");
				return 0;
			}
			break;
		}
		case 'k': {
			if (cycling::_c__atoi(optarg, &sink_type_) < 0) {
				DLOGD( "sink_type_ invalid! \r\n");
				return 0;
			}
			if (sink_type_ < 0 || sink_type_ > 1) {
				DLOGD( "sink_type_ invalid! \r\n");
				return 0;
			}
			break;
		}
		case 'd': {
			have_delete = 1;
			break;
		}
		case 'w' : {
			int window_enable = 0;
			if (cycling::_c__atoi(optarg, &window_enable) < 0) {
				DLOGD( "window_enable invalid! \r\n");
				return 0;
			}
			DLOGD( "window_enable = %d \r\n", window_enable);
			
			if (mFootballPPTester != nullptr ) {
				SessionParameter parameter;
				parameter.trigger_request = 1;
				parameter.request_type = 1;
				parameter.trigger_data.type = 1; 
				parameter.trigger_data.param_int0 = window_enable;
				mFootballPPTester->setParameter(&parameter);
			}
			else {
				DLOGD( "NO tester created !!! \r\n");
			}
			return 0;
			break;
		}
		case 'f' : {
			float f0 = 0.0f;
			float f1 = 0.0f;

			if (optind >= argc) {
				DLOGD( "less input argument ! \r\n");
				return 0;
			}
			char *f0_str = optarg;
			char *f1_str = argv[optind++];
			f0 = atof(f0_str);
			f1 = atof(f1_str);
			if (f0 < 0.05 || f0 > 0.45) {
				DLOGD( "input f0:%1.6f invalid! \r\n", f0);
				return 0;
			}
			if (f1 < 0.6 || f1 > 0.96) {
				DLOGD( "input f1:%1.6f invalid! \r\n", f1);
				return 0;
			}
			DLOGD( "input f0:%1.6f f1:%1.6f \r\n", f0, f1);
	
			if (mFootballPPTester != nullptr) {
				SessionParameter parameter;
				parameter.trigger_request = 1;
				parameter.request_type = 1;
				parameter.trigger_data.type = 2; 
				parameter.trigger_data.factor0 = f0;
				parameter.trigger_data.factor1 = f1;
				mFootballPPTester->setParameter(&parameter);
			}
			else {
				DLOGD( "NO tester created !!! \r\n");
			}
			return 0;
			break;
		}
		case 'm' : {
			int screen_mode_ = 0;
			if (cycling::_c__atoi(optarg, &screen_mode_) < 0) {
				DLOGD( "screen_mode_ invalid! \r\n");
				return 0;
			}
			DLOGD( "screen_mode_ = %d \r\n", screen_mode_);
			
			if (mFootballPPTester != nullptr ) {
				SessionParameter parameter;
				parameter.trigger_request = 1;
				parameter.request_type = 1;
				parameter.trigger_data.type = 3; 
				parameter.trigger_data.param_int0 = screen_mode_;
				mFootballPPTester->setParameter(&parameter);

				// then should invalidate the whole screen !!!
				{
					football::DisplayWindow_ *dispWindow_ = new football::DisplayWindow_(
						"#dummy", 1, 1, AIMAGE_FORMAT_RGBA_8888);
					dispWindow_->setPos(0, 0);
					dispWindow_->fillColor(0);
					dispWindow_->show();
					usleep(33*1000);
					delete dispWindow_;
				}
			}
			else {
				DLOGD( "NO tester created !!! \r\n");
			}
			return 0;
			break;
		}
		case 't' : {
			float f0 = 0.0f;
			float f1 = 0.0f;

			if (optind >= argc) {
				DLOGD( "less input argument ! \r\n");
				return 0;
			}
			char *f0_str = optarg;
			char *f1_str = argv[optind++];
			f0 = atof(f0_str);
			f1 = atof(f1_str);
			if (f0 < 0.05 || f0 > 1.9) {
				DLOGD( "input f0:%1.6f invalid! \r\n", f0);
				return 0;
			}
			if (f1 < 0.05 || f1 > 4.0) {
				DLOGD( "input f1:%1.6f invalid! \r\n", f1);
				return 0;
			}
			DLOGD( "input f0:%1.6f f1:%1.6f \r\n", f0, f1);
	
			if (mFootballPPTester != nullptr) {
				SessionParameter parameter;
				parameter.trigger_request = 1;
				parameter.request_type = 1;
				parameter.trigger_data.type = 4; 
				parameter.trigger_data.factor0 = f0;
				parameter.trigger_data.factor1 = f1;
				mFootballPPTester->setParameter(&parameter);
			}
			else {
				DLOGD( "NO tester created !!! \r\n");
			}
			return 0;
			break;
		}

		default: {
			DLOGD( "default input invalid! \r\n");
			return 0;
		}
		}
	}
	int can_create_tester = !( pp_type_ < 0 || session_type_< 0 || source_type_ < 0);
	if (have_delete == 0 && ! can_create_tester) {
		DLOGD( "must specify all pp_type, session_type, source_type \r\n");
		return 0;
	}
	if (have_delete) {
		if (mFootballPPTester != nullptr ) {
			delete mFootballPPTester;
			mFootballPPTester = nullptr;
		}
	}
	if (can_create_tester) {
		if (mFootballPPTester == nullptr) {
			mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, pp_type_, session_type_, source_type_, sink_type_);
			if (source_type_ == FootballPPTester::SOURCE_DISPLAY) {
				mFootballPPTester->virtual_display_source_setup(football::FootballPPTester::VD_OP_OPEN);
			}
		}
		else {
			DLOGD( "tester is already created, you should use -d to delete it first ! \r\n");
		}
	}
	return 0;
}

CL_SFUNC_IMPL(TestCmdline, TestCmdline, cct);
int TestCmdline::cct(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	int cct_flags = 1;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &cct_flags) < 0) {
			DLOGD( "cct_flags invalid! \r\n");
			return 0;
		}
		if (cct_flags < 0 || cct_flags > 100) {
			DLOGD( "cct_flags invalid! \r\n");
			return 0;
		}
	}
	else {
		return 0;
	}
	int ret = ANativeHoo_CCT_autocontrol(cct_flags);
	DLOGD("ANativeHoo_CCT_autocontrol ret = %d \r\n", ret);


	return 0;
}

#if TestPanelTool_TEST

/*static*/ int TestCmdline::cmdline_test1(void *ctx, int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);
	TestCmdline *pTestCmdline = (TestCmdline *)ctx;

	return 0;
}


/*static*/ int TestCmdline::cmdline_test2(void *ctx,int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);
	TestCmdline *pTestCmdline = (TestCmdline *)ctx;

	return 0;
}
/*static*/ int TestCmdline::cmdline_test3(void *ctx,int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);
	TestCmdline *pTestCmdline = (TestCmdline *)ctx;

	return 0;
}
#endif

int TestCmdline::setPendingCommandFlags(uint32_t flags) {
	std::unique_lock<std::mutex> caller_lock(PendingCommandFlags_mutex_);
	mPendingCommandFlags = flags;
	PendingCommandFlags_cv_.notify_one();
	return 0;
}
int TestCmdline::checkPendingCommandFlags(uint32_t flags) {
	std::unique_lock<std::mutex> caller_lock(PendingCommandFlags_mutex_);
	if (mPendingCommandFlags & flags) {
		return 1;
	}
	return 0;
}
int TestCmdline::waitPendingCommandFlags(uint32_t flags, long timeout) {
	std::unique_lock<std::mutex> caller_lock(PendingCommandFlags_mutex_);
	if(mPendingCommandFlags != flags) {
		if (PendingCommandFlags_cv_.wait_for(caller_lock, std::chrono::milliseconds(timeout))
			== std::cv_status::timeout) {
			if(mPendingCommandFlags != flags) {
				return -1;
			}
			return 0;
		}
		if(mPendingCommandFlags != flags) {
			return -1;
		}
		return 0;
	}
	return 0;
}

#define USE_QUICK_TEST_ 1
#if USE_QUICK_TEST_
#define OPT_QUICK_TEST "t:f:"

void QUICK_TEST_RUN_COMMAND(TestCmdline *_test, int quick_test_index) {
	if (quick_test_index == 0) {
		_test->runCommand("pp -p 2 -u 0 -s 1 -d");
	}
	else if (quick_test_index == 1) {
		_test->runCommand("pp -p 2 -u 1 -s 1 -d");
	}
	else if (quick_test_index == 11) {
		_test->runCommand("pp -p 2 -u 11 -s 1 -d");
	}
	else if (quick_test_index == 12) {
		_test->runCommand("pp -p 2 -u 12 -s 1 -d");
	}
	else if (quick_test_index == 13) {
		_test->runCommand("pp -p 2 -u 13 -s 1 -d");
	}
	else if (quick_test_index == 14) {
		_test->runCommand("pp -p 2 -u 14 -s 1 -d");
	}
	else {
		DLOGD("********* WARNING : quick test: %d not defined !!! ************** \r\n", quick_test_index);
	}
}
#else
#define OPT_QUICK_TEST ""
#endif

/*static*/ int TestCmdline::_main(int argc, char **argv) {
	UNUSED_(argc); UNUSED_(argv);
	DLOGD( "Hello,world! \r\n");

	::football::FootballConfig::print_version();

	NativeHooApi_INITIALIZE();
	NativeFootballReceiver_INITIALIZE();
	NativeServiceFootball_INITIALIZE();
	TestService_INITIALIZE();

/** when bootanimation completed, the framework is fully started, but /sdcard/ is still not accessible after a short time ? */
	if (::football::utils::ProcessBootAnimCompleted() == false) {
		while(::football::utils::ProcessBootAnimCompleted() == false) { usleep(500*1000); }
		usleep(3*1000*1000);
		// todo: how to check /sdcard/ is just available... 
	}


    static const struct option longOptions[] = {
        { "help",               no_argument,        NULL, 'h' },
        { "version",            no_argument,        NULL, 'v' },
        { "verbose",            no_argument,        NULL, 'b' },
        { "daemon",             no_argument,  		NULL, 'd' },
        { "service",            no_argument,  		NULL, 's' },
 		{ "alone",              no_argument,  		NULL, 'a' },
 		{ "log",              no_argument,  		NULL, 'l' },
        { "dummy",              no_argument,  		NULL, 'm' },
        { NULL,                 0,                  NULL, 0 }
    };
	int run_for_daemon = 0;
	int run_service_ = 0;
	int run_client_ = 1;

	int quick_test_index = -1;
	int run_alone = 0;
	int run_for_receiver = 0;

	while (true) {
		int optionIndex = 0;
		int ic = getopt_long(argc, argv, OPT_QUICK_TEST "ahvbdsml", longOptions, &optionIndex);
		if (ic == -1) {
			break;
		}
		switch (ic) {
		case 'h': {
			DLOGD("no option		run as cmdline clientl \r\n");
			DLOGD("--help/-h		print this message \r\n");
			DLOGD("--verbose/-v		verbose log \r\n");
			DLOGD("--daemon/-d		run as daemon \r\n");
			DLOGD("--service/-s		run as service \r\n");
			DLOGD("--alone/-a		run as alone application  \r\n");
			DLOGD("--log/-l			run as receiver application  \r\n");
			DLOGD("--dummy/-m 	dummy \r\n");
			return 0;
		}
		case 'v': {
			::football::FootballConfig::print_version();
			return 0;
		}
		case 'b': {
			break;
		}
		case 'd': {
			run_for_daemon = 1;
			break;
		}
		case 's': {
			run_service_ = 1;
			break;
		}
		case 'a': {
			run_alone = 1;
		}
		case 'l': {
			run_for_receiver = 1;
		}
		case 'm': {
			break;
		}
#if USE_QUICK_TEST_
		case 't': {
			if (optarg == NULL || cycling::_c__atoi(optarg, &quick_test_index) < 0) {
				DLOGD( "quick_test_index invalid! \r\n");
				return 0;
			}
			DLOGD( "quick_test_index = %d \r\n", quick_test_index);
			break;
		}
		case 'f': {
			if (optarg == NULL || cycling::_c__atoi(optarg, &quick_test_index) < 0) {
				DLOGD( "quick_test_index invalid! \r\n");
				return 0;
			}
			DLOGD( "quick_test_index = %d \r\n", quick_test_index);
			break;
		}
#endif
		}
	}

	if (run_service_) {
		run_for_receiver = 0;
	}
	if (run_for_daemon) {
		run_for_receiver = 0;
	}

	if (run_alone) {
		run_service_ = 0;
		run_client_ = 0;
		run_for_daemon = 0;
		run_for_receiver = 0;
	}
	if (run_service_ && run_client_) {
		run_client_ = 0;
	}
	if (run_for_daemon && run_client_) {
		run_client_ = 0;
	}


//////////////////////////////////////////////////////////////////
	if (run_for_daemon) {
		DLOGD("run_for_daemon. \r\n");
		daemon(0, 0) ;

		{
			uint32_t initFlags = 0x00;
			
			initFlags |= INIT_RUN_SERVICE;  // if daemon, always run service !!!

			TestCmdline *_test = new TestCmdline(initFlags);
			if (_test->isValid()) {
				_test->initCmd();
#if USE_QUICK_TEST_
				if (run_client_ == 0 
					//&& run_service_ == 0
					) {
					QUICK_TEST_RUN_COMMAND(_test, quick_test_index);
				}
#endif

				_test->loop();
			}
			else {
				DLOGD( "%s,%d not valid!", __func__, __LINE__);
			}
			delete _test;
		}

		#if 0
		{
			uint64_t running_tick = 0; // ~ 584,942,417,355 years to overflow
			while(1) {
				LOGW("%s running_tick %" PRIu64 "", __func__, running_tick++);
				usleep(1*1000*1000);
			}
		}
		#endif

		NativeHooApi_UNINITIALIZE();
		NativeServiceFootball_UNINITIALIZE();
		NativeFootballReceiver_UNINITIALIZE();
		TestService_UNINITIALIZE();
		return 0;
	}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


#if 1  // test cmdline	
	{
		uint32_t initFlags = 0x00;
		if (run_service_) {
			initFlags |= INIT_RUN_SERVICE;
			DLOGD("init service. \r\n");
		}
		if (run_client_) {
			initFlags |= INIT_FOR_CLIENT;
			DLOGD("init client. \r\n");
		}
		if (run_for_receiver) {
			initFlags |= INIT_FOR_RECEIVER;
			DLOGD("init receiver. \r\n");
		}
		TestCmdline *_test = new TestCmdline(initFlags);
		do {
			if (run_for_receiver) {
				if (_test->isReceiverRegistered() != 1) {
					DLOGD( "************* warning: service not ready!!! ***************** \r\n");
					break;
				}
			}
			if (!_test->isValid()) {
				DLOGD( "%s,%d TestCmdline not valid!", __func__, __LINE__);
				break;
			}
			_test->initCmd();
#if USE_QUICK_TEST_
			if (run_client_ == 0 
				) {
				QUICK_TEST_RUN_COMMAND(_test, quick_test_index);
			}
#endif
			_test->loop();
		} while(0);

		delete _test;
	}
#endif

	NativeHooApi_UNINITIALIZE();
	NativeServiceFootball_UNINITIALIZE();
	NativeFootballReceiver_UNINITIALIZE();
	TestService_UNINITIALIZE();

	return 0;
}

};


