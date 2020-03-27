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

int TestCmdline::test_FootballPPVk_test2(int argc, char *const*argv) {
#if 0
	{
		int num_of_cycle = 1;
		long render_frames = 10;
		if (argc == 2) {
			if (cycling::_c__atol(argv[1], &render_frames) < 0) {
				DLOGD( "render_frames invalid! \r\n");
				return 0;
			}
			if (render_frames <= 0 || render_frames > 100000*1000) {
				DLOGD( "render_frames invalid! \r\n");
				return 0;
			}
		}
		else if (argc == 3) {
			if (cycling::_c__atoi(argv[1], &num_of_cycle) < 0) {
				DLOGD( "num_of_cycle invalid! \r\n");
				return 0;
			}
			if (num_of_cycle <= 0 || num_of_cycle > 98000) {
				DLOGD( "num_of_cycle invalid! \r\n");
				return 0;
			}
			if (cycling::_c__atol(argv[2], &render_frames) < 0) {
				DLOGD( "render_frames invalid! \r\n");
				return 0;
			}
			if (render_frames <= 0 || render_frames > 100000*1000) {
				DLOGD( "render_frames invalid! \r\n");
				return 0;
			}
		} else if (argc > 3) {
			DLOGD( "input invalid! \r\n");
			return 0;
		}
	
		FootballPPVk::test2(num_of_cycle, render_frames);
	}
#endif
	return 0;
}

