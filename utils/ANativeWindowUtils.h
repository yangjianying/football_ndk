#ifndef __ANATIVE_WINDOW_UTILS_H___
#define __ANATIVE_WINDOW_UTILS_H___

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "StbImage_.h"

namespace football {

struct AImageAndroidInfoPlane {
	int32_t pixelStride_ = 0;
	int32_t rowStride_ = 0;
	uint8_t * data_ = nullptr;
	int dataLength_ = 0;
};
struct AImageAndroidInfo {
	int32_t width = 0;
	int32_t height = 0;
	int32_t format = 0;
	int64_t timestampNs = 0;
	int32_t numPlanes = 0;
#define AImageAndroidInfo_MAX_PLANES (6)
	AImageAndroidInfoPlane planes[AImageAndroidInfo_MAX_PLANES]{{0}};
	uint32_t getColor32(int x, int y, int p) {
		if (p >= numPlanes) { return 0; }
		AImageAndroidInfoPlane *plane = &planes[p];
		uint32_t *pixel_ptr = (uint32_t *)(plane->data_ + y * plane->rowStride_ + x * plane->pixelStride_);
		return pixel_ptr[0];
	}
};

int getAImageAndroidInfo(AImage *image_, AImageAndroidInfo *info);
void printAImageAndroidInfo(const char *title, AImageAndroidInfo &info);

class AImageData {
public:
	int32_t width = 0;
	int32_t height = 0;
	int32_t format = 0;
	int64_t timestampNs = 0;
#define MAX_PLANES (3)
	int32_t numPlanes = 0;
	bool havePlaneData = false;

	struct PlaneData {
		int32_t pixelStride_ = 0;  // one pixel size in bytes
		int32_t rowStride_ = 0;  // one row size in bytes
		uint8_t * data_ = nullptr;	// format == 1 : ABGR
		int dataLength_ = 0;
	};

	struct PlaneData mPlanes[MAX_PLANES];

	AImageData() {}
	AImageData(int32_t w_, int32_t h_, int32_t f_ = AIMAGE_FORMAT_RGBA_8888, int32_t plane_num = 1);
	AImageData(AImage *image_);
	AImageData(AImageData *another);
	~AImageData();

	int getPixelData(int plane, int x, int y, uint32_t *outColor);
	int setPixelData(int plane, int x, int y, uint32_t color_);

	void print_as_uint32();

};

AImageData *getAImageData_from_AImage(AImage *image_);


int fill_ANativeWindow_with_color(ANativeWindow *window_, uint32_t color_);
int fill_ANativeWindow_with_AImageData(ANativeWindow *window_, AImageData *image_data);
int fill_ANativeWindow_with_buff(ANativeWindow *window_, 
	uint8_t *buf, int buf_width, int buf_height, int rowStride_, int pixelStride_);

int fill_ANativeWindow_with_StbImage(ANativeWindow *window_, StbImage_ *stbImage);


/////
class ImageReaderImageListenerWrapper: public AImageReader_ImageListener {
public:
	static void s_AImageReader_ImageCallback(void* context, AImageReader* reader) {
		if (context != nullptr) {
			ImageReaderImageListenerWrapper *wrapper = (ImageReaderImageListenerWrapper*)context;
			wrapper->onImageAvailableCallback(reader);
		}
	}
	ImageReaderImageListenerWrapper() {
		// setup AImageReader_ImageListener
		context = (void *)this; onImageAvailable = s_AImageReader_ImageCallback;
	}
	virtual ~ImageReaderImageListenerWrapper() {}

	virtual void onImageAvailableCallback(AImageReader *reader) = 0;

};

//////////////////////////////////////////////////////////////

class TestReader: public AImageReader_ImageListener {
public:
	static void s_TestReader_AImageReader_ImageCallback(void* context, AImageReader* reader);

	long mFrameTraced = 0;  // initial frame number is 0 !!!
	//std::mutex mFrameTraced_mutex_;
	std::recursive_mutex mFrameTraced_mutex_;
	//std::condition_variable mFrameTraced_cv_;
	std::condition_variable_any mFrameTraced_cv_;

	AImageReader *mReader = nullptr;

	std::mutex caller_mutex_;
	std::condition_variable caller_cv_;
	int cb_is_ongoing = 0;
	int destroyed = 0;
	void notify_cb_ongoing(int ongoing_);

