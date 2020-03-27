


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include "FootballConfig.h"
#include "utils/football_debugger.h"

#include "utils/ANativeWindowUtils.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#include "computedemo1_1.h"
#include "computedemo1_1_Parasite.h"
#include "computedemo1_1_HistogramCompute.h"
#include "computedemo1_1_HistogramGraphics.h"
#include "computedemo1_1_MasiaEO1.h"


// Android log function wrappers
static const char* kTAG = "computedemo1_1";
#include "utils/android_logcat_.h"

#undef __CLASS__
#define __CLASS__ "VulkanExample_Parasite"

namespace computedemo1_1 {

/*static*/ VulkanExample_Parasite * VulkanExample_Parasite::create(VulkanExample *example, int type) {
	DLOGD("%s, type = %d \r\n", __func__, type);
	if (type == 0) {
		return new HistogramGraphics(example);
	}
	else if(type == 1) {
		return new HistogramCompute(example);
	}
	else if(type == 2) {
		return new MasiaEO1(example);
	}
	return nullptr;
}

};


namespace computedemo1_1 {


void IndirectStats__c::print(uint32_t flags) {
	int height = 1;
	int width = 256;
	height = IndirectStats__c_HIST_BIN_SIZE/width;

	int line_end = 0;
	int line_num = 16;

	fprintf(stderr, "%s size : %4d x %4d drawCount : %d \r\n", __func__, width, height, drawCount);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			line_end = 0;

			if (PRINT_HIST&flags) {
				uint32_t color_ =*((uint32_t *) (lodCount + line*width + col*1));
				double percent_ = lodCount_d[line*width + col*1];
				fprintf(stderr, "%6d(%4.1f) ", color_, percent_);
			}
			if (PRINT_ACC_HIST &flags) {
				uint32_t color_ =*((uint32_t *) (accHist + line*width + col*1));
				float percent_ = accHistNormal[line*width + col*1];
				fprintf(stderr, "%6d(%2.3f) ", color_, percent_);
			}
			if (PRINT_AGC_LUT & flags) {
				uint32_t color_ =*((uint32_t *) (agc_lut + line*width + col*1));
				fprintf(stderr, "%6d ", color_);
			}
			if (PRINT_BHE_LUT & flags) {
				uint32_t color_ =*((uint32_t *) (bhe_lut + line*width + col*1));
				fprintf(stderr, "%6d ", color_);
			}
			if (PRINT_MERGED_LUT & flags) {
				uint32_t color_ =*((uint32_t *) (merged_lut + line*width + col*1));
				fprintf(stderr, "%6d ", color_);
			}
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

void IndirectStats__c::print_u32() {
	int height = 1;
	int width = 256;
	height = IndirectStats__c_HIST_BIN_SIZE/width;

	int line_end = 0;
	int line_num = 16;

	fprintf(stderr, "%s size : %4d x %4d drawCount : %d \r\n", __func__, width, height, drawCount);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			
			uint32_t color_ =*((uint32_t *) (lodCount + line*width + col*1));

			line_end = 0;
			fprintf(stderr, "%8x ", color_);
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

class HistogramGrapherImpl1: public HistogramGrapher {
#define X_OFFSET (0) // (50)
#define WIDTH_ (512)
#define HEIGHT_ (256)

public:
	HistogramGrapherImpl1();
	virtual ~ HistogramGrapherImpl1();

	void draw_lock();
	void draw_unlock();
	void draw_clear_l(uint32_t color_);
	void draw_vertical_line_l(int x, int y0, int y1, uint32_t color_);
	void draw_point_l(int x, int y0, uint32_t color_);
	void draw_horizontal_line_l(int x0, int x1, int y0, uint32_t color_) ;

	virtual void setBHE_factor(float f0, float f1) override {
		y0_points[0] = f0;
		y0_points[2] = f1;
	}

	virtual void draw_stats(IndirectStats__c *stats) override;

	double y0_points[3] = {0.3f, 0.5f, 0.8f};

	int mBase_x = X_OFFSET;
	int mWidth = WIDTH_ + mBase_x*2;
	int mHeight = HEIGHT_;
	ANativeHooSurface *mHooSurface = nullptr;
	std::mutex mHooSurface_mutex_;
	ANativeWindow *mWindow_ = nullptr;
	ANativeWindow_Buffer mLockBuffer;
	int locked = 0;

#define DRAW_POINTS_NUM (256)

	uint32_t draw_points[DRAW_POINTS_NUM];

	double draw_percents[DRAW_POINTS_NUM];

	double accHistNormal[DRAW_POINTS_NUM];

	double pointIndex[32];

	uint32_t agc_lut[DRAW_POINTS_NUM];

	uint32_t bhe_lut[DRAW_POINTS_NUM];

	uint32_t merged_lut[DRAW_POINTS_NUM];
};

HistogramGrapherImpl1::HistogramGrapherImpl1() {


	ANativeWindow *surface_window_ = nullptr;
	{
		std::unique_lock<std::mutex> caller_lock(mHooSurface_mutex_);
		int ret = 0;
		ret = ANativeHooSurface_create(FootballPPTester_special_SURFACE_NAME, mWidth, mHeight, AIMAGE_FORMAT_RGBA_8888, 
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
	mWindow_ = surface_window_;

	draw_lock();
	draw_clear_l(0xff202020);
	draw_unlock();

	if (mHooSurface != nullptr) {
		ANativeHooSurface_setPos(mHooSurface, 28, 2340 - 50 - mHeight);
		ANativeHooSurface_show(mHooSurface);
	}

}
HistogramGrapherImpl1::~ HistogramGrapherImpl1() {
	{
		std::unique_lock<std::mutex> caller_lock(mHooSurface_mutex_);
		if (mHooSurface != nullptr) {
			ANativeHooSurface_destroy(mHooSurface);
			mHooSurface = nullptr;
		}
	}
}
void HistogramGrapherImpl1::draw_lock() {
	if (mWindow_ == nullptr) { return ;}
	if (locked) { return ; }

	ANativeWindow_acquire(mWindow_);

	int32_t width_ = ANativeWindow_getWidth(mWindow_);
	int32_t height_ = ANativeWindow_getHeight(mWindow_);
	int32_t format_ = ANativeWindow_getFormat(mWindow_);
	
	DLOGD( "lock window size:%4dx%4d format:0x%8x \r\n", width_, height_, format_); // 1 , ABGR

	if (ANativeWindow_lock(mWindow_, &mLockBuffer, nullptr) == 0) {
		DLOGD( "	 Buffer size:%4dx%4d stride:%d \r\n", mLockBuffer.width, mLockBuffer.height, mLockBuffer.stride);
		locked = 1;
	}
}
void HistogramGrapherImpl1::draw_unlock() {
	if (mWindow_ == nullptr) { return ;}
	if (locked == 0) { return ; }
	
	if (locked) {
		locked = 0;
		DLOGD( "%s \r\n", __func__);
		ANativeWindow_unlockAndPost(mWindow_);
		ANativeWindow_release(mWindow_);
	}
}
void HistogramGrapherImpl1::draw_clear_l(uint32_t color_) {
	if (mWindow_ == nullptr) { return ;}
	if (locked == 0) { return ; }

	uint32_t *pixel = (uint32_t *)mLockBuffer.bits;
	int line = 0;
	for(;line<mLockBuffer.height; line++, pixel += mLockBuffer.stride) {
		uint32_t c_ = color_;
#if 0
		if (line < (mLockBuffer.height/3)) { c_ = 0xff0000ff;}
		else if(line < ((2*mLockBuffer.height)/3)) { c_ = 0xff00ff00; }
		else if(line < mLockBuffer.height) { c_ = 0xfff0000; }
#endif
		for(int col=0;col<mLockBuffer.width;col++) {
			pixel[col] = c_;
		}
	}
}
void HistogramGrapherImpl1::draw_vertical_line_l(int x, int y0, int y1, uint32_t color_) {
	if (mWindow_ == nullptr) { return ;}
	if (locked == 0) { return ; }

	if (x < 0) {x = 0; }
	if (y0 < 0) { y0 = 0; }
	if (y1 < 0) { y1 = 0; }
	if (x >= mLockBuffer.width) { x = mLockBuffer.width; }
	if (y0 >= mLockBuffer.height) { y0 = mLockBuffer.height - 1; }
	if (y1 >= mLockBuffer.height) { y1 = mLockBuffer.height - 1; }

	if (x >= mLockBuffer.width || y0 >= mLockBuffer.height || y1 >= mLockBuffer.height
		|| y0 > y1) {
		DLOGD( "error !!! mLockBuffer size: %4d x %4d x=%4d y0=%4d y1=%4d color_:0x%08x \r\n", 
			mLockBuffer.width, mLockBuffer.height,
			x, y0, y1, color_);
		return ;
	}
	uint32_t *pixel = (uint32_t *)mLockBuffer.bits;
	int line = y0;
	pixel += y0*mLockBuffer.stride;
	for(;line<mLockBuffer.height && line <= y1; line++, pixel += mLockBuffer.stride) {
		pixel[x] = color_;
	}
}
void HistogramGrapherImpl1::draw_point_l(int x, int y0, uint32_t color_) {
	if (mWindow_ == nullptr) { return ;}
	if (locked == 0) { return ; }

	if (x < 0) {x = 0; }
	if (y0 < 0) { y0 = 0; }
	if (x >= mLockBuffer.width) { x = mLockBuffer.width; }
	if (y0 >= mLockBuffer.height) { y0 = mLockBuffer.height - 1; }

	if (x >= mLockBuffer.width || y0 >= mLockBuffer.height) {
		DLOGD( "error !!! mLockBuffer size: %4d x %4d x=%4d y0=%4d color_:0x%08x \r\n", 
			mLockBuffer.width, mLockBuffer.height,
			x, y0, color_);
		return ;
	}
	uint32_t *pixel = (uint32_t *)mLockBuffer.bits;
	int line = y0;
	pixel += y0*mLockBuffer.stride;
	for(;line<mLockBuffer.height && line <= y0; line++, pixel += mLockBuffer.stride) {
		pixel[x] = color_;
	}
}
void HistogramGrapherImpl1::draw_horizontal_line_l(int x0, int x1, int y0, uint32_t color_) {
	if (mWindow_ == nullptr) { return ;}
	if (locked == 0) { return ; }

	if (x0 < 0) {x0 = 0; }
	if (x1 < 0) { x1 = 0; }
	if (y0 < 0) { y0 = 0; }
	if (x0 >= mLockBuffer.width) { x0 = mLockBuffer.width; }
	if (x1 >= mLockBuffer.width) { x1 = mLockBuffer.width - 1; }
	if (y0 >= mLockBuffer.height) { y0 = mLockBuffer.height - 1; }

	if (x0 >= mLockBuffer.width || x1 >= mLockBuffer.width || y0 >= mLockBuffer.height
		|| x0 > x1) {
		DLOGD( "error !!! mLockBuffer size: %4d x %4d x0=%4d x1=%4d y0=%4d color_:0x%08x \r\n", 
			mLockBuffer.width, mLockBuffer.height,
			x0, x1, y0, color_);
		return ;
	}
	uint32_t *pixel = (uint32_t *)mLockBuffer.bits;
	int line = y0;
	pixel += y0*mLockBuffer.stride;
	int col = 0;
	for(;line<mLockBuffer.height && line <= y0; line++, pixel += mLockBuffer.stride) {
		for(col = x0; col <= x1; col++) {
			pixel[col] = color_;
		}
	}
}

void HistogramGrapherImpl1::draw_stats(IndirectStats__c *stats) {
	long total_ = 0;
	long max_ = 0;
	long min_ = stats->drawCount;

	int total_points = stats->drawCount;

	// all scale to draw window size !!!

	// stats->bin_count -> DRAW_POINTS_NUM
	// stats->lodCount -> draw_points
	
	int block_size = stats->bin_count/DRAW_POINTS_NUM;
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		int sum_ = 0;
		for(int local_=0;local_ < block_size; local_++) {
			sum_ += stats->lodCount[i*block_size + local_];
		}
		draw_points[i] = sum_;

		//
		double normalAccSum = 0.0f;
		for(int local_=0;local_ < block_size; local_++) {
			normalAccSum += stats->accHistNormal[i*block_size + local_];
		}
		accHistNormal[i] = normalAccSum/block_size;
		accHistNormal[i] *= (mHeight - 1);

		//
		int lut_entry = 0;
		double lut_normal;
		for(int local_=0;local_ < block_size; local_++) {
			lut_entry += stats->agc_lut[i*block_size + local_];
		}
		agc_lut[i] = lut_entry/block_size;
		lut_normal = agc_lut[i]; lut_normal /= stats->bin_count;
		agc_lut[i] = (uint32_t)(lut_normal*(mHeight - 1));

		//
		lut_entry = 0;
		for(int local_=0;local_ < block_size; local_++) {
			lut_entry += stats->bhe_lut[i*block_size + local_];
		}
		bhe_lut[i] = lut_entry/block_size;
		lut_normal = bhe_lut[i]; lut_normal /= stats->bin_count;
		bhe_lut[i] = (uint32_t)(lut_normal*(mHeight - 1));

		//
		lut_entry = 0;
		for(int local_=0;local_ < block_size; local_++) {
			lut_entry += stats->merged_lut[i*block_size + local_];
		}
		merged_lut[i] = lut_entry/block_size;
		lut_normal = merged_lut[i]; lut_normal /= stats->bin_count;
		merged_lut[i] = (uint32_t)(lut_normal*(mHeight - 1));

	}

	for(int i=0;i<32;i++) {
		pointIndex[i] = stats->pointIndex[i]/block_size;
	}


	///////////////////////////////////////////////////
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		total_ += draw_points[i];
		if(min_ > draw_points[i]) {
			min_ = draw_points[i];
		}
		if (max_ < draw_points[i]) {
			max_ = draw_points[i];
		}
	}
	long range_ = max_ - min_;

	DLOGD( "HistogramGrapherImpl1::%s, total_points/total_ = %d/%ld min=%ld, max=%ld bin_count:%d \r\n", 
		__func__, total_points, total_, min_, max_, stats->bin_count);

	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		double normal_height = static_cast<double>(draw_points[i]);
		//draw_points[i] = (normal_height * mHeight)/total_points;
		normal_height -= min_; normal_height /= range_;  draw_points[i] = normal_height * mHeight;
	}
	

	draw_lock();
	draw_clear_l(0xff202020);

	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		draw_vertical_line_l(mBase_x + i*2+0, mHeight - 1 - draw_points[i], mHeight - 1, 0xff009090);
		draw_vertical_line_l(mBase_x + i*2+1, mHeight - 1 - draw_points[i], mHeight - 1, 0xff009090);
	}

	// normal acc hist curve
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		draw_point_l(mBase_x + i*2+0, mHeight - 1 -(int)accHistNormal[i], 0xff0000ff);
		//draw_point_l(mBase_x + i*2+1, mHeight - 1 -(int)accHistNormal[i], 0xff0000ff);
	}

