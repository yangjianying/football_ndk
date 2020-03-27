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

#include "utils/ANativeWindowUtils.h"
#include "utils/StbImage_.h"
#include "utils/foot_utils.h"


#include "standalone/StbImageUtils.h"

#include "FootballPPFactory.h"
#include "FootballPPTester.h"

#undef __CLASS__
#define __CLASS__ "FootballPPTester"

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
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
		setDirectory(directory_);
	}
	~FileManager_() {
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
	}
	int setDirectory(const char *directory_) {
		mDirectory = directory_;
		if (directory_ != nullptr && directory_[strlen(directory_) - 1] == '/') {
			mDirectory = mDirectory.substr(0, strlen(directory_) - 1);
		}
		DLOGD( "%s, mDirectory:%s \r\n", __func__, mDirectory.c_str());
		int ret = rescan();
		DLOGD( "%s, mFileList.size() = %zd \r\n", __func__, mFileList.size());
		return ret;
	}
	const char* getDirectory() {
		return mDirectory.c_str();
	}
	int rescan() {
		mFileList.clear();
		mFileIndex = -1;
		DLOGD( "%s, mDirectory:%s \r\n", __func__, mDirectory.c_str());
		int ret = rescan(mDirectory);
		DLOGD( "%s, mFileList.size() = %zd \r\n", __func__, mFileList.size());
		return ret;
	}
	int rescan(string directory_) {
		//DLOGD( "%s, directory_:%s \r\n", __func__, directory_.c_str());

		DIR *dirptr = NULL;
		struct dirent *entry = nullptr;
		if ((dirptr = opendir(directory_.c_str())) == NULL) {
			DLOGD( "%s,%d error! \r\n", __func__, __LINE__);
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
		DLOGD( "mFileIndex:%d \r\n", mFileIndex);
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
		DLOGD( "mFileIndex:%d \r\n", mFileIndex);
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
		DLOGD( "mFileIndex:%d \r\n", mFileIndex);
		string r = mFileList[mFileIndex];
		return r;
	}
	void resetFileIndex() {
		mFileIndex = -1;
	}
};

/////////////////////////////////////



/////////////////////////////////////