int TestCmdline::test_FootballPPGles_test1(int argc, char *const*argv) {
#if 1
{
	int num_of_cycle = 1;
	long wait_ms = 100;
	if (argc == 2) {
		if (cycling::_c__atol(argv[1], &wait_ms) < 0) {
			DLOGD( "wait_ms invalid! \r\n");
			return 0;
		}
		if (wait_ms <= 0 || wait_ms > 100000*1000) {
			DLOGD( "wait_ms invalid! \r\n");
			return 0;
		}
	}
	else if (argc == 3) {
		if (cycling::_c__atoi(argv[1], &num_of_cycle) < 0) {
			DLOGD( "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (num_of_cycle <= 0 || num_of_cycle > 98000) {
			DLOGD( "num_of_cycle invalid! \r\n");
			return 0;
		}
		if (cycling::_c__atol(argv[2], &wait_ms) < 0) {
			DLOGD( "wait_ms invalid! \r\n");
			return 0;
		}
		if (wait_ms <= 0 || wait_ms > 100000*1000) {
			DLOGD( "wait_ms invalid! \r\n");
			return 0;
		}
	} else if (argc > 3) {
		DLOGD( "input invalid! \r\n");
		return 0;
	}

	FootballPPGles::test1(num_of_cycle, wait_ms);
}
#endif
	return 0;
}

int TestCmdline::test_ANativeServiceFootballProxy(int argc, char *const*argv) {
#if 1
{
	for(int i=0;i<1000;i++) {
		ANativeServiceFootballProxy *proxy_ = nullptr;
		if(ANativeServiceFootballProxy_create(football::FootballConfig::cycling_service_name, &proxy_) == 0) {
			if (ANativeServiceFootballProxy_isValid(proxy_)) {
				NativeServiceFootball *p_ = ANativeServiceFootballProxy_get(proxy_);
				DLOGD( "%s,  ANativeServiceFootball_Proxy_isValid !  i = %d \r\n", __func__, i);
				p_->dummy();
				p_->dummy2();
				p_->dummy3(0, 0);
			}
			ANativeServiceFootballProxy_destroy(proxy_);
		}
	}
}
#endif
	return 0;
}
int TestCmdline::test_FootballService_Impl1(int argc, char *const*argv) {
#if 0  // test service
		{
			if (mFootballService_Impl1 == nullptr) {
	/* can be registered only once !!!	can be registered only once !!!  can be registered only once !!!  
	
	2019-01-02 07:13:58.048 11416-11416/? A/DEBUG: backtrace:
	2019-01-02 07:13:58.049 11416-11416/? A/DEBUG:	   #00 pc 000000000000c5b0	/system/lib64/libutils.so (android::RefBase::~RefBase()+36)
	2019-01-02 07:13:58.049 11416-11416/? A/DEBUG:	   #01 pc 000000000006d520	/system/lib64/libsfhookerapi.so (native_service::NativeServiceFootball_Stub::~NativeServiceFootball_Stub()+52)
	2019-01-02 07:13:58.049 11416-11416/? A/DEBUG:	   #02 pc 000000000006d16c	/system/lib64/libsfhookerapi.so (ANativeServiceFootball_destroy+40)
	*/
				FootballService_Impl1 *Impl = new FootballService_Impl1(nullptr);
				mFootballService_Impl1 = Impl;
				// join here !!!
				//ANativeIPCThreadState_joinThreadPool();
	
			} else {
				delete mFootballService_Impl1;
				mFootballService_Impl1 = nullptr;
			}
		
		}
#endif
	return 0;
}

int TestCmdline::test_ndk_extend_1(int argc, char *const*argv) {
	
#if 0
		//::football::utils::foot_utils_test_system_properties_h();
	
	{
		AFootballSysApi *sys_api_ = nullptr;
		if (AFootballSysApi_create(&sys_api_) == 0) {
			AFootballSysApi_football_daemon_thread_start(sys_api_);
			usleep(10*1000*1000);
			AFootballSysApi_football_daemon_thread_stop(sys_api_);
			AFootballSysApi_release(sys_api_);
		}
	}
#endif
#if 0  // special ANativeHooDisplay and ANativeHooSurface test
	{
		//const char *display_name_ = "#test_display";
		const char *display_name_ = FootballPPTester_special_DISPLAY_NAME;
	
		int ret = 0;
		ANativeWindow *display_window = nullptr;
		football::TestReader * mTestReader = nullptr;
	
		if (display_window == nullptr) {
			DLOGD( "surface window create failed !\r\n");
			mTestReader = new football::TestReader(1080, 1920, AIMAGE_FORMAT_RGBA_8888, 3,
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
				| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
				//| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
				| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER);
			display_window = mTestReader->getANativeWindow();
		}
		if (display_window == nullptr) {
			DLOGD( "### NO window created !\r\n");
			return 0;
		}
	
		ANativeHooDisplay *display_ = nullptr;
		ret = ANativeHooDisplay_create(display_name_, 1080, 1920, display_window, &display_);
		DLOGD( "ANativeHooDisplay_create ret=%d \r\n", ret);
		if (ret == 0 && display_ != nullptr) {
		}
	
	
#define ANativeHooSurface_TEST1_NUM (1000)
		for(int i=0;i<ANativeHooSurface_TEST1_NUM;i++) {
			ANativeHooSurface *hooSurface = nullptr;
			//	  AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM 		  = 1,
	
			const char *surface_name_ = FootballPPTester_special_SURFACE_NAME;
			//const char *surface_name_ = "#dummy_1";
		
			int ret = ANativeHooSurface_create(surface_name_, 1080, 1920, AIMAGE_FORMAT_RGBA_8888, 
				ANativeHoo_ISurfaceComposerClient_eHidden
				//| ANativeHoo_ISurfaceComposerClient_eOpaque
				, 
				&hooSurface);
			if (ret == 0 && hooSurface != nullptr) {
				//////////////////////////////// fill input_window
				{
					ANativeWindow *inputWindow = nullptr;
					ANativeHooSurface_getWindow(hooSurface, &inputWindow);
					if (inputWindow != nullptr) {
						ANativeWindow_acquire(inputWindow);
	
#if 1
						int32_t width_ = ANativeWindow_getWidth(inputWindow);
						int32_t height_ = ANativeWindow_getHeight(inputWindow);
						int32_t format_ = ANativeWindow_getFormat(inputWindow);
						DLOGD( "fill window size:%04dx%04d format:0x%08x \r\n", width_, height_, format_);
						// 1 , ABGR
					
						ANativeWindow_Buffer lockBuffer;
						if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
							DLOGD( "	 lockBuffer size:%04dx%04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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
#endif
	
						ANativeWindow_release(inputWindow);
					}
				}
				//////////////////////////////// fill done !!!
				usleep(5*1000*1000);
				ANativeHooSurface_destroy(hooSurface);
			}
		}
	
		if (display_ != nullptr) {
			ANativeHooDisplay_destroy(display_);
		}
	
		if (mTestReader != nullptr) {
			delete mTestReader;
		}
	}
#endif
	
#if 0  // ndk display test
	{
		int ret = 0;
		ANativeWindow *display_window = nullptr;
		football::TestReader * mTestReader = nullptr;
		ANativeHooSurface *hooSurface = nullptr;
	
#if 1
		ret = ANativeHooSurface_create("", 1080, 1920, AIMAGE_FORMAT_RGBA_8888, 0, &hooSurface);
		if (ret == 0 && hooSurface != nullptr) {
			ANativeHooSurface_getWindow(hooSurface, &display_window);
		}
#endif
		if (display_window == nullptr) {
			DLOGD( "surface window create failed !\r\n");
			mTestReader = new football::TestReader(1080, 1920, AIMAGE_FORMAT_RGBA_8888, 3,
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
				| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
				//| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
				| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER);
			display_window = mTestReader->getANativeWindow();
		}
		if (display_window == nullptr) {
			DLOGD( "### NO window created !\r\n");
			return 0;
		}
	
		ANativeHooDisplay *display_ = nullptr;
	
		ret = ANativeHooDisplay_create("#test_display", 1080, 1920, display_window, &display_);
		DLOGD( "ANativeHooDisplay_create ret=%d \r\n", ret);
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






};

