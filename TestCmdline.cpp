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
//#include <xf86drm.h>
//#include <xf86drmMode.h>

//#include <xf86drm.h>
//#include <xf86drmMode.h>
//#include <drm_fourcc.h>
#endif


#include "CmdLineUtils.h"

#include "miniled/TestCmdMiniLed.h"

#include "TestCmdline.h"

#include "FootballPPTester.h"

#include "ndk_extend/NativeHooApi.h"

#include "impl/FootballPPGles.h"
#include "impl/FootballPPVk.h"

#include "cmdline/cmdline_lua/CmdlineLuaImpl.h"


#undef UNUSED_
#define UNUSED_(x) ((void)x)


using namespace football;

namespace NS_test_cmdline {

IMPL_EMPTY_CB_WRAPPER(TestCmdline, TestCmdline, onEmptyCmd);

TestCmdline::TestCmdline(uint32_t flags):
	mInitFlags(flags)
	, mCmdline(::android::cmdline::factory::makeCmdline())
	{
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

	ANativeProcessState_startThreadPool();

	ADD_EMPTY_CB_FUNC(mCmdline, TestCmdline, onEmptyCmd, this);

	uint32_t TestCmdMiniLed_flags = 0x00;
	mTestCmdMiniLed = new NS_test_miniled::TestCmdMiniLed(TestCmdMiniLed_flags);

	mValid = 1;

	mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);
}
TestCmdline::~TestCmdline() {
	if (mFootballPPTester != nullptr) {
		delete mFootballPPTester;
	}
	delete mTestCmdMiniLed;
	delete mCmdline;
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}
void TestCmdline::onEmptyCmd() {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}

void TestCmdline::checkFootballPPTester_created() {
	if (mFootballPPTester != nullptr && mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_FILE) {
		delete mFootballPPTester;
		mFootballPPTester = nullptr;
	}

	if (mFootballPPTester == nullptr) {
		fprintf(stderr, "%s! \r\n", __func__);
		mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);
	}
}

void TestCmdline::initCmd() {
	// init command line
	mCmdline->setPrompt("footTool>>");

#if TestPanelTool_TEST
	mCmdline->add("test1", "test1", cmdline_test1, this);
	mCmdline->add("test2", "test2", cmdline_test2, this);
	mCmdline->add("test3", "test3", cmdline_test3, this);
#endif
	CL_ADD_FUNC(mCmdline, date);

	CL_ADD_FUNC(mCmdline, fp);
	CL_ADD_FUNC(mCmdline, re);
	CL_ADD_FUNC(mCmdline, set);
	CL_ADD_FUNC(mCmdline, n);
	CL_ADD_FUNC(mCmdline, p);
	CL_ADD_FUNC(mCmdline, c);
	CL_ADD_FUNC(mCmdline, vd);
	//CL_ADD_FUNC(mCmdline, bl);
	CL_ADD_FUNC(mCmdline, pr);

	CL_ADD_FUNC_f(mCmdline, init, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, init));
	CL_ADD_FUNC_f(mCmdline, w, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, w));
	CL_ADD_FUNC_f(mCmdline, r, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, r));
	CL_ADD_FUNC_f(mCmdline, bl, mTestCmdMiniLed, CL_SFUNC_NS(NS_test_miniled::TestCmdMiniLed, bl));

	date(0, nullptr);
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
	//fprintf(stderr, "panel test tool, version:" PANEL_TEST_TOOL_VERSION " \r\n");
	fprintf(stderr, "build at %s %s \r\n", __DATE__, __TIME__);
	return 0;
}
CL_SFUNC_IMPL(TestCmdline, TestCmdline, fp);
int TestCmdline::fp(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	int num_of_test = -1;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &num_of_test) < 0) {
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
		if (num_of_test < 0 || num_of_test > 50000) {
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
	} else if(argc > 2) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}

	if (num_of_test < 0) {  // default toggle tester !!!
		if (mFootballPPTester != nullptr) {
			fprintf(stderr, "*** cleanup tester! \r\n");
			delete mFootballPPTester;
			mFootballPPTester = nullptr;
		} else {
			fprintf(stderr, "*** create tester! \r\n");
			mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);
		}
	}
	else if (num_of_test > 0) {  // other value , do delete/create cycle with specific times !!!
		for(int i=0;i<num_of_test;i++) {
			if (mFootballPPTester != nullptr) {
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
			mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed);
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
			fprintf(stderr, "%s \r\n", mFootballPPTester->directoryGet());
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
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
		if (num_of_file <= 0 || num_of_file > 50000) {
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
	} else if(argc > 2) {
		fprintf(stderr, "input invalid! \r\n");
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
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
		if (num_of_file <= 0 || num_of_file > 50000) {
			fprintf(stderr, "num_of_file invalid! \r\n");
			return 0;
		}
	} else if(argc > 2) {
		fprintf(stderr, "input invalid! \r\n");
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
			fprintf(stderr, "have_algo invalid! \r\n");
			return 0;
		}
		if (have_algo < 0 || have_algo > 50000) {
			fprintf(stderr, "have_algo invalid! \r\n");
			return 0;
		}
	} else if(argc == 3) {
		if (cycling::_c__atoi(argv[1], &num_of_times) < 0) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
		if (num_of_times <= 0 || num_of_times > 90000) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
		//
		if (cycling::_c__atoi(argv[2], &have_algo) < 0) {
			fprintf(stderr, "have_algo invalid! \r\n");
			return 0;
		}
		if (have_algo < 0 || have_algo > 50000) {
			fprintf(stderr, "have_algo invalid! \r\n");
			return 0;
		}
	} else if(argc > 3) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}

	if (mFootballPPTester != nullptr) {
		mFootballPPTester->process_current_file(num_of_times, have_algo);
	}

	return 0;
}