class BacklightDataReader: public TestReader {
public:
	FootballBlController *bl_controller__ = nullptr;
	BacklightDataReader(FootballBlController *bl_controller, int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages), bl_controller__(bl_controller) {
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~BacklightDataReader() {
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
	}
	virtual void onImageAvailableCallback_2_l(AImageReader *reader) override {
		DLOGD( ">>>BacklightDataReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, getFrameIndex(), pthread_self());
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			
			AImageData *imageData = getAImageData_from_AImage(image_);
			AImage_delete(image_);
			if (imageData != nullptr) {
				
				//imageData->print_as_uint32();
			#if 0
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
			#endif

				delete imageData;
			}
		}


	}

};

#define FINAL_SAVE_DIR "/sdcard/data/final_image"
class FinalImageReader : public TestReader {
public:
	FinalImageReader(int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages) {
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~FinalImageReader() {
		DLOGD( "%s,%d \r\n", __func__, __LINE__);
	}
	virtual void onImageAvailableCallback_2_l(AImageReader *reader) override {
		DLOGD( ">>>FinalImageReader::%s,%d frame: %ld tid:%6lu \r\n", __func__, __LINE__, getFrameIndex(), pthread_self());
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			AImageAndroidInfo info;
			getAImageAndroidInfo(image_, &info);
			printAImageAndroidInfo("FinalImageReader : ", info);
			
#if 1
			if (info.format == AIMAGE_FORMAT_RGBA_8888 && info.numPlanes == 1)
			{
				if (football::utils::CheckFile::exist(FINAL_SAVE_DIR)) {
					// /sdcard/data/
					char path_[256] = {0};
					snprintf(path_, 255, FINAL_SAVE_DIR "/frame_%06lu.png", getFrameIndex());
					int n = 4;
					vk___stbi_write_png(path_, info.width, info.height, n, info.planes[0].data_, info.planes[0].rowStride_);
					DLOGD( "saving done! \r\n");
				} else {
					DLOGD( "%s not exist ! \r\n", FINAL_SAVE_DIR);
				}
			}
			else {
				DLOGD( "format not support save into file ! \r\n");
			}
#endif

		
			AImage_delete(image_);
		}
		
	}

};

/////////////////////////////////////

IMPL_SessionInfo_on_frame(FootballPPTester, on_frame_);


// FootballPPTester
FootballPPTester::FootballPPTester(
	FootballBlController *bl_controller, int pp_type_, int pp_sub_type, int source_type_, int sink_type):
	mFootballBlController(bl_controller), 
	mPPType(pp_type_), 
	mPPSessionType(pp_sub_type), 
	source_type(source_type_), 
	mSinkType(sink_type) {

	DLOGD( "FootballPPTester::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

	virtual_display_source_ready = 0;

	mTargetWidth = FootballPPTester_window_width;
	mTargetHeight = FootballPPTester_window_height;
	mBlDataWidth = 15;
	mBlDataHeight = 31;

	int width = mTargetWidth;
	int height = mTargetHeight;

	DLOGD( "%s,%d \r\n", __func__, __LINE__);

	int type_ = FootballPPFactory::PP_CPU;
	type_ = FootballPPFactory::PP_VK;
	type_ = mPPType;
	mFootballPP = FootballPPFactory::createFootballPP(type_);

	// init a session
	mSessionInfo._on_frame_set(
		SessionInfo_on_frame_s_func(on_frame_), this);
	
	//	  AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM 		  = 1,

	ANativeWindow *surface_window_ = nullptr;

	if (mSinkType == SINK_SURFACE)
	{
		std::unique_lock<std::mutex> caller_lock(mHooSurface_mutex_);
		int ret = 0;
/** frankie, note, here if not use ANativeHoo_ISurfaceComposerClient_eOpaque flag, the UI will be transparent , this is abnormal !!! */
		ret = ANativeHooSurface_create(FootballPPTester_special_SURFACE_NAME, width, height, AIMAGE_FORMAT_RGBA_8888, 
			ANativeHoo_ISurfaceComposerClient_eHidden
			| ANativeHoo_ISurfaceComposerClient_eOpaque
			,
			&mHooSurface);
		if (ret != 0 || mHooSurface == nullptr) {
			DLOGD( "%s,%d error! \r\n", __func__, __LINE__);
		}
		if(mHooSurface != nullptr) {
		   ANativeHooSurface_getWindow(mHooSurface, &surface_window_);
		   if (surface_window_ == nullptr) {
			   ANativeHooSurface_destroy(mHooSurface);
			   mHooSurface = nullptr;
		   }
		}
	}
	mSessionInfo.final_image = surface_window_;

	if (mSinkType == SINK_IMAGE_READER || mSessionInfo.final_image == nullptr) {
/** when render into cpu reader, the UI will be abnormal , as its transparency !!! */
		mFinalImageReader = new FinalImageReader(width, height, AIMAGE_FORMAT_RGBA_8888);
		mSessionInfo.final_image = mFinalImageReader->getANativeWindow();
	}
	if (mSessionInfo.final_image == nullptr) {
		DLOGD( "### %s,%d no target final image setup! ### \r\n", __func__, __LINE__);
		abort();
	}
	
	mBlDataReader = new BacklightDataReader(bl_controller, mBlDataWidth, mBlDataHeight, AIMAGE_FORMAT_RGBA_8888);
	mSessionInfo.backlight_data = mBlDataReader->getANativeWindow();

	// build session
{
	int ret = mFootballPP->buildSession(mPPSessionType, mSessionInfo, &mSessionId);
	DLOGD( "buildSession ret = %d id = %d \r\n", ret, mSessionId);
}
	if (mFinalImageReader != nullptr) {
		mFinalImageReader->resetFrameIndex(mFrameIndex);
	}
	mFileManager_ = new FileManager_(FootballPPTester_FileManager_DEFAULT_PATH);
	string file_ = mFileManager_->nextFile();
	DLOGD( "file_:%s \r\n", file_.c_str());
	mFileManager_->resetFileIndex();

}
FootballPPTester::~FootballPPTester() {

	// destroy session first, this stop the render into reader/surface !!
	if (mSessionId >= 0) {
		mFootballPP->closeSession(mSessionId);
	}

	if (mNativeHooDisplay != nullptr) {  // must first destroy display source !!!
		ANativeHooDisplay_destroy(mNativeHooDisplay);
		mNativeHooDisplay = nullptr;
	}
	if (mFinalImageReader != nullptr) {
		delete mFinalImageReader;
	}

	{
		std::unique_lock<std::mutex> caller_lock(mHooSurface_mutex_);
		if (mHooSurface != nullptr) {
			ANativeHooSurface_destroy(mHooSurface);
			mHooSurface = nullptr;
		}
	}

	delete mBlDataReader;

	if (mFileManager_ != nullptr) {
		delete mFileManager_;
	}

	// destroy PP core
	if (mFootballPP != nullptr) {
		delete mFootballPP;
	}
	DLOGD( "FootballPPTester::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
}
void FootballPPTester::on_frame_() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	{
		std::unique_lock<std::mutex> caller_lock(mHooSurface_mutex_);
#define FIRST_FRAME_NUM 1  // (2)

		if (first_few_frame_comming < FIRST_FRAME_NUM) {
			first_few_frame_comming++;
			if (first_few_frame_comming >= FIRST_FRAME_NUM) {
				// until now, show the surface on the screen !!!
/** frankie, note, it's strange that even the surface is hiden , the screen still show black ! */
				if (mHooSurface != nullptr) {
					ANativeHooSurface_show(mHooSurface);
				}
				// after the surface shown, then open the layer/display cross arbitration
			}
		}
	}
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
		DLOGD( "source type is not FILE !!! \r\n");
		return 0;
	}
	DLOGD( "%s,%d \r\n", __func__, __LINE__);

#undef TEST_FRAME_NUM
#define TEST_FRAME_NUM 1 // (50*10)

	if (mSessionId >= 0) {
		SessionParameter parameter;
		parameter.have_algo = have_algo;
		mFootballPP->setSessionParameter(mSessionId, &parameter);
	}

	for(int frame_no = 0; frame_no < TEST_FRAME_NUM; frame_no++) {
		mFrameIndex++;
		
		DLOGD( "++++++++ producing frame:%ld \r\n", mFrameIndex);
		//////////////////////////////// fill input_window
		{
			ANativeWindow *inputWindow = mSessionInfo.input_window;
			//fill_ANativeWindow_with_color(inputWindow, 0xff00ff00);

			StbImage_ *stb_image_ = new StbImage_(filename_, mTargetWidth, mTargetHeight);
			fill_ANativeWindow_with_StbImage(inputWindow, stb_image_);
			delete stb_image_;
		}
		//////////////////////////////// fill done !!!
	
		DLOGD( "frame:%ld produced !\r\n", mFrameIndex);
	
		// wait
		if (mFinalImageReader != nullptr) {
			mFinalImageReader->waitFrame(mFrameIndex, 5*1000);
		}
		// wait 
		mBlDataReader->waitFrame(mFrameIndex, 5*1000);
	
		DLOGD( "--------- frame:%ld done \r\n", mFrameIndex);
	}
	return 0;
}

void FootballPPTester::process_next_file(int num_of_file) {
	for(int i=0;i<num_of_file;i++) {
		string file_ = mFileManager_->nextFile();
		if (file_.empty()) { DLOGD( "filename empty! \r\n");return ;}
		DLOGD( "******************* num_of_file: %d next file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str());
	}
}

void FootballPPTester::process_prev_file(int num_of_file) {
	for(int i=0;i<num_of_file;i++) {
		string file_ = mFileManager_->prevFile();
		if (file_.empty()) { DLOGD( "filename empty! \r\n");return ;}
		DLOGD( "******************* num_of_file: %d prev file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str());
	}
}
void FootballPPTester::process_current_file(int num_of_times, int have_algo) {
	for(int i=0;i<num_of_times;i++) {
		string file_ = mFileManager_->currentFile();
		if (file_.empty()) { DLOGD( "filename empty! \r\n");return ;}
		DLOGD( "******************* num_of_file: %d current file_:%s \r\n", i, file_.c_str());
		process_with_file(file_.c_str(), have_algo);
	}
}

// 0 close
// 1 open
// 2 toggle
int FootballPPTester::virtual_display_source_setup(int flags) {
	if (source_type != SOURCE_DISPLAY) {
		DLOGD( "source type is not virtual display !!! \r\n");
		return 0;
	}
	if (virtual_display_source_ready == 0 && (VD_OP_OPEN == 1 || VD_OP_TOGGLE == 2)) {
		virtual_display_source_ready = 1;
		ANativeWindow *inputWindow = mSessionInfo.input_window;
	
		ANativeHooDisplay *display_ = nullptr;
		int ret;
		DLOGD( "session input size: %4d x %4d \r\n", mSessionInfo.width, mSessionInfo.height);
		ret = ANativeHooDisplay_create(FootballPPTester_special_DISPLAY_NAME,
			mSessionInfo.width, mSessionInfo.height, inputWindow, &display_);
		DLOGD( "ANativeHooDisplay_create ret=%d \r\n", ret);
		if (ret == 0 && display_ != nullptr) {
			mNativeHooDisplay = display_;
		}

	} else if (virtual_display_source_ready && (VD_OP_CLOSE == 0 || VD_OP_TOGGLE == 2)) {
		virtual_display_source_ready = 0;
		if (mNativeHooDisplay != nullptr) {
			ANativeHooDisplay_destroy(mNativeHooDisplay);
			mNativeHooDisplay = nullptr;
		}
	}

	return 0;
}

//
int FootballPPTester::setParameter(SessionParameter *parameter) {
	return mFootballPP->setSessionParameter(mSessionId, parameter);
}

void FootballPPTester::print() {
	mFootballPP->print(mSessionId);
}


// initial test
void FootballPPTester::test() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
#define TEST_SESSION_NUM (1)

	for(int i=0;i<TEST_SESSION_NUM;i++) {
		DLOGD( "-------------------------- test session:%d \r\n", i);
		int ret = 0;
		int session_id = -1;
		SessionInfo session;
		TestReader *final_image_reader = nullptr;
		ANativeHooSurface *hooSurface = nullptr;
		TestReader *bl_data_reader = nullptr;

		//    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM           = 1,
		ret = ANativeHooSurface_create("", 1080, 1920, AIMAGE_FORMAT_RGBA_8888, 0, &hooSurface);
		if (ret != 0 || hooSurface == nullptr) {
			DLOGD( "%s,%d error! \r\n", __func__, __LINE__);
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
			DLOGD( "%s,%d no target final image setup! \r\n", __func__, __LINE__);
			return ;
		}

		bl_data_reader = new BacklightDataReader(nullptr, 15, 31, AIMAGE_FORMAT_RGBA_8888);
		session.backlight_data = bl_data_reader->getANativeWindow();
		ret = mFootballPP->buildSession(0, session, &session_id);
		DLOGD( "buildSession ret = %d id = %d \r\n", ret, session_id);

		if (ret == 0 && session_id >= 0) {
#undef TEST_FRAME_NUM
#define TEST_FRAME_NUM (50*10)
			long frame_index = 0;
			for(int frame_no = 0; frame_no < TEST_FRAME_NUM; frame_no++) {
				frame_index++;
				DLOGD( "++++++++ producing frame:%ld \r\n", frame_index);
				//////////////////////////////// fill input_window
				{
					ANativeWindow *inputWindow = session.input_window;
					ANativeWindow_acquire(inputWindow);
					
					int32_t width_ = ANativeWindow_getWidth(inputWindow);
					int32_t height_ = ANativeWindow_getHeight(inputWindow);
					int32_t format_ = ANativeWindow_getFormat(inputWindow);
					DLOGD( "fill window size:%04dx%04d format:0x%08x \r\n", width_, height_, format_);

					ANativeWindow_Buffer lockBuffer;
					if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
						DLOGD( "    lockBuffer size:%04dx%04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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

				DLOGD( "frame:%ld produced !\r\n", frame_index);

				// wait
				if (final_image_reader != nullptr) {
					final_image_reader->waitFrame(frame_index, 0);
				}
				
				// wait 
				bl_data_reader->waitFrame(frame_index, 0);

				DLOGD( "--------- frame:%ld done \r\n", frame_index);
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

