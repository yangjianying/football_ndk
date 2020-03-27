#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>

#include "FootballConfig.h"

#include "ANativeWindowUtils.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#undef __CLASS__
#define __CLASS__ "ANativeWindowUtils"

namespace football {

#define ALIGN(size, align) ((size + align - 1) & (~(align - 1)))

int getAImageAndroidInfo(AImage *image_, AImageAndroidInfo *info) {
	media_status_t ret = AMEDIA_OK;

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

	info->width = img_width_;
	info->height = img_height_;
	info->format = img_format_;
	info->timestampNs = img_timestampNs_;
	info->numPlanes = img_numPlanes_;
	for(int planeIndex = 0;planeIndex<img_numPlanes_;planeIndex++) {
		int32_t pixelStride_ = 0;
		int32_t rowStride_ = 0;
		uint8_t * data_ = nullptr;
		int dataLength_ = 0;
		ret = AImage_getPlanePixelStride(image_, planeIndex, &pixelStride_);
		if (ret != AMEDIA_OK) { DLOGD( "	 AImage_getPlanePixelStride failed \r\n"); }
		ret = AImage_getPlaneRowStride(image_, planeIndex, &rowStride_);
		if (ret != AMEDIA_OK) { DLOGD( "	 AImage_getPlaneRowStride failed \r\n"); }
		ret = AImage_getPlaneData(image_, planeIndex, &data_, &dataLength_);
		if (ret != AMEDIA_OK) { DLOGD( "	 AImage_getPlaneData failed \r\n"); }
		info->planes[planeIndex].pixelStride_ = pixelStride_;
		info->planes[planeIndex].rowStride_ = rowStride_;
		info->planes[planeIndex].data_ = data_;
		info->planes[planeIndex].dataLength_ = dataLength_;
	}
	return 0;
}
void printAImageAndroidInfo(const char *title, AImageAndroidInfo &info) {
	DLOGD( "%s image:%04dx%04d format:%08x numPlanes:%d \r\n", title == nullptr ? "" : title,
		info.width, info.height, info.format, info.numPlanes);
	for(int planeIndex = 0; planeIndex<info.numPlanes; planeIndex++) {
		DLOGD( "  plane:%01d pixelStride:%05d rowStride:%05d dataLength:%d \r\n",
			planeIndex, 
			info.planes[planeIndex].pixelStride_, info.planes[planeIndex].rowStride_, info.planes[planeIndex].dataLength_);
	}
}


AImageData::AImageData(int32_t w_, int32_t h_, int32_t f_, int32_t plane_num) {
	width = w_; height = h_; format = f_; numPlanes = plane_num;
	assert(format == AIMAGE_FORMAT_RGBA_8888);
	assert(plane_num == 1);

	// format : ABGR
	for(int planeIndex = 0;
		planeIndex<numPlanes && planeIndex < MAX_PLANES;
		planeIndex++) {
		mPlanes[planeIndex].pixelStride_ = 4;
		mPlanes[planeIndex].rowStride_ = ALIGN(width*mPlanes[planeIndex].pixelStride_, 64); // aligned to 64
		mPlanes[planeIndex].dataLength_ = mPlanes[planeIndex].rowStride_ * height;
		int dataLength_ = mPlanes[planeIndex].dataLength_;
		mPlanes[planeIndex].data_ = (uint8_t *)malloc(dataLength_ + 256);
		memset(mPlanes[planeIndex].data_, 0, dataLength_ + 256);
	}
	
}
AImageData::AImageData(AImage *image_) {
	AImage_getWidth(image_, &width);
	AImage_getHeight(image_, &height);
	AImage_getFormat(image_, &format);
	AImage_getTimestamp(image_, &timestampNs);
	AImage_getNumberOfPlanes(image_, &numPlanes);
	DLOGD( "  image:%04d x %04d format:%08x numPlanes:%d \r\n",
		width, height, format, numPlanes);

	for(int planeIndex = 0;
		planeIndex<numPlanes && planeIndex < MAX_PLANES;
		planeIndex++) {
		uint8_t * data_ = nullptr;
		int dataLength_ = 0;
	
		AImage_getPlanePixelStride(image_, planeIndex,
			&mPlanes[planeIndex].pixelStride_);
		AImage_getPlaneRowStride(image_, planeIndex,
			&mPlanes[planeIndex].rowStride_);
		AImage_getPlaneData(image_, planeIndex, 
			&data_, 
			&dataLength_);
		DLOGD( "    plane:%01d pixelStride:%05d rowStride:%05d dataLength:%d \r\n",
			planeIndex,
			mPlanes[planeIndex].pixelStride_,
			mPlanes[planeIndex].rowStride_,
			dataLength_);
		mPlanes[planeIndex].dataLength_ = dataLength_;
		if (dataLength_ > 0) {
			havePlaneData = true;
			mPlanes[planeIndex].data_ = (uint8_t *)malloc(dataLength_ + 256);
			memcpy(mPlanes[planeIndex].data_, data_, dataLength_);
		}
	}

}
AImageData::AImageData(AImageData *another) {
	width = another->width;
	height = another->height;
	format = another->format;
	timestampNs = another->timestampNs;
	numPlanes = another->numPlanes;
	havePlaneData = another->havePlaneData;
	for(int planeIndex = 0;
		planeIndex<numPlanes && planeIndex < MAX_PLANES;
		planeIndex++) {
		mPlanes[planeIndex].pixelStride_ = another->mPlanes[planeIndex].pixelStride_;
		mPlanes[planeIndex].rowStride_ = another->mPlanes[planeIndex].rowStride_;
		mPlanes[planeIndex].dataLength_ = another->mPlanes[planeIndex].dataLength_;
		if (mPlanes[planeIndex].dataLength_ > 0) {
			mPlanes[planeIndex].data_ = (uint8_t *)malloc(mPlanes[planeIndex].dataLength_ + 256);
			memcpy(mPlanes[planeIndex].data_, another->mPlanes[planeIndex].data_, mPlanes[planeIndex].dataLength_);
		}
	}
}
AImageData::~AImageData() {
	for(int32_t i=0;i<numPlanes;i++) {
		if (mPlanes[i].data_ != nullptr) {
			::free(mPlanes[i].data_);
		}
	}
}

int AImageData::getPixelData(int plane, int x, int y, uint32_t *outColor) {
	if (x >= width || y >= height || plane >= numPlanes) {
		return -1;
	}
	PlaneData * planeData = &mPlanes[plane];
	uint8_t *pixel_byte_ptr = planeData->data_ + y*planeData->rowStride_ + x*planeData->pixelStride_;
	uint32_t *pixel_ptr = (uint32_t *)pixel_byte_ptr;
	*outColor = *pixel_ptr;
	return 0;
}
int AImageData::setPixelData(int plane, int x, int y, uint32_t color_) {
	if (x >= width || y >= height || plane >= numPlanes) {
		return -1;
	}
	PlaneData * planeData = &mPlanes[plane];
	uint8_t *pixel_byte_ptr = planeData->data_ + y*planeData->rowStride_ + x*planeData->pixelStride_;
	uint32_t *pixel_ptr = (uint32_t *)pixel_byte_ptr;
	*pixel_ptr = color_;
	return 0;
}
void AImageData::print_as_uint32() {
	assert(format == AIMAGE_FORMAT_RGBA_8888);

	int line_end = 0;
	int line_num = width;

	DLOGD( "%s size : %4d x %4d format:0x%08x \r\n", __func__, width, height, format);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			// format == 1, ABGR
			uint32_t color_ =*((uint32_t *) (mPlanes[0].data_
				+ line*mPlanes[0].rowStride_
				+ col*mPlanes[0].pixelStride_));
			
			line_end = 0;
			DLOGD( "%4d ", color_);
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				DLOGD( "\r\n");
			}
		}
		if (line_end == 0) {
			DLOGD( "\r\n");
		}
	}
}