CL_SFUNC_IMPL(TestCmdline, TestCmdline, vd);
int TestCmdline::vd(int argc, char *const *argv) {
	UNUSED_(argc); UNUSED_(argv);
	int num_of_times = 0;
	long render_wait_ms_times = 100;
	
	if (argc == 2) {
		fprintf(stderr, "argc:%d argv[0]:%s argv[1]:%s \r\n", argc, argv[0], argv[1]);
		
		if (cycling::_c__atoi(argv[1], &num_of_times) < 0) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
		if (num_of_times <= 0 || num_of_times > 10000000) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
	}
	else if (argc == 3) {
		fprintf(stderr, "argc:%d argv[0]:%s argv[1]:%s argv[2]:%s \r\n", argc, argv[0], argv[1], argv[2]);
		
		if (cycling::_c__atoi(argv[1], &num_of_times) < 0) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
		if (num_of_times <= 0 || num_of_times > 10000000) {
			fprintf(stderr, "num_of_times invalid! \r\n");
			return 0;
		}
		if (cycling::_c__atol(argv[2], &render_wait_ms_times) < 0) {
			fprintf(stderr, "render_wait_ms_times invalid! \r\n");
			return 0;
		}
		if (render_wait_ms_times < 0 || render_wait_ms_times > 100000*1000) {
			fprintf(stderr, "render_wait_ms_times invalid! \r\n");
			return 0;
		}
	}
	else if (argc > 3) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}

	fprintf(stderr, "num_of_times:%d render_wait_ms_times: %ld \r\n", num_of_times, render_wait_ms_times);

	if (num_of_times > 0) {
		for(int i=0;i<num_of_times;i++) {
			fprintf(stderr, "######################################### num_of_time = %d \r\n", i);
			if (mFootballPPTester != nullptr 
				//&& mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_DISPLAY
				) {
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
			if (mFootballPPTester == nullptr) {
				mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, FootballPPTester::SOURCE_DISPLAY);
			}
			if (mFootballPPTester != nullptr) {
				mFootballPPTester->virtual_display_source_setup(1);
				if (render_wait_ms_times > 0) {
					usleep(render_wait_ms_times*1000);
				}
				else if (render_wait_ms_times == 0) {
					fprintf(stderr, "******************************************************** \r\n");
					fprintf(stderr, "******************* wait forever !!!");
					fprintf(stderr, "******************************************************** \r\n");
					while(1) { usleep(1000); }
				}
				delete mFootballPPTester;
				mFootballPPTester = nullptr;
			}
		}
		return 0;
	}
	
	if (mFootballPPTester != nullptr && mFootballPPTester->getSourceType() != FootballPPTester::SOURCE_DISPLAY) {
		delete mFootballPPTester;
		mFootballPPTester = nullptr;
	}
	if (mFootballPPTester == nullptr) {
		mFootballPPTester = new football::FootballPPTester(mTestCmdMiniLed, FootballPPTester::SOURCE_DISPLAY);
	}
	if (mFootballPPTester != nullptr) {
		mFootballPPTester->virtual_display_source_setup(2);
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

#if TestPanelTool_TEST

/*static*/ int TestCmdline::cmdline_test1(void *ctx, int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);

#if 1
{
	// called here using commandline v2 implementation, has problem !!!
	return ::android::CmdlineLuaImpl::test(argc, (char**)argv);
}
#endif

#if 0
{
	football::FootballPPTester * tester = new football::FootballPPTester(nullptr);
	tester->test();
	delete tester;
}
#endif

#if 0
{
	int num_of_cycle = 1;
	long render_frames = 10;
	if (argc == 2) {
		if (cycling::_c__atol(argv[1], &render_frames) < 0) {
			fprintf(stderr, "render_frames invalid! \r\n");
			return 0;
		}
		if (render_frames <= 0 || render_frames > 100000*1000) {
			fprintf(stderr, "render_frames invalid! \r\n");
			return 0;
		}
	}
	else if (argc == 3) {
		if (cycling::_c__atoi(argv[1], &num_of_cycle) < 0) {
			fprintf(stderr, "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (num_of_cycle <= 0 || num_of_cycle > 98000) {
			fprintf(stderr, "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (cycling::_c__atol(argv[2], &render_frames) < 0) {
			fprintf(stderr, "render_frames invalid! \r\n");
			return 0;
		}
		if (render_frames <= 0 || render_frames > 100000*1000) {
			fprintf(stderr, "render_frames invalid! \r\n");
			return 0;
		}
	} else if (argc > 3) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}

	FootballPPVk::test2(num_of_cycle, render_frames);
}
#endif

	return 0;
}
/*static*/ int TestCmdline::cmdline_test2(void *ctx,int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);

#if 1
{
	int num_of_cycle = 1;
	long wait_ms = 100;
	if (argc == 2) {
		if (cycling::_c__atol(argv[1], &wait_ms) < 0) {
			fprintf(stderr, "wait_ms invalid! \r\n");
			return 0;
		}
		if (wait_ms <= 0 || wait_ms > 100000*1000) {
			fprintf(stderr, "wait_ms invalid! \r\n");
			return 0;
		}
	}
	else if (argc == 3) {
		if (cycling::_c__atoi(argv[1], &num_of_cycle) < 0) {
			fprintf(stderr, "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (num_of_cycle <= 0 || num_of_cycle > 98000) {
			fprintf(stderr, "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (cycling::_c__atol(argv[2], &wait_ms) < 0) {
			fprintf(stderr, "wait_ms invalid! \r\n");
			return 0;
		}
		if (wait_ms <= 0 || wait_ms > 100000*1000) {
			fprintf(stderr, "wait_ms invalid! \r\n");
			return 0;
		}
	} else if (argc > 3) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}

	FootballPPGles::test1(num_of_cycle, wait_ms);
}
#endif

	return 0;
}
/*static*/ int TestCmdline::cmdline_test3(void *ctx,int argc, char *const*argv) {
	UNUSED_(ctx), UNUSED_(argc); UNUSED_(argv);

#if 0  // ANativeHooSurface test
{
#define ANativeHooSurface_TEST1_NUM (1000)
	for(int i=0;i<ANativeHooSurface_TEST1_NUM;i++) {
		ANativeHooSurface *hooSurface = nullptr;
		//    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM           = 1,
		int ret = ANativeHooSurface_create("", 1080, 1920, 0x01, 0, &hooSurface);
		if (ret == 0 && hooSurface != nullptr) {
			//////////////////////////////// fill input_window
			{
				ANativeWindow *inputWindow = nullptr;
				ANativeHooSurface_getWindow(hooSurface, &inputWindow);
				if (inputWindow != nullptr) {
					ANativeWindow_acquire(inputWindow);
					
					int32_t width_ = ANativeWindow_getWidth(inputWindow);
					int32_t height_ = ANativeWindow_getHeight(inputWindow);
					int32_t format_ = ANativeWindow_getFormat(inputWindow);
					fprintf(stderr, "fill window size:%04dx%04d format:0x%08x \r\n", width_, height_, format_);
					// 1 , ABGR
				
					ANativeWindow_Buffer lockBuffer;
					if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
						fprintf(stderr, "	 lockBuffer size:%04dx%04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
						uint32_t *pixel = (uint32_t *)lockBuffer.bits;
						int line = 0;
						for(;line<lockBuffer.height && line < 200;line++, pixel += lockBuffer.stride) {
							//memset((void*)pixel, 0xa0, lockBuffer.stride*4);
							for(int col=0;col<lockBuffer.width;col++) {
								pixel[col] = 0xffff0000;
							}
						}
						for(;line<lockBuffer.height && line < 400;line++, pixel += lockBuffer.stride) {
							//memset((void*)pixel, 0xa0, lockBuffer.stride*4);
							for(int col=0;col<lockBuffer.width;col++) {
								pixel[col] = 0xff00ff00;
							}
						}
						for(;line<lockBuffer.height;line++, pixel += lockBuffer.stride) {
							//memset((void*)pixel, 0xa0, lockBuffer.stride*4);
							for(int col=0;col<lockBuffer.width;col++) {
								pixel[col] = 0xff0000ff;
							}
						}
						ANativeWindow_unlockAndPost(inputWindow);
					}
					ANativeWindow_release(inputWindow);
				}
			}
			//////////////////////////////// fill done !!!
			//usleep(5*1000*1000);
			ANativeHooSurface_destroy(hooSurface);
		}
	}
}
#endif

#if 1  // ndk display test
{
	int ret = 0;
	ANativeWindow *display_window = nullptr;
	football::TestReader * mTestReader = nullptr;
	ANativeHooSurface *hooSurface = nullptr;

#if 1
	ret = ANativeHooSurface_create("", 1080, 1920, 0x01, 0, &hooSurface);
	if (ret == 0 && hooSurface != nullptr) {
		ANativeHooSurface_getWindow(hooSurface, &display_window);
	}
#endif
	if (display_window == nullptr) {
		fprintf(stderr, "surface window create failed !\r\n");
		mTestReader = new football::TestReader(1080, 1920, 0x01, 3,
			AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
			| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
			//| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
			| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER);
		display_window = mTestReader->getANativeWindow();
	}
	if (display_window == nullptr) {
		fprintf(stderr, "### NO window created !\r\n");
		return 0;
	}

	ANativeHooDisplay *display_ = nullptr;

	ret = ANativeHooDisplay_create("#test_display", 1080, 1920, display_window, &display_);
	fprintf(stderr, "ANativeHooDisplay_create ret=%d \r\n", ret);
	if (ret == 0 && display_ != nullptr) {
		usleep(5*1000*1000);
		ANativeHooDisplay_destroy(display_);
	}

	usleep(100*1000);

	if (hooSurface != nullptr) {
		ANativeHooSurface_destroy(hooSurface);
	}
	if (mTestReader != nullptr) {
		delete mTestReader;
	}
}
#endif
	
	return 0;
}
#endif

};


