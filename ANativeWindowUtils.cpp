#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>

#include "ANativeWindowUtils.h"

namespace football {

#define ALIGN(size, align) ((size + align - 1) & (~(align - 1)))

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
	fprintf(stderr, "  image:%04d x %04d format:%08x numPlanes:%d \r\n",
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
		fprintf(stderr, "    plane:%01d pixelStride:%05d rowStride:%05d dataLength:%d \r\n",
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

	fprintf(stderr, "%s size : %4d x %4d format:0x%08x \r\n", __func__, width, height, format);
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
			fprintf(stderr, "%4d ", color_);
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				fprintf(stderr, "\r\n");
			}
		}
		if (line_end == 0) {
			fprintf(stderr, "\r\n");
		}
	}
}


AImageData *getAImageData_from_AImage(AImage *image_) {
	fprintf(stderr, "> %s: \r\n", __func__);
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
	fprintf(stderr, "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);

	fprintf(stderr, "fill color_ : 0x%08x \r\n", color_);
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		fprintf(stderr, "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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
			fprintf(stderr, "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		fprintf(stderr, "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}

	ANativeWindow_release(inputWindow);
	return 0;
}

int fill_ANativeWindow_with_AImageData(ANativeWindow *window_, AImageData *image_data) {
	if (window_ == nullptr || image_data == nullptr) { 
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	if (image_data->numPlanes > 1) {
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
	fprintf(stderr, "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
#if 0
	if (width_ != image_data->width || height_ != image_data->height || format_ != image_data->format) {
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
#endif
	fprintf(stderr, "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	fprintf(stderr, "  with data size:%04d x %04d format:0x%08x \r\n",
		image_data->width, image_data->height, image_data->format);
	fprintf(stderr, "  pixelStride_:%d rowStride_:%d \r\n", image_data->mPlanes[0].pixelStride_, image_data->mPlanes[0].rowStride_);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		fprintf(stderr, "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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
			fprintf(stderr, "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		fprintf(stderr, "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}
	ANativeWindow_release(inputWindow);
	return 0;;
}
int fill_ANativeWindow_with_buff(ANativeWindow *window_, 
	uint8_t *buf, int buf_width, int buf_height, int rowStride_, int pixelStride_) {

	if (window_ == nullptr || buf == nullptr) { 
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	fprintf(stderr, "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
#if 0
	if (width_ != image_data->width || height_ != image_data->height || format_ != image_data->format) {
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1;
	}
#endif
	fprintf(stderr, "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	fprintf(stderr, "  with buf size:%04d x %04d \r\n",
		buf_width, buf_height);
	fprintf(stderr, "  pixelStride_:%d rowStride_:%d \r\n", pixelStride_, rowStride_);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		fprintf(stderr, "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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
			fprintf(stderr, "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		fprintf(stderr, "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}
	ANativeWindow_release(inputWindow);
	return 0;;
}

int fill_ANativeWindow_with_StbImage(ANativeWindow *window_, StbImage_ *stbImage) {
	if (window_ == nullptr || stbImage == nullptr) { 
		fprintf(stderr, "%s,%d err \r\n", __func__, __LINE__);
		return -1; 
	}
	if (stbImage->tw <= 0 || stbImage->th <= 0 || stbImage->n <= 0) {
		fprintf(stderr, "%s,%d stb image no size err \r\n", __func__, __LINE__);
		return -1;
	}
	fprintf(stderr, "%s \r\n", __func__);

	ANativeWindow * inputWindow = window_;
	int32_t width_ = ANativeWindow_getWidth(inputWindow);
	int32_t height_ = ANativeWindow_getHeight(inputWindow);
	int32_t format_ = ANativeWindow_getFormat(inputWindow);
	if (width_ != stbImage->tw || height_ != stbImage->th) {
		fprintf(stderr, "%s,%d size not match err \r\n", __func__, __LINE__);
		return -1;
	}
	fprintf(stderr, "fill window size:%04d x %04d format:0x%08x \r\n", width_, height_, format_);
	fprintf(stderr, "  with stb image size:%04d x %04d stbImage:%d \r\n",
		stbImage->tw, stbImage->th, stbImage->n);
	
	ANativeWindow_acquire(inputWindow);

	ANativeWindow_Buffer lockBuffer;
	if (ANativeWindow_lock(inputWindow, &lockBuffer, nullptr) == 0) {
		fprintf(stderr, "	 lockBuffer size:%04d x %04d stride:%d \r\n", lockBuffer.width, lockBuffer.height, lockBuffer.stride);
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
			fprintf(stderr, "%s,%d ANativeWindow_unlockAndPost failed: %d! \r\n", __func__, __LINE__, post_ret);
		}
	}
	else {
		fprintf(stderr, "%s,%d ANativeWindow_lock failed! \r\n", __func__, __LINE__);
	}

	ANativeWindow_release(inputWindow);
	return 0;;
}


//




};

