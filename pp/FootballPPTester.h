#ifndef __FOOTBALL_PP_TESTER_H__
#define __FOOTBALL_PP_TESTER_H__

#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT

#include "utils/ANativeWindowUtils.h"

#include "pp/FootballPP.h"
#include "pp/FootballPPFactory.h"

#include "miniled/FootballBlController.h"

#include "ndk_extend/NativeHooApi_Loader.h"

namespace football {

class FileManager_;


class FootballPPTester {
public:

	DECLARE_SessionInfo_on_frame(on_frame_);

	enum {
		SOURCE_FILE = 0,
		SOURCE_DISPLAY = 1,
	};
	enum {
		SINK_SURFACE = 0,
		SINK_IMAGE_READER = 1,
	};
	FootballPPTester(FootballBlController *bl_controller,
		int pp_type_ = FootballPPFactory::PP_CPU,
		int pp_sub_type = 0,
		int source_type_ = SOURCE_FILE,
		int sink_type = SINK_SURFACE
		);
	~FootballPPTester();

	int getPPType() { return mPPType; }
	int getSessionType() { return mPPSessionType; }
	int getSourceType() { return source_type; }

	// with file as source
	int directorySet(const char *dir_);
	const char *directoryGet();
	int directoryRescan();
	int process_with_file(const char *filename_, int have_algo = 1);
	void process_next_file(int num_of_file = 1);
	void process_prev_file(int num_of_file = 1);
	void process_current_file(int num_of_times, int have_algo);

	// with display as source
	enum {
		VD_OP_CLOSE = 0,
		VD_OP_OPEN = 1,
		VD_OP_TOGGLE = 2,
	};
	int virtual_display_source_setup(int flags);

	//
	int setParameter(SessionParameter *parameter);
	void print();

	void test();

	FootballBlController *mFootballBlController = nullptr;
	int mPPType = FootballPPFactory::PP_VK;
	int mPPSessionType = 0;
	int source_type = SOURCE_FILE;
	int mSinkType = SINK_SURFACE;
	
	FootballPP *mFootballPP = nullptr;

	//
	int virtual_display_source_ready = 0;
	ANativeHooDisplay *mNativeHooDisplay = nullptr;

	//
	int mTargetWidth = 0;
	int mTargetHeight = 0;
	int mBlDataWidth = 0;
	int mBlDataHeight = 0;
	
	int mSessionId = -1;
		/** this info is maintained and shared in the all  lifetime of session !!! **/
	SessionInfo mSessionInfo;
	
	ANativeHooSurface *mHooSurface = nullptr;
	int first_few_frame_comming = 0;
	std::mutex mHooSurface_mutex_;

	TestReader *mFinalImageReader = nullptr;
	
	TestReader *mBlDataReader = nullptr;

	long mFrameIndex = 0;

	FileManager_ *mFileManager_ = nullptr;
};

};

#endif

