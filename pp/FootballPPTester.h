#ifndef __FOOTBALL_PP_TESTER_H__
#define __FOOTBALL_PP_TESTER_H__

#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT

#include "FootballPP.h"

#include "FootballBlController.h"

#include "ndk_extend/NativeHooApi.h"

namespace football {

class FileManager_;

class TestReader: public AImageReader_ImageListener {
public:
	static void s_TestReader_AImageReader_ImageCallback(void* context, AImageReader* reader);

	long mFrameIndex = 0;  // initial frame number is 0 !!!
	AImageReader *mReader = nullptr;

	std::mutex caller_mutex_;
	std::condition_variable caller_cv_;
	int cb_is_ongoing = 0;
	int destroyed = 0;
	void notify_cb_ongoing(int ongoing_);

	TestReader(int width, int height, int format = 0x01, int maxImages = 3, uint64_t usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN);
	virtual ~TestReader();
	ANativeWindow *getANativeWindow();

	// this waill wakeup waitData !
	void incFrameNumber();
	virtual void onImageAvailableCallback(AImageReader *reader);

	// wait until mFrameIndex >= frameIndex !
	virtual int waitFrame(long frameIndex, long timeout_ms);
};

class FootballPPTester {
public:
	enum {
		SOURCE_FILE = 0,
		SOURCE_DISPLAY = 1,
	};
	FootballPPTester(FootballBlController *bl_controller, int source_type_ = SOURCE_FILE);
	~FootballPPTester();
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
	int virtual_display_source_setup(int flags);

	//
	void print();

	void test();

	FootballBlController *mFootballBlController = nullptr;
	int source_type = SOURCE_FILE;
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
	FootSession mSession;
	TestReader *mFinalImageReader = nullptr;
	ANativeHooSurface *mHooSurface = nullptr;
	TestReader *mBlDataReader = nullptr;

	long mFrameIndex = 0;

	FileManager_ *mFileManager_ = nullptr;
};

};

#endif

