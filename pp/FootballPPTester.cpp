#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <dirent.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "FootballConfig.h"

#include "ndk_extend/NativeHooApi.h"

#include "ANativeWindowUtils.h"
#include "StbImage_.h"

#include "FootballPPFactory.h"
#include "FootballPPTester.h"

using namespace std;

namespace football {

// file manager
/*
enum
{ 
    DT_UNKNOWN = 0, 
 # define DT_UNKNOWN DT_UNKNOWN 
     DT_FIFO = 1, 
 # define DT_FIFO DT_FIFO 
     DT_CHR = 2, 
 # define DT_CHR DT_CHR 
     DT_DIR = 4, 
 # define DT_DIR DT_DIR 
     DT_BLK = 6, 
 # define DT_BLK DT_BLK 
     DT_REG = 8, 
 # define DT_REG DT_REG 
     DT_LNK = 10, 
 # define DT_LNK DT_LNK 
     DT_SOCK = 12, 
 # define DT_SOCK DT_SOCK 
     DT_WHT = 14 
 # define DT_WHT DT_WHT 
}; 
*/

/////////////////////////////////////

class FileManager_ {
public:
	std::string mDirectory;
	vector<string> mFileList;
	int mFileIndex = -1;

	FileManager_(const char *directory_) {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
		setDirectory(directory_);
	}
	~FileManager_() {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	int setDirectory(const char *directory_) {
		mDirectory = directory_;
		if (directory_ != nullptr && directory_[strlen(directory_) - 1] == '/') {
			mDirectory = mDirectory.substr(0, strlen(directory_) - 1);
		}
		fprintf(stderr, "%s, mDirectory:%s \r\n", __func__, mDirectory.c_str());
		int ret = rescan();
		fprintf(stderr, "%s, mFileList.size() = %zd \r\n", __func__, mFileList.size());
		return ret;
	}
	const char* getDirectory() {
		return mDirectory.c_str();
	}
	int rescan() {
		mFileList.clear();
		mFileIndex = -1;
		fprintf(stderr, "%s, mDirectory:%s \r\n", __func__, mDirectory.c_str());
		int ret = rescan(mDirectory);
		fprintf(stderr, "%s, mFileList.size() = %zd \r\n", __func__, mFileList.size());
		return ret;
	}
	int rescan(string directory_) {
		//fprintf(stderr, "%s, directory_:%s \r\n", __func__, directory_.c_str());

		DIR *dirptr = NULL;
		struct dirent *entry = nullptr;
		if ((dirptr = opendir(directory_.c_str())) == NULL) {
			fprintf(stderr, "%s,%d error! \r\n", __func__, __LINE__);
			return -1;
		}
		entry = readdir(dirptr);
		while (entry != nullptr) {
			if (DT_REG == entry->d_type) {
				//cout << "DIR '" << directory_ << "' FILE: " << entry->d_name << endl;
				string file_name = entry->d_name;
				int suffix_start = file_name.rfind(".");
				if (suffix_start != string::npos && suffix_start < file_name.length()) {
					string suffix_ = file_name.substr(suffix_start + 1, file_name.length());
					//cout << file_name << "/" << suffix_ << endl;
					//std::transform(suffix_.begin(), suffix_.end(), suffix_.begin(), [](unsigned char c) { return std::tolower(c); });
					std::transform(suffix_.begin(), suffix_.end(), suffix_.begin(), ::tolower);
					if (suffix_ == "bmp" || suffix_ == "jpg" || suffix_ == "png") {
						mFileList.push_back(directory_ + "/" + file_name);
					}
				}
			}
			else if(DT_DIR == entry->d_type) {
				string dir_name_ = entry->d_name;
				if (dir_name_ != "." && dir_name_ != "..") {
					string dir__ = directory_ + "/" + entry->d_name;
					rescan(dir__);
				}
			}
			entry = readdir(dirptr);
		}
		closedir(dirptr);
		return 0;
	}
	string nextFile() {
		if (mFileList.size() <= 0) {
			return string();
		}
		mFileIndex++;
		if (mFileIndex >= mFileList.size()) {
			mFileIndex = 0;
		}
		fprintf(stderr, "mFileIndex:%d \r\n", mFileIndex);
		string r = mFileList[mFileIndex];
		return r;
	}
	string prevFile() {
		if (mFileList.size() <= 0) {
			return string();
		}
		if(mFileIndex < 0) { // in case the first time, use last item !
			mFileIndex = mFileList.size() -1;
		} else {
			mFileIndex--;
			if(mFileIndex < 0) {
				mFileIndex = mFileList.size() -1;
			}
		}
		fprintf(stderr, "mFileIndex:%d \r\n", mFileIndex);
		string r = mFileList[mFileIndex];
		return r;
	}
	string currentFile() {
		if (mFileList.size() <= 0) {
			return string();
		}
		if(mFileIndex < 0) { // in case the first time , use index = 0
			mFileIndex = 0;
		} 
		fprintf(stderr, "mFileIndex:%d \r\n", mFileIndex);
		string r = mFileList[mFileIndex];
		return r;
	}
	void resetFileIndex() {
		mFileIndex = -1;
	}
};

/////////////////////////////////////
TestReader::TestReader(int width, int height, int format, int maxImages, uint64_t usage) {
	fprintf(stderr, "%s,%d ...\r\n", __func__, __LINE__);

	// setup AImageReader_ImageListener
	context = (void *)this; onImageAvailable = s_TestReader_AImageReader_ImageCallback;

	AImageReader *reader_ = nullptr;
	//media_status_t ret = AImageReader_new(width, height, format, maxImages, &reader_);
	media_status_t ret = AImageReader_newWithUsage(width, height, format, usage, maxImages, &reader_);
	if (ret == AMEDIA_OK && reader_ != nullptr) {
		mReader = reader_;
		AImageReader_setImageListener(mReader, this);
		fprintf(stderr, "%s,%d ok! \r\n", __func__, __LINE__);
	}
	else {
		fprintf(stderr, "%s,%d failed! \r\n", __func__, __LINE__);
	}
}
TestReader::~TestReader() {
	fprintf(stderr, "%s,%d destroying ... \r\n", __func__, __LINE__);
	if (mReader != nullptr) {
		// wait onImageAvailable to be executed !!! or if there's existing Image , will result error !
		// close: parent AImageReader closed without releasing image 0x7967bf7400

		{
			std::unique_lock<std::mutex> caller_lock(caller_mutex_);
			destroyed = 1;
			
			AImageReader_setImageListener(mReader, nullptr);
			
			while(cb_is_ongoing) {
				fprintf(stderr, "cb_is_ongoing == 1 wait ...\r\n");
				caller_cv_.wait(caller_lock);
			}
			AImage *image_ = nullptr;
			media_status_t ret = AImageReader_acquireLatestImage(mReader, &image_);
			while (ret == AMEDIA_OK && image_ != nullptr) {
				fprintf(stderr, "still have image, delete ...\r\n");
				AImage_delete(image_);
				ret = AImageReader_acquireLatestImage(mReader, &image_);
			}
		}
		AImageReader_delete(mReader);
	}
	fprintf(stderr, "%s,%d destroyed. \r\n", __func__, __LINE__);
}
ANativeWindow *TestReader::getANativeWindow() {
	if (mReader != nullptr) {
		int ret;
		ANativeWindow *window_ = nullptr;
		ret = AImageReader_getWindow(mReader, &window_);
		if (ret == AMEDIA_OK && window_ != nullptr) {
			return window_;
		}
	}
	fprintf(stderr, "%s,%d return nullptr \r\n", __func__, __LINE__);
	return nullptr;
}

void TestReader::notify_cb_ongoing(int ongoing_) {
	std::unique_lock<std::mutex> caller_lock(caller_mutex_);
	cb_is_ongoing = ongoing_;
	caller_cv_.notify_one();
}

// this waill wakeup waitData !
void TestReader::incFrameNumber() {
	mFrameIndex++;
}
void TestReader::onImageAvailableCallback(AImageReader *reader) {
	std::unique_lock<std::mutex> caller_lock(caller_mutex_);
	if (destroyed) {
		fprintf(stderr, "%s,%d destroyed skip !!! \r\n", __func__, __LINE__);
		return ;
	}
	fprintf(stderr, "TestReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameIndex, pthread_self());

	cb_is_ongoing = 1; // notify_cb_ongoing(1);

	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		int32_t img_width_ = 0;
		int32_t img_height_ = 0;
		int32_t img_format_ = 0;
		int64_t img_timestampNs_ = 0;
		int32_t img_numPlanes_ = 0;
		AImage_getWidth(image_, &img_width_);
		AImage_getHeight(image_, &img_height_);
		AImage_getFormat(image_, &img_format_);
		AImage_getTimestamp(image_, &img_timestampNs_);
		AImage_getNumberOfPlanes(image_, &img_numPlanes_);
		fprintf(stderr, "  TestReader image:%04dx%04d format:%08x numPlanes:%d \r\n",
			img_width_, img_height_, img_format_, img_numPlanes_);
		for(int planeIndex = 0;planeIndex<img_numPlanes_;planeIndex++) {
			int32_t pixelStride_ = 0;
			int32_t rowStride_ = 0;
			uint8_t * data_ = nullptr;
			int dataLength_ = 0;
			ret = AImage_getPlanePixelStride(image_, planeIndex, &pixelStride_);
				if (ret != AMEDIA_OK) { fprintf(stderr, "    AImage_getPlanePixelStride failed \r\n"); }
			ret = AImage_getPlaneRowStride(image_, planeIndex, &rowStride_);
				if (ret != AMEDIA_OK) { fprintf(stderr, "    AImage_getPlaneRowStride failed \r\n"); }
			ret = AImage_getPlaneData(image_, planeIndex, &data_, &dataLength_);
				if (ret != AMEDIA_OK) { fprintf(stderr, "    AImage_getPlaneData failed \r\n"); }
			fprintf(stderr, "    TestReader,  plane:%01d pixelStride:%05d rowStride:%05d dataLength:%d \r\n",
				planeIndex, pixelStride_, rowStride_, dataLength_);
		}
	
		AImage_delete(image_);
	}
	incFrameNumber();

	cb_is_ongoing = 0; // notify_cb_ongoing(0);
	caller_cv_.notify_one();

}

// wait until mFrameIndex >= frameIndex !
int TestReader::waitFrame(long frameIndex, long timeout_ms) {
	fprintf(stderr, "wait frame expect:%ld / current: %ld timeout:%lu tid:%lu \r\n",
		frameIndex, mFrameIndex, timeout_ms, pthread_self());
	long time_ms_cnt = 0;
	while (mFrameIndex < frameIndex) {
		usleep(1*1000);
		if (timeout_ms > 0) {
			time_ms_cnt++;
			if (time_ms_cnt >= timeout_ms) {
				fprintf(stderr, "wait frame timeout! \r\n");
				break;
			}
		}
	}
	fprintf(stderr, "%s done! \r\n", __func__);
	return 0;
}

/*static*/ void TestReader::s_TestReader_AImageReader_ImageCallback(void* context, AImageReader* reader) {
	if (context != nullptr) {
		TestReader *testReader = (TestReader*)context;
		testReader->onImageAvailableCallback(reader);
	}
}

/////////////////////////////////////

class BacklightDataReader: public TestReader {
public:
	FootballBlController *bl_controller__ = nullptr;
	BacklightDataReader(FootballBlController *bl_controller, int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages), bl_controller__(bl_controller) {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~BacklightDataReader() {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual void onImageAvailableCallback(AImageReader *reader) {
		//TestReader::onImageAvailableCallback(reader);
		
		fprintf(stderr, "BacklightDataReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameIndex, pthread_self());
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			
			AImageData *imageData = getAImageData_from_AImage(image_);
			AImage_delete(image_);
			if (imageData != nullptr) {
				imageData->print_as_uint32();

				if (bl_controller__ != nullptr) {
					for(int y=0;y<imageData->height;y++) {
						for(int x=0;x<imageData->width;x++) {
							uint32_t color_ =*((uint32_t *) (imageData->mPlanes[0].data_
								+ y*imageData->mPlanes[0].rowStride_
								+ x*imageData->mPlanes[0].pixelStride_));
							bl_controller__->backlightSetData(x, y, color_);
						}
					}
					bl_controller__->backlightCommit();
				}

				delete imageData;
			}
		}
		incFrameNumber();

	}

};
class FinalImageReader : public TestReader {
public:
	FinalImageReader(int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages) {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~FinalImageReader() {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual void onImageAvailableCallback(AImageReader *reader) {
		//TestReader::onImageAvailableCallback(reader);
		
		fprintf(stderr, "FinalImageReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameIndex, pthread_self());
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			AImage_delete(image_);
		}
		incFrameNumber();

	}

};

/////////////////////////////////////

// FootballPPTester
FootballPPTester::FootballPPTester(FootballBlController *bl_controller, int source_type_):
	mFootballBlController(bl_controller), source_type(source_type_) {
	fprintf(stderr, "FootballPPTester::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

	virtual_display_source_ready = 0;

	mTargetWidth = 1080;
	mTargetHeight = 1920; // 2340; // 1920;
	mBlDataWidth = 15;
	mBlDataHeight = 31;

	int width = mTargetWidth;
	int height = mTargetHeight;

	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

	int type_ = FootballPPFactory::PP_CPU;
	type_ = FootballPPFactory::PP_VK;
	mFootballPP = FootballPPFactory::createFootballPP(type_);

	// init a session
	int ret = 0;
	
	//	  AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM 		  = 1,
	ret = ANativeHooSurface_create(FootballPPTester_special_SURFACE_NAME, width, height, 0x01, 0, &mHooSurface);
	if (ret != 0 || mHooSurface == nullptr) {
		fprintf(stderr, "%s,%d error! \r\n", __func__, __LINE__);
	}
	if(mHooSurface != nullptr) {
	   ANativeWindow *surface_window_ = nullptr;
	   ANativeHooSurface_getWindow(mHooSurface, &surface_window_);
	   if (surface_window_ != nullptr) {
		   mSession.final_image = surface_window_;
	   }
	   else {
		   ANativeHooSurface_destroy(mHooSurface);
		   mHooSurface = nullptr;
	   }
	}
	
	if (mSession.final_image == nullptr) {
		mFinalImageReader = new FinalImageReader(width, height, AIMAGE_FORMAT_RGBA_8888);
		mSession.final_image = mFinalImageReader->getANativeWindow();
	}
	if (mSession.final_image == nullptr) {
		fprintf(stderr, "### %s,%d no target final image setup! ### \r\n", __func__, __LINE__);
		abort();
	}
	
	mBlDataReader = new BacklightDataReader(bl_controller, mBlDataWidth, mBlDataHeight, AIMAGE_FORMAT_RGBA_8888);
	mSession.backlight_data = mBlDataReader->getANativeWindow();

	// build session
	ret = mFootballPP->buildSession(&mSession, &mSessionId);
	fprintf(stderr, "buildSession ret = %d id = %d \r\n", ret, mSessionId);

	mFileManager_ = new FileManager_(FootballPPTester_FileManager_DEFAULT_PATH);
	string file_ = mFileManager_->nextFile();
	fprintf(stderr, "file_:%s \r\n", file_.c_str());
	mFileManager_->resetFileIndex();

}
FootballPPTester::~FootballPPTester() {
	if (mNativeHooDisplay != nullptr) {  // must first destroy display source !!!
		ANativeHooDisplay_destroy(mNativeHooDisplay);
		mNativeHooDisplay = nullptr;
	}

	// destroy session
	if (mSessionId >= 0) {
		mFootballPP->closeSession(mSessionId);
	}
	if (mFinalImageReader != nullptr) {
		delete mFinalImageReader;
	}
	if (mHooSurface != nullptr) {
		ANativeHooSurface_destroy(mHooSurface);
	}
	delete mBlDataReader;

	if (mFileManager_ != nullptr) {
		delete mFileManager_;
	}

	// destroy PP core
	if (mFootballPP != nullptr) {
		delete mFootballPP;
	}
	fprintf(stderr, "FootballPPTester::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
}

//
int FootballPPTester::directorySet(const char *dir_) {
	if (mFileManager_ != nullptr) {
		return mFileManager_->setDirectory(dir_);
	}
	return 0;
}
const char *FootballPPTester::directoryGet() {
	if (mFileManager_ != nullptr) {
		return mFileManager_->getDirectory();
	}
	return "";
}

int FootballPPTester::directoryRescan() {
	if (mFileManager_ != nullptr) {
		return mFileManager_->rescan();
	}
	return 0;
}
int FootballPPTester::process_with_file(const char *filename_, int have_algo) {
	if (source_type != SOURCE_FILE) {
		fprintf(stderr, "source type is not FILE !!! \r\n");
		return 0;
	}
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

#undef TEST_FRAME_NUM
#define TEST_FRAME_NUM 1 // (50*10)

	if (mSessionId >= 0) {
		SessionParameter parameter;
		parameter.have_algo = have_algo;
		mFootballPP->setSessionParameter(mSessionId, &parameter);
	}

	for(int frame_no = 0; frame_no < TEST_FRAME_NUM; frame_no++) {
		mFrameIndex++;
		
		fprintf(stderr, "++++++++ producing frame:%ld \r\n", mFrameIndex);
		//////////////////////////////// fill input_window
		{
			ANativeWindow *inputWindow = mSession.input_window;
			//fill_ANativeWindow_with_color(inputWindow, 0xff00ff00);

			StbImage_ *stb_image_ = new StbImage_(filename_, mTargetWidth, mTargetHeight);
			fill_ANativeWindow_with_StbImage(inputWindow, stb_image_);
			delete stb_image_;
		}
		//////////////////////////////// fill done !!!
	
		fprintf(stderr, "frame:%ld produced !\r\n", mFrameIndex);
	
		// wait
		if (mFinalImageReader != nullptr) {
			mFinalImageReader->waitFrame(mFrameIndex, 5*1000);
		}
		
		// wait 
		mBlDataReader->waitFrame(mFrameIndex, 5*1000);
	
		fprintf(stderr, "--------- frame:%ld done \r\n", mFrameIndex);
	}
	return 0;
}

void FootballPPTester::process_next_file(int num_of_file) {
	for(int i=0;i<num_of_file;i++) {
		string file_ = mFileManager_->nextFile();
		if (file_.empty()) { fprintf(stderr, "filename empty! \r\n");return ;}
		fprintf(stderr, "******************* num_of_file: %d next file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str());
	}
}

void FootballPPTester::process_prev_file(int num_of_file) {
	for(int i=0;i<num_of_file;i++) {
		string file_ = mFileManager_->prevFile();
		if (file_.empty()) { fprintf(stderr, "filename empty! \r\n");return ;}
		fprintf(stderr, "******************* num_of_file: %d prev file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str());
	}
}
void FootballPPTester::process_current_file(int num_of_times, int have_algo) {
	for(int i=0;i<num_of_times;i++) {
		string file_ = mFileManager_->currentFile();
		if (file_.empty()) { fprintf(stderr, "filename empty! \r\n");return ;}
		fprintf(stderr, "******************* num_of_file: %d current file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str(), have_algo);
	}
}

//
// 0 close
// 1 open
// 2 toggle
int FootballPPTester::virtual_display_source_setup(int flags) {
	if (source_type != SOURCE_DISPLAY) {
		fprintf(stderr, "source type is not virtual display !!! \r\n");
		return 0;
	}
	if (virtual_display_source_ready == 0 && (flags == 1 || flags == 2)) {
		virtual_display_source_ready = 1;
		ANativeWindow *inputWindow = mSession.input_window;
	
		ANativeHooDisplay *display_ = nullptr;
		int ret;
		fprintf(stderr, "session input size: %4d x %4d \r\n", mSession.width, mSession.height);
		ret = ANativeHooDisplay_create(FootballPPTester_special_DISPLAY_NAME,
			mSession.width, mSession.height, inputWindow, &display_);
		fprintf(stderr, "ANativeHooDisplay_create ret=%d \r\n", ret);
		if (ret == 0 && display_ != nullptr) {
			mNativeHooDisplay = display_;
		}

	} else if (virtual_display_source_ready && (flags == 0 || flags == 2)) {
		virtual_display_source_ready = 0;
		if (mNativeHooDisplay != nullptr) {
			ANativeHooDisplay_destroy(mNativeHooDisplay);
			mNativeHooDisplay = nullptr;
		}
	}

	return 0;
}

//
void FootballPPTester::print() {
	mFootballPP->print(mSessionId);
}


// initial test
void FootballPPTester::test() {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
#define TEST_SESSION_NUM (1)

	for(int i=0;i<TEST_SESSION_NUM;i++) {
		fprintf(stderr, "-------------------------- test session:%d \r\n", i);
		int ret = 0;
		int session_id = -1;
		FootSession session;
		TestReader *final_image_reader = nullptr;
		ANativeHooSurface *hooSurface = nullptr;
		TestReader *bl_data_reader = nullptr;

		//    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM           = 1,
		ret = ANativeHooSurface_create("", 1080, 1920, 0x01, 0, &hooSurface);
		if (ret != 0 || hooSurface == nullptr) {
			fprintf(stderr, "%s,%d error! \r\n", __func__, __LINE__);
			final_image_reader = new FinalImageReader(1080, 1920, AIMAGE_FORMAT_RGBA_8888);
		}
		
		if(hooSurface != nullptr) {
		   ANativeWindow *surface_window_ = nullptr;
		   ANativeHooSurface_getWindow(hooSurface, &surface_window_);
		   if (surface_window_ != nullptr) {
			   session.final_image = surface_window_;
		   }
		   else {
			   ANativeHooSurface_destroy(hooSurface);
		   }
	   	}
		if (session.final_image == nullptr && final_image_reader != nullptr) {
			session.final_image = final_image_reader->getANativeWindow();
		}
		if (session.final_image == nullptr) {
			fprintf(stderr, "%s,%d no target final image setup! \r\n", __func__, __LINE__);
			return ;
		}

		bl_data_reader = new BacklightDataReader(nullptr, 15, 31, AIMAGE_FORMAT_RGBA_8888);
		session.backlight_data = bl_data_reader->getANativeWindow();
		ret = mFootballPP->buildSession(&session, &session_id);
		fprintf(stderr, "buildSession ret = %d id = %d \r\n", ret, session_id);

		if (ret == 0 && session_id >= 0) {
#undef TEST_FRAME_NUM
#define TEST_FRAME_NUM (50*10)
			long frame_index = 0;
			for(int frame_no = 0; frame_no < TEST_FRAME_NUM; frame_no++) {
				frame_index++;
				fprintf(stderr, "++++++++ producing frame:%ld \r\n", frame_index);
				//////////////////////////////// fill input_window
				{
					ANativeWindow *inputWindow = session.input_window;
					ANativeWindow_acquire(inputWindow);
					
					int32_t width_ = ANativeWindow_getWidth(inputWindow);
					int32_t height_ = ANativeWindow_getHeight(inputWindow);
					int32_t format_ = ANativeWindow_getFormat(inputWindow);
					fprintf(stderr, "fill window size:%04dx%04d format:0x%08x \r\n", width_, height_, format_);

					ANativeWindow_Buffer lockBuffer;
					if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
						fprintf(stderr, "    lockBuffer size:%04dx%04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
						uint32_t *pixel = (uint32_t *)lockBuffer.bits;

					#if 1
						for(int line = 0;line<lockBuffer.height;line++, pixel += lockBuffer.stride) {
							memset((void*)pixel, 0xff, lockBuffer.stride*4);
						}
					#endif

						ANativeWindow_unlockAndPost(inputWindow);
					}
					ANativeWindow_release(inputWindow);
				}
				//////////////////////////////// fill done !!!

				fprintf(stderr, "frame:%ld produced !\r\n", frame_index);

				// wait
				if (final_image_reader != nullptr) {
					final_image_reader->waitFrame(frame_index, 0);
				}
				
				// wait 
				bl_data_reader->waitFrame(frame_index, 0);

				fprintf(stderr, "--------- frame:%ld done \r\n", frame_index);
			}

			mFootballPP->closeSession(session_id);
		}

		if (final_image_reader != nullptr) {
			delete final_image_reader;
		}
		if (hooSurface != nullptr) {
			ANativeHooSurface_destroy(hooSurface);
		}
		delete bl_data_reader;
	}

}

};

