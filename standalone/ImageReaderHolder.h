#ifndef __IMAGE_READER_HOLDER_H__
#define __IMAGE_READER_HOLDER_H__


class ImageReaderHolder: public AImageReader_ImageListener {
public:
	static void __s_cb(void* context, AImageReader* reader);

	ImageReaderHolder(int width_, int height_, int32_t format_, uint64_t usage_);
	virtual ~ImageReaderHolder();

	ANativeWindow * getWindow();

	void test_fill_pattern();
	void test_fill_from_file(long wait_ms);

	virtual void cb(AImageReader* reader);
	virtual void onPendingImageReady() {}

	AImage *lockPendingImage();
	int unlockPendingImage_andDelete();

	pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_mutex_t lockMutex;
    pthread_cond_t lockCond;
	bool cb_is_ongoing = false;
	bool is_destroying = false;

	AImageReader *mImageReader = nullptr;
	ANativeWindow *mNativeWindow = nullptr;
	AImage *mPendingImage = nullptr;
};


#endif

