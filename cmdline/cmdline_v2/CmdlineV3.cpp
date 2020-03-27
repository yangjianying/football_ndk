
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT


#include "utils/football_debugger.h"

#include "cli.h"

#include "CmdlineV3.h"
#include "cmdline_v1/MenuV1.h"

#undef __CLASS__
#define __CLASS__ "CmdlineV3"

#define CMD_POSTER_TEST 0 // 1

namespace android {

static int __quit(void *, int argc, char * const argv[]) {
	return -10000;
}

#define STR_C11_LITERAL(s) (s)

class cli_impl_v3 : public ::cli {
public:
	cli_impl_v3(CmdlineV3 *pc, int flags) : cli(flags), pCmdline(pc) {
		//DLOGD( "%s,%d ... \r\n", __func__, __LINE__);
		//DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	}
	~ cli_impl_v3() {
		DLOGD( "%s,%d ... \r\n", __func__, __LINE__);
		reader_join();
		DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	}

	void reader_start() {
		// start reader thread
	    std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (started_ == 0) {
			started_ = 1;
		    std::thread worker_thread(thread_main, this);
		    worker_thread_.swap(worker_thread);
		    caller_cv_.wait(caller_lock);
		}
	}
	void reader_join() {
		// join reader thread
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (started_) {
			started_ = 0;
			set_exit_request(1);
			worker_thread_.join();
		}
	}
	static void thread_main(cli_impl_v3 *cli_) {
		if (cli_) {
			cli_->thread_task();
		}
	}

	void thread_task() {
		// New scope to limit scope of caller lock to this block.
		{
			//DLOGD( "%s,%d started \r\n", __func__, __LINE__);
		  // Signal caller thread that worker thread is ready to listen to events.
		  std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		  caller_cv_.notify_one();
		}

		//DLOGD( "%s, start ... \r\n", __func__);

		cli_loop(pCmdline->mPrompt.c_str());

		//DLOGD( "%s, done !!! \r\n", __func__);
	}

	virtual int cli_intercept_command_repeatable(const char *cmd, int flag) override {
		if (pCmdline != nullptr) {
			return pCmdline->cli_intercept_command_repeatable(cmd, flag);
		}
		return 0;
	}
	virtual int cli_cmd_process_(int flag, int argc, char * const argv[],
			       int *repeatable, ulong *ticks) override {
		if (pCmdline != nullptr) {
			return pCmdline->cli_cmd_process_(flag, argc, argv, repeatable, ticks);
		}
		return 0;
	}
	virtual void cli_cmd_empty() override {
		if (pCmdline != nullptr) {
			return pCmdline->cli_cmd_empty_();
		}
	}

	CmdlineV3 *pCmdline = nullptr;
	std::thread worker_thread_;
	std::mutex caller_mutex_;
	std::condition_variable caller_cv_;
	int started_ = 0;

};

class TestMessagePoster {
public:
	CmdlineV3 *cmd_ = nullptr;
	long total_ = 0;
	TestMessagePoster(CmdlineV3 *c_): cmd_(c_){
	    std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (started_ == 0) {
			started_ = 1;
		    std::thread worker_thread(thread_main, this);
		    worker_thread_.swap(worker_thread);
		    caller_cv_.wait(caller_lock);
		}
		DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	}
	~TestMessagePoster() {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (started_) {
			started_ = 0;
			set_exit_();
			worker_thread_.join();
		}
		DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	}
	static void thread_main(TestMessagePoster *poster) {
		if (poster) {
			poster->thread_task();
		}
	}

	void thread_task() {
		int cnt = 0;

		// New scope to limit scope of caller lock to this block.
		{
			//DLOGD( "%s,%d started \r\n", __func__, __LINE__);
		  // Signal caller thread that worker thread is ready to listen to events.
		  std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		  caller_cv_.notify_one();
		}
		while(get_exit_() == 0) {
			usleep(1000);
			cnt++;
			if (cnt >=100) {
				cnt = 0;
				cmd_->postCommand("help");
				total_++;
				DLOGD( "%s,%d total_ = %ld \r\n", __func__, __LINE__, total_);
			}
		}
	}

