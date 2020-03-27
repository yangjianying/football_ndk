#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>


#include <android/log.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <android/hardware_buffer.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <unistd.h>


#include "FootballConfig.h"


#include "ImageReaderHolder.h"

#undef __CLASS__
#define __CLASS__ "ImageReaderHolder"

/*static*/ void ImageReaderHolder::__s_cb(void* context, AImageReader* reader) {
	ImageReaderHolder *holder = (ImageReaderHolder*)context;
	if (holder != nullptr) {
		holder->cb(reader);
	}
}
ImageReaderHolder::ImageReaderHolder(int width_, int height_, int32_t format_, uint64_t usage_) {
	context = this; onImageAvailable = __s_cb;
	
    pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_mutex_init(&lockMutex, NULL);
	pthread_cond_init(&lockCond, NULL);
	
	media_status_t ret = AImageReader_newWithUsage(width_, height_, format_, usage_, 3, &mImageReader);
	if (mImageReader != nullptr && ret == AMEDIA_OK) {
		AImageReader_setImageListener(mImageReader, this);
		ret = AImageReader_getWindow(mImageReader, &mNativeWindow);
	}
}
ImageReaderHolder::~ImageReaderHolder() {
	if (mImageReader != nullptr) {
		//
		pthread_mutex_lock(&lockMutex);
		is_destroying = true;
		while (cb_is_ongoing == true) {
			DLOGD( "%s cb_is_ongoing \r\n", __func__);
	        pthread_cond_wait(&lockCond, &lockMutex);
	    }
		AImageReader_setImageListener(mImageReader, nullptr);
		
		{
			AImage *image_ = nullptr;
			media_status_t ret = AImageReader_acquireLatestImage(mImageReader, &image_);
			while(image_ != nullptr) {
				AImage_delete(image_);
				ret = AImageReader_acquireLatestImage(mImageReader, &image_);
			}
		}

		AImageReader_delete(mImageReader);

		pthread_mutex_unlock(&lockMutex);
	}
}
ANativeWindow * ImageReaderHolder::getWindow() {
	ANativeWindow *aNativeWindow = nullptr;
	media_status_t ret = AImageReader_getWindow(mImageReader, &aNativeWindow);
	if (ret == AMEDIA_OK) {
		return aNativeWindow;
	}
	return nullptr;
}

void ImageReaderHolder::test_fill_pattern() {
	DLOGD( "%s  ...\r\n", __func__);
	
	pthread_mutex_lock(&lockMutex);
	if (is_destroying) {
		DLOGD( "%s is_destroying \r\n", __func__);
		pthread_mutex_unlock(&lockMutex);
		return ;
	}
	pthread_mutex_unlock(&lockMutex);

	ANativeWindow_Buffer lockedBuffer;
	int32_t ret = ANativeWindow_lock(mNativeWindow, &lockedBuffer, nullptr);
	if (ret == 0) {
		DLOGD( "%s ANativeWindow_lock ok: %4dx%4d stride:%d \r\n", 
			__func__, lockedBuffer.width, lockedBuffer.height, lockedBuffer.stride);

		uint32_t *start_ptr = (uint32_t*)lockedBuffer.bits;
		uint32_t *pixel_ptr = nullptr;
		for(int line=0;line<lockedBuffer.height;line++) {
			pixel_ptr = start_ptr + line*lockedBuffer.stride;

			int col=0;
			for(;col<lockedBuffer.width/3;col++) {
				pixel_ptr[col] = 0xff0000ff;  // ABGR
			}
			for(;col<(lockedBuffer.width*2)/3;col++) {
				pixel_ptr[col] = 0xff00ff00;  // ABGR
			}
			for(;col<lockedBuffer.width;col++) {
				pixel_ptr[col] = 0xffff0000;  // ABGR
			}
		}
		ANativeWindow_unlockAndPost(mNativeWindow);
	}
	else {
		DLOGD( "%s ANativeWindow_lock fail: %d \r\n", __func__, ret);
	}
	DLOGD( "%s  done\r\n", __func__);
}
void ImageReaderHolder::test_fill_from_file(long wait_ms) {

}

void ImageReaderHolder::cb(AImageReader* reader) {
	AImage *image_ = nullptr;

	pthread_mutex_lock(&lockMutex);
	DLOGD( "%s  ...\r\n", __func__);
	if (is_destroying) {
		DLOGD( "%s is_destroying \r\n", __func__);
		pthread_mutex_unlock(&lockMutex);
		return ;
	}
	cb_is_ongoing = true;
	pthread_cond_broadcast(&lockCond);
	pthread_mutex_unlock(&lockMutex);

	media_status_t ret = AImageReader_acquireLatestImage(mImageReader, &image_);
	if (image_ != nullptr) {
		//AImage_delete(image_);

		// wait previous image to be processed !!!
		pthread_mutex_lock(&mutex);
	    while (mPendingImage != nullptr) {
	        pthread_cond_wait(&cond, &mutex);
	    }
	    //pthread_mutex_unlock(&mutex);

		// set new image then notify
		//pthread_mutex_lock(&mutex);
		mPendingImage = image_;
		pthread_mutex_unlock(&mutex);

		/*******************************************/
		onPendingImageReady();
		/*******************************************/
	}


	pthread_mutex_lock(&lockMutex);
	cb_is_ongoing = false;
	pthread_cond_broadcast(&lockCond);
	DLOGD( "%s  done\r\n", __func__);
	pthread_mutex_unlock(&lockMutex);
	
}

AImage *ImageReaderHolder::lockPendingImage() {
	pthread_mutex_lock(&mutex);
	DLOGD( "%s  +++ \r\n", __func__);
	return mPendingImage;
}
int ImageReaderHolder::unlockPendingImage_andDelete() {
	AImage_delete(mPendingImage);
	mPendingImage = nullptr;
	pthread_cond_broadcast(&cond);
	DLOGD( "%s  ---\r\n", __func__);
	pthread_mutex_unlock(&mutex);
	return 0;
}


