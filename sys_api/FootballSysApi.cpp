#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()
#include <errno.h>


#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>


#include "FootballConfig.h"


static const char* kTAG = "football-sysapi";
#include "utils/android_logcat_.h"

#include "utils/foot_utils.h"
#include "utils/FootThread.h"
#include "utils/StbImage_.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#include "miniled/TestCmdMiniLed.h"

#include "standalone/AAssetManagerImpl_.h"

#include "TestCmdline.h"

#include "FootballSysApi.h"

#undef __CLASS__
#define __CLASS__ "FootballSysApi"

namespace football {

class FootballSysApi {
public:
	FootballSysApi();
	virtual ~FootballSysApi();
};

FootballSysApi::FootballSysApi() {

}
FootballSysApi::~FootballSysApi() {

}


};


static std::mutex s_api_lock_;
struct AFootballSysApi {
	NS_test_cmdline::TestCmdline *_pTestCmdline = nullptr;
	football::FootThread *_pDaemonThread = nullptr;
};


int AFootballSysApi_setAssetsBasePath(const char *base_path_) {
	::android_facade::AAssetManagerImpl_setAssetBasePath(base_path_);
	return 0;
}

// __attribute__ ((visibility("default")))
int AFootballSysApi_create(AFootballSysApi **api_) {
	std::unique_lock<std::mutex> caller_lock(s_api_lock_);
	LOGW("%s", __func__);

	AFootballSysApi *sysapi_ = new AFootballSysApi();

	uint32_t initFlags = 0x00;
	sysapi_->_pTestCmdline = new NS_test_cmdline::TestCmdline(initFlags);
	if (sysapi_->_pTestCmdline->isValid()) {
		sysapi_->_pTestCmdline->initCmd();
		*api_ = sysapi_;
		return 0;
	}
	delete sysapi_->_pTestCmdline;
	delete sysapi_;
	return -1;
}
int AFootballSysApi_football_daemon_thread_start(AFootballSysApi *api_) {
	std::unique_lock<std::mutex> caller_lock(s_api_lock_);
	LOGW("%s", __func__);
#if FootballSysApi_AUTO_RUN_
	class daemon_thread: public football::FootThread {
	public:
		AFootballSysApi *mApi_ = nullptr;
		daemon_thread(AFootballSysApi *api_): football::FootThread(), mApi_(api_) {
			__start();
		}
		~daemon_thread() {}
		virtual int thread_loop() override {
			mApi_->_pTestCmdline->runCommand("vpp -p 2 -c 1 -t 0");
			return 0; // exit thread
		}
	};
	if (api_ != nullptr) {
		api_->_pDaemonThread = new daemon_thread(api_);
	}
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////
// stop, release ... if have chance to run ...
int AFootballSysApi_football_daemon_thread_stop(AFootballSysApi *api_) {
	std::unique_lock<std::mutex> caller_lock(s_api_lock_);
	LOGW("%s", __func__);
#if FootballSysApi_AUTO_RUN_
	if (api_->_pDaemonThread != nullptr) {
		api_->_pDaemonThread->set_worker_thread_exit_(true);
	}
	api_->_pTestCmdline->setPendingCommandFlags(NS_test_cmdline::TestCmdline::COMMAND_EXIT);
	if (api_->_pDaemonThread != nullptr) {
		delete api_->_pDaemonThread;
		api_->_pDaemonThread = nullptr;
	}
	api_->_pTestCmdline->setPendingCommandFlags(0);
#endif
	return 0;
}
int AFootballSysApi_release(AFootballSysApi *api_) {
	std::unique_lock<std::mutex> caller_lock(s_api_lock_);
	LOGW("%s", __func__);
	
	if (api_ != nullptr) {
		AFootballSysApi_football_daemon_thread_stop(api_);
		if (api_->_pTestCmdline != nullptr) {
			delete api_->_pTestCmdline;
			api_->_pTestCmdline = nullptr;
		}
		delete api_;
	}
	return 0;
}