AImageData *getAImageData_from_AImage(AImage *image_) {
	DLOGD( "> %s: \r\n", __func__);
	AImageData *data_ = new AImageData(image_);
	if (!data_->havePlaneData) {
		delete data_;
		return nullptr;
	}
	return data_;
}

int fill_ANativeWindow_with_color(ANativeWindow *window_, uint32_t color_) {
	if (window_ == nullptr) { return -1; }

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
	DLOGD( "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);

	DLOGD( "fill color_ : 0x%08x \r\n", color_);
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		DLOGD( "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
		uint32_t *pixel = (uint32_t *)lockBuffer.bits;
#if 0
		for(int line = 0;line<lockBuffer.height;line++, pixel += lockBuffer.stride) {
			memset((void*)pixel, 0xff, lockBuffer.stride*4);
		}
#endif
		int line = 0;
		for(;line<lockBuffer.height 
			//&& line < 200
			;line++, pixel += lockBuffer.stride) {
			//memset((void*)pixel, 0xa0, lockBuffer.stride*4);
			for(int col=0;col<lockBuffer.width;col++) {
				pixel[col] = color_;
			}
		}
		int32_t post_ret = ANativeWindow_unlockAndPost(inputWindow);
		if (post_ret != 0) {
			DLOGD( "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		DLOGD( "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}

	ANativeWindow_release(inputWindow);
	return 0;
}

int fill_ANativeWindow_with_AImageData(ANativeWindow *window_, AImageData *image_data) {
	if (window_ == nullptr || image_data == nullptr) { 
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	if (image_data->numPlanes > 1) {
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
	DLOGD( "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
#if 0
	if (width_ != image_data->width || height_ != image_data->height || format_ != image_data->format) {
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
#endif
	DLOGD( "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	DLOGD( "  with data size:%04d x %04d format:0x%08x \r\n",
		image_data->width, image_data->height, image_data->format);
	DLOGD( "  pixelStride_:%d rowStride_:%d \r\n", image_data->mPlanes[0].pixelStride_, image_data->mPlanes[0].rowStride_);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		DLOGD( "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
		uint32_t *pixel = (uint32_t *)lockBuffer.bits;

		int line = 0;
		for(;line<lockBuffer.height 
			&& line < image_data->height
			;line++, pixel += lockBuffer.stride) {
			for(int col=0;col<lockBuffer.width
				&& col < image_data->width;col++) {
				// format == 1, ABGR
				pixel[col] =*((uint32_t *) (image_data->mPlanes[0].data_
					+ line*image_data->mPlanes[0].rowStride_
					+ col*image_data->mPlanes[0].pixelStride_));
			}
		}
		int32_t post_ret = ANativeWindow_unlockAndPost(inputWindow);
		if (post_ret != 0) {
			DLOGD( "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		DLOGD( "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}
	ANativeWindow_release(inputWindow);
	return 0;;
}
int fill_ANativeWindow_with_buff(ANativeWindow *window_, 
	uint8_t *buf, int buf_width, int buf_height, int rowStride_, int pixelStride_) {

	if (window_ == nullptr || buf == nullptr) { 
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	DLOGD( "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
#if 0
	if (width_ != image_data->width || height_ != image_data->height || format_ != image_data->format) {
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
#endif
	DLOGD( "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	DLOGD( "  with buf size:%04d x %04d \r\n",
		buf_width, buf_height);
	DLOGD( "  pixelStride_:%d rowStride_:%d \r\n", pixelStride_, rowStride_);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		DLOGD( "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
		uint32_t *pixel = (uint32_t *)lockBuffer.bits;

		int line = 0;
		for(;line<lockBuffer.height 
			&& line < buf_height
			;line++, pixel += lockBuffer.stride) {
			for(int col=0;col<lockBuffer.width
				&& col < buf_width;col++) {
				// format == 1, ABGR
				pixel[col] =*((uint32_t *) (buf
					+ line*rowStride_
					+ col*pixelStride_));
			}
		}
		int32_t post_ret = ANativeWindow_unlockAndPost(inputWindow);
		if (post_ret != 0) {
			DLOGD( "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		DLOGD( "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}
	ANativeWindow_release(inputWindow);
	return 0;;
}

int fill_ANativeWindow_with_StbImage(ANativeWindow *window_, StbImage_ *stbImage) {
	if (window_ == nullptr || stbImage == nullptr) { 
		DLOGD( "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	if (stbImage->tw <= 0 || stbImage->th <= 0 || stbImage->n <= 0) {
		DLOGD( "%s,%d stb image no size err \r\n", __func__, __LINE__);
		return -1;
	}
	DLOGD( "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
	if (width_ != stbImage->tw || height_ != stbImage->th) {
		DLOGD( "%s,%d size not match err \r\n", __func__, __LINE__);
		return -1;
	}
	DLOGD( "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	DLOGD( "  with stb image size:%04d x %04d stbImage:%d \r\n",
		stbImage->tw, stbImage->th, stbImage->n);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		DLOGD( "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
		uint32_t *pixel = (uint32_t *)lockBuffer.bits;
		uint32_t color__ = 0;
		int line = 0;
		for(;line<lockBuffer.height 
			&& line < stbImage->th
			;line++, pixel += lockBuffer.stride) {
			for(int col=0;col<lockBuffer.width
				&& col < stbImage->tw;col++) {
				stbImage->getColor(col, line, &color__);
 				pixel[col] = 0xff000000 | color__;  // ABGR
			}
		}
		int32_t post_ret = ANativeWindow_unlockAndPost(inputWindow);
		if (post_ret != 0) {
			DLOGD( "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		DLOGD( "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}

	ANativeWindow_release(inputWindow);
	return 0;;
}


//
/**** frankie, this reader should be optimized , when destruct, sometimes will hang up !!! */
/**** frankie, this reader should be optimized , when destruct, sometimes will hang up !!! */
/**** frankie, this reader should be optimized , when destruct, sometimes will hang up !!! */


/*static*/ void TestReader::s_TestReader_AImageReader_ImageCallback(void* context, AImageReader* reader) {
	if (context != nullptr) {
		TestReader *testReader = (TestReader*)context;
		testReader->onImageAvailableCallback(reader);
	}
}
TestReader::TestReader(int width, int height, int format, int maxImages, uint64_t usage) {
	DLOGD( "%s,%d ...\r\n", __func__, __LINE__);

	// setup AImageReader_ImageListener
	context = (void *)this; onImageAvailable = s_TestReader_AImageReader_ImageCallback;

	AImageReader *reader_ = nullptr;
	//media_status_t ret = AImageReader_new(width, height, format, maxImages, &reader_);
	media_status_t ret = AImageReader_newWithUsage(width, height, format, usage, maxImages, &reader_);
	if (ret == AMEDIA_OK && reader_ != nullptr) {
		mReader = reader_;
		AImageReader_setImageListener(mReader, this);
		DLOGD( "%s,%d ok! \r\n", __func__, __LINE__);
	}
	else {
		DLOGD( "%s,%d failed! \r\n", __func__, __LINE__);
	}
}
TestReader::~TestReader() {
	DLOGD( "%s,%d destroying ... \r\n", __func__, __LINE__);
	if (mReader != nullptr) {
		// wait onImageAvailable to be executed !!! or if there's existing Image , will result error !
		// close: parent AImageReader closed without releasing image 0x7967bf7400

		{
			std::unique_lock<std::mutex> caller_lock(caller_mutex_);
			destroyed = 1;
			
			AImageReader_setImageListener(mReader, nullptr);
			
			while(cb_is_ongoing) {
				DLOGD( "cb_is_ongoing == 1 wait ...\r\n");
				caller_cv_.wait(caller_lock);
			}
			AImage *image_ = nullptr;
			media_status_t ret = AImageReader_acquireLatestImage(mReader, &image_);
			while (ret == AMEDIA_OK && image_ != nullptr) {
				DLOGD( "still have image, delete ...\r\n");
				AImage_delete(image_);
				ret = AImageReader_acquireLatestImage(mReader, &image_);
			}
		}
		AImageReader_delete(mReader);
	}
	DLOGD( "%s,%d destroyed. \r\n", __func__, __LINE__);
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
	DLOGD( "%s,%d return nullptr \r\n", __func__, __LINE__);
	return nullptr;
}

void TestReader::notify_cb_ongoing(int ongoing_) {
	std::unique_lock<std::mutex> caller_lock(caller_mutex_);
	cb_is_ongoing = ongoing_;
	caller_cv_.notify_one();
}

void TestReader::onImageAvailableCallback(AImageReader *reader) {
	std::unique_lock<std::mutex> caller_lock(caller_mutex_);
	if (destroyed) {
		DLOGD( "%s,%d destroyed skip 1 !!! \r\n", __func__, __LINE__);
		return ;
	}
	cb_is_ongoing = 1; //notify_cb_ongoing(1);
	caller_cv_.notify_one();

	if (destroyed) {
		DLOGD( "%s,%d destroyed skip 2 !!! \r\n", __func__, __LINE__);
		return ;
	}
	DLOGD( ">>>>>>>>>>>>>> TestReader::%s,%d frame: %ld tid:%lu \r\n", 
		__func__, __LINE__, getFrameIndex(), pthread_self());

	onImageAvailableCallback_2_l(reader);

	incFrameIndex();

	cb_is_ongoing = 0; // notify_cb_ongoing(0);
	caller_cv_.notify_one();

}
void TestReader::onImageAvailableCallback_2_l(AImageReader *reader) { // default only print the AImage's information !
	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		AImageAndroidInfo info;
		getAImageAndroidInfo(image_, &info);

		printAImageAndroidInfo("TestReader default: ",info);
		
		AImage_delete(image_);
	}
}


// wait until mFrameIndex >= frameIndex !
int TestReader::waitFrame(long frameIndex, long timeout_ms) {
	DLOGD( "wait frame expect:%ld / current: %ld timeout:%lu tid:%lu \r\n",
		frameIndex, getFrameIndex(), timeout_ms, pthread_self());

{
	std::unique_lock<std::recursive_mutex> trace_lock(mFrameTraced_mutex_);

	long timeout_cnt = timeout_ms;
	while (mFrameTraced < frameIndex && timeout_cnt-- > 0) {
		/*
		template <class Rep, class Period>
		  cv_status wait_for (unique_lock<mutex>& lck, const chrono::duration<Rep,Period>& rel_time);
		*/
		if (mFrameTraced_cv_.wait_for(trace_lock, std::chrono::milliseconds(1))
			!= std::cv_status::timeout) {
		}
	}
	if (mFrameTraced < frameIndex) {
		if (timeout_ms > 0) {
			DLOGD( "wait timeout ! \r\n");
		}
		return 0;
	}
}
	DLOGD( "%s done! \r\n", __func__);
	return 1;
}

HardwareBufferReader::HardwareBufferReader(CB *cb_, int width, int height, int format, int maxImages, 
	uint64_t usage
	, uint32_t flags)
	:TestReader(width, height, format, maxImages, usage)
	, pCB_(cb_)
	, mFlags(flags)
{
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
HardwareBufferReader::~HardwareBufferReader() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}

void HardwareBufferReader::onImageAvailableCallback_2_l(AImageReader *reader) {
	DLOGD( ">>>HardwareBufferReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameTraced, pthread_self());

	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		// here read using CPU will failed !!!
		if (pCB_ != nullptr) {
			if (mFlags&PROC_IMAGE) {
				pCB_->on_process_image(image_);
			}
			else if (mFlags&PROC_HARDWARE_BUFFER) {
				AHardwareBuffer *hardware_buffer_ = nullptr;
				ret = AImage_getHardwareBuffer(image_, &hardware_buffer_);
				if (ret == AMEDIA_OK && hardware_buffer_ != nullptr) {
					pCB_->on_process_frame(hardware_buffer_);
				}
				else {
					DLOGD( "AImage_getHardwareBuffer failed ! \r\n");
				}
			}
		}
		AImage_delete(image_);
	}
}


DisplayWindow_::DisplayWindow_(const char *name, uint32_t width, uint32_t height, uint32_t format) {
	ANativeHooSurface *surface_ = nullptr;
	int ret = ANativeHooSurface_create(name, width, height, format, 
		ANativeHoo_ISurfaceComposerClient_eHidden
		| ANativeHoo_ISurfaceComposerClient_eOpaque
		, 
		&surface_);
	if (ret != 0 || surface_ == nullptr) {
		DLOGD( "%s,%d error! \r\n", __func__, __LINE__);
	}
	impl = static_cast<void *>(surface_);
	getANativeWindow();
}
DisplayWindow_::~DisplayWindow_() {
	ANativeHooSurface *surface_ = static_cast<ANativeHooSurface *>(impl);
	if(surface_ != nullptr) {
		ANativeHooSurface_destroy(surface_);
		impl = nullptr;
	}
}
ANativeWindow *DisplayWindow_::getANativeWindow() {
	ANativeHooSurface *surface_ = static_cast<ANativeHooSurface *>(impl);
	ANativeWindow *windows_ = nullptr;
	if(surface_ != nullptr) {
	   ANativeHooSurface_getWindow(surface_, &windows_);
	   if (windows_ == nullptr) {
		   ANativeHooSurface_destroy(surface_);
		   impl = nullptr;
	   }
	}
	return windows_;
}
void DisplayWindow_::fillColor(uint32_t color_) {
	ANativeWindow *window_ = getANativeWindow();
	if (window_ != nullptr) {
		fill_ANativeWindow_with_color(window_, color_);
	}
}
void DisplayWindow_::show() {
	ANativeHooSurface *surface_ = static_cast<ANativeHooSurface *>(impl);
	if (surface_ != nullptr) {
		ANativeHooSurface_show(surface_);
	}
}
void DisplayWindow_::hide() {
	ANativeHooSurface *surface_ = static_cast<ANativeHooSurface *>(impl);
	if (surface_ != nullptr) {
		ANativeHooSurface_hide(surface_);
	}
}
void DisplayWindow_::setPos(int x, int y) {
	ANativeHooSurface *surface_ = static_cast<ANativeHooSurface *>(impl);
	if (surface_ != nullptr) {
		ANativeHooSurface_setPos(surface_, x, y);
	}
}



};