	// draw grid
	for(int i=0;i<3;i++) {  // only 0, 1, 2
		draw_vertical_line_l(mBase_x + pointIndex[i]*2+0, 0, mHeight - 1 - 0, 0xff404040);
		//draw_vertical_line_l(mBase_x + pointIndex[i]*2+1, 0, mHeight - 1 - 0, 0xff404040);
	}
	for(int i=0;i<3;i++) {  // only 0, 1, 2
		draw_horizontal_line_l(mBase_x + 0*2+0, mWidth - 1, mHeight - 1 - y0_points[i]*(mHeight - 1), 0xff404040);
		//draw_horizontal_line_l(mBase_x + 0*2+1, mWidth - 1, mHeight - 1 - y0_points[i]*(mHeight-1), 0xff404040);
	}

	// agc lut curve
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		draw_point_l(mBase_x + i*2+0, mHeight - 1 -(int)agc_lut[i], 0xffffffff);
		//draw_point_l(mBase_x + i*2+1, mHeight - 1 -(int)agc_lut[i], 0xffffffff);
	}

	// bhe lut curve
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		draw_point_l(mBase_x + i*2+0, mHeight - 1 -(int)bhe_lut[i], 0xff00ff00);
		//draw_point_l(mBase_x + i*2+1, mHeight - 1 -(int)bhe_lut[i], 0xff00ff00);
	}
	
	// merge lut curve
	for(int i=0;i<DRAW_POINTS_NUM;i++) {
		draw_point_l(mBase_x + i*2+0, mHeight - 1 -(int)merged_lut[i], 0xffff0000);
		//draw_point_l(mBase_x + i*2+1, mHeight - 1 -(int)merged_lut[i], 0xffff0000);
	}

	draw_unlock();
}



/*static*/ HistogramGrapher *HistogramGrapher::create() {
	return new HistogramGrapherImpl1();
}

}