	std::thread worker_thread_;
	std::mutex caller_mutex_;
	std::condition_variable caller_cv_;
	int started_ = 0;
	int exit_ = 0;
	std::mutex exit_mutex_;
	int get_exit_() {
		std::unique_lock<std::mutex> lock_(exit_mutex_);
		return exit_;
	}
	void set_exit_() {
		std::unique_lock<std::mutex> lock_(exit_mutex_);
		exit_ = 1;
	}

};
CmdlineV3::CmdlineV3(int opt_):  Cmdline(opt_)  {
	impl1 = (void *)new cli_impl_v3(this, 
		(opt_ & CLI_OPT_console_reader) == 0
		? cli::cli_FLAG_intercept_raw | cli::cli_FLAG_non_blocking_getchar | cli::cli_FLAG_no_init_console
		: cli::cli_FLAG_intercept_raw | cli::cli_FLAG_non_blocking_getchar );
	menu_impl = (void*) new ::NS_cmdline_v1::NS_menu_v1::MenuV1();
}
CmdlineV3::~ CmdlineV3() {
	if (post_tester_ == nullptr) {
		delete post_tester_;
		post_tester_ = nullptr;
	}
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	delete menu;

	cli_impl_v3 *_impl = (cli_impl_v3*)impl1;
	delete _impl;

}

void CmdlineV3::setPrompt(const char *prompt)  {
	//DLOGD( "%s, prompt:%s \r\n", __func__, prompt);
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	mPrompt = std::string(prompt);
}
void CmdlineV3::add_on_empty_cmd(PF_on_empty_cmd cb, void *ctx) {
	mPF_on_empty_cmd = cb;
	mPF_on_empty_cmd_ctx = ctx;
}
void CmdlineV3::add_on_intercept_command(PF_on_intercept_command pf, void *ctx) {
	mPF_on_intercept_command = pf;
	mPF_on_intercept_command_ctx = ctx;
}
void CmdlineV3::on_empty_cmd_i() {
}

int CmdlineV3::add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx)  {
	//DLOGD( "%s, desc:%s \r\n", __func__, desc);
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	return menu->menuConfig(STR_C11_LITERAL(cmd), STR_C11_LITERAL(desc), handler, ctx);
}
int CmdlineV3::loop()  {
	cli_impl_v3 *_impl = (cli_impl_v3*)impl1;

#if 1
	//DLOGD( "%s,%d tid=%lu start wait queue ... \r\n", __func__, __LINE__, pthread_self());

	// must start reader after everything is prepared !!!
	if (CLI_OPT_console_reader & mOpt) {
		_impl->reader_start();
	}
	else {
		DLOGD( "with no reader started ! \r\n");
#if CMD_POSTER_TEST
		if (post_tester_ == nullptr) {
			post_tester_ = new TestMessagePoster(this);
		}
#endif
	}


	// fetch from queue, then call _impl->run_cli_command(cmd);
	int should_fetching = 1;
	while(should_fetching) {
		{
			std::unique_lock<std::mutex> _lock(queue_mutex_);
			while(mQueue.size() <= 0) {
				queue_cv_.wait(_lock);
			}
		}
		while(1) {
			cmds_total_++;
			std::string front_;
			//DLOGD( "to run command. queue size:%ld cmds_total_:%ld \r\n", (long)mQueue.size(), cmds_total_);

			{
				std::unique_lock<std::mutex> _lock(queue_mutex_);
				front_ = mQueue.front();
				std::vector<std::string>::iterator iter_ = mQueue.begin();
				mQueue.erase(iter_);
			}

			//
			int rc = runCommand(front_.c_str());
			DLOGD( "rc = %d \r\n", rc);
			if (rc == -10000 || rc == -10001) {
				DLOGD( "quit from loop ... \r\n");
				should_fetching = 0;
				break;
			}

			//
			{
				std::unique_lock<std::mutex> _lock(queue_mutex_);
				if (mQueue.size() <= 0) {
					break;
				}
			}

		}
	}
#else
	_impl->cli_loop(mPrompt.c_str());
#endif

	if (post_tester_ == nullptr) {
		delete post_tester_;
		post_tester_ = nullptr;
	}

	return 0;
}
int CmdlineV3::check_command_matched(const char * cmd, const char *matched_) {
	cli_impl_v3 *_impl = (cli_impl_v3*)impl1;
	return _impl->check_cli_command_matched(cmd, matched_);
}
int CmdlineV3::postCommand(const char * cmd) {
	{
		std::unique_lock<std::mutex> _lock(queue_mutex_);
		mQueue.push_back(std::string(cmd));
		queue_cv_.notify_one();
		//DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	}
	return 0;
}
int CmdlineV3::runCommand(const char * cmd) {
	cli_impl_v3 *_impl = (cli_impl_v3*)impl1;
	return _impl->run_cli_command(cmd);
}
void CmdlineV3::addInternalCmd() {
	if (internalAddon == 0) {
		internalAddon = 1;

		::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
		//menu->menuConfig(STR_C11_LITERAL("clversion"), 
		//	STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION "(Based on Linux 3.18.6)"),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL(kCmd_clversion), 
			STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL(kCmd_quit),
			STR_C11_LITERAL("Quit from cmdline tool"), __quit, NULL);
		menu->menuConfig(STR_C11_LITERAL(kCmd_q_),
			STR_C11_LITERAL("Quit from cmdline tool"), __quit, NULL);

	}
}
int CmdlineV3::cli_intercept_command_repeatable(const char *cmd, int flag)  {
	//DLOGD( "%s,%d tid=%lu \r\n", __func__, __LINE__, pthread_self());

	if (mPF_on_intercept_command != nullptr) {
		return mPF_on_intercept_command(mPF_on_intercept_command_ctx, cmd, flag);
	}
	else {
		// post (const char *cmd, int flag) into queue
		postCommand(cmd);
	}

	return 0;
}

int CmdlineV3::cli_cmd_process_(int flag, int argc, char * const argv[],
				   int *repeatable, uint64_t *ticks) {
   CLI_UNUSED_(flag);
   CLI_UNUSED_(argc);
   CLI_UNUSED_(argv);
   CLI_UNUSED_(repeatable);
   CLI_UNUSED_(ticks);
   	//DLOGD( "%s,%d \r\n", __func__, __LINE__);
#if 0
   	DLOGD("%s, %d argc: %d \r\n", __func__, __LINE__, argc);
	for(int i=0;i<argc;i ++) {
		DLOGD("%04d, (%s)\r\n", i, argv[i]);
	}
#endif
	if (menu_impl != NULL) {
		::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
		return menu->cmd_process(argc, argv);
	}
   return 0;

}
void CmdlineV3::cli_cmd_empty_()     {
	if (mPF_on_empty_cmd != nullptr) {
		mPF_on_empty_cmd(mPF_on_empty_cmd_ctx);
	}
	else {
		on_empty_cmd_i();
	}
}

};