	TestReader(int width, int height, int format = AIMAGE_FORMAT_RGBA_8888, int maxImages = 3, 
		uint64_t usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN);
	virtual ~TestReader();
	ANativeWindow *getANativeWindow();
	
	void resetFrameIndex(long index_) {
		std::unique_lock<std::recursive_mutex> trace_lock(mFrameTraced_mutex_);
		mFrameTraced = index_;
		mFrameTraced_cv_.notify_one();
	}
	long getFrameIndex() {
		std::unique_lock<std::recursive_mutex> trace_lock(mFrameTraced_mutex_);
		return mFrameTraced;
	}
	// this waill wakeup waitData !
	void incFrameIndex() {
		std::unique_lock<std::recursive_mutex> trace_lock(mFrameTraced_mutex_);
		mFrameTraced++;
		mFrameTraced_cv_.notify_one();
	}
	virtual void onImageAvailableCallback(AImageReader *reader);

	virtual void onImageAvailableCallback_2_l(AImageReader *reader);

	// wait until mFrameIndex >= frameIndex !
	virtual int waitFrame(long frameIndex, long timeout_ms);
};

class HardwareBufferReader: public TestReader {
public:
	enum {
		PROC_HARDWARE_BUFFER = 0x01,
		PROC_IMAGE = 0x02,
	};
	class CB {
	public:
		virtual ~CB() {}
		virtual int on_process_frame(AHardwareBuffer *hardware_buffer) { return -1; };
		virtual int on_process_image(AImage *image_) { return -1; } ;
	};

	CB *pCB_ = nullptr;
	uint32_t mFlags = 0;
	
	HardwareBufferReader(CB *cb_, int width, int height, int format, int maxImages = 3, 
		uint64_t usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
					| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
		, uint32_t flags = PROC_HARDWARE_BUFFER);
	
	virtual ~HardwareBufferReader();

	virtual void onImageAvailableCallback_2_l(AImageReader *reader);


};


class DisplayWindow_ {
public:
	DisplayWindow_(const char *name, uint32_t width, uint32_t height, uint32_t format);
	~DisplayWindow_();
	ANativeWindow *getANativeWindow();
	void fillColor(uint32_t color_);
	void show();
	void hide();
	void setPos(int x, int y);

	void * impl = nullptr;
};

template <typename T>
class MapKeyedInt {
public:
	MapKeyedInt() {}
	~MapKeyedInt() {}
	int getSize() { return static_cast<int>(mMap.size()); }
	int insert(int key_, T t) {
		std::pair<typename std::map<int, T>::iterator, bool> Insert_Pair;
		Insert_Pair = mMap.insert(typename std::pair<int, T>(key_, t));
		if(Insert_Pair.second == true) {
			return 0;
		}
		std::cout<<"Insert Failure"<<std::endl;
		assert(false);
		return -1;
	}
	int remove(int key_) {
		if (mMap.erase(key_) == 1) {
			return 0;
		}
		return -1;
	}
	T getAssert(int key_) {
		for(typename std::map<int, T>::iterator iter = mMap.begin(); iter != mMap.end(); iter++)  {
			if (iter->first == key_) {
				return iter->second;
			}
		}
		assert(false);
	}
	int get(int key_, T *p) {
		for(typename std::map<int, T>::iterator iter = mMap.begin(); iter != mMap.end(); iter++)  {
			if (iter->first == key_) {
				*p = iter->second;
				return 0;
			}
		} 
		return -1;
	}
	void next_begin() {
		next_begin_first = 1;
	}
	int next(int *key_, T *p) {
		if (next_begin_first) {
			next_begin_first = 0;
			
			mCurrentIter = mMap.begin();
			if (key_ != nullptr) {
				*key_ = mCurrentIter->first;
			}
			if (p != nullptr) {
				*p = mCurrentIter->second;
			}
			return 0;
		}
		mCurrentIter++;
		if(mCurrentIter != mMap.end()) {
			if (key_ != nullptr) {
				*key_ = mCurrentIter->first;
			}
			if (p != nullptr) {
				*p = mCurrentIter->second;
			}
			return 0;
		}
		return -1;
	}

	typename std::map<int, T> mMap;
	typename std::map<int, T>::iterator mCurrentIter;
	int next_begin_first = 0;
};


};


#endif

