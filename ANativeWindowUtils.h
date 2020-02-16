#ifndef __ANATIVE_WINDOW_UTILS_H___
#define __ANATIVE_WINDOW_UTILS_H___

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "StbImage_.h"

namespace football {

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
	AImageData(int32_t w_, int32_t h_, int32_t f_ = 0x01, int32_t plane_num = 1);
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


};


#endif

