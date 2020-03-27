#ifndef __FOOTBALL_PP_CPU_H__
#define __FOOTBALL_PP_CPU_H__

#include <vector>
#include <iostream>
#include <string>
#include <map>


#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <android/surface_control.h>
#include <android/hardware_buffer.h>
#include <android/choreographer.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "FootballPP.h"

#include "ANativeWindowUtils.h"

namespace football {


class TestColorGenerator;


class FootballPPCpu: public FootballPP {
public:

	class FootSessionCpu: 
		public ::football::FootballPP::FootballSession
		, public football::ImageReaderImageListenerWrapper
		, public HardwareBufferReader::CB
			{
	public:
		FootSessionCpu(SessionInfo &session);
		virtual ~FootSessionCpu();
		bool isValid();

		int max_mean_value(AImageData *image_data, int x, int y, int w_, int h_, long *max_, long *mean_);
		int led_spread();
		void processImage1(AImageData *image_data);
		void onImageProc1(AImageData *image_data);

		void processingImage(AImage *image_ );

		// impl public football::ImageReaderImageListenerWrapper
		virtual void onImageAvailableCallback(AImageReader *reader) override ;

		// impl public HardwareBufferReader::CB
		virtual int on_process_image(AImage *image_) override;

		// impl public ::football::FootballPP::FootballSession
		virtual int setSessionParameter(SessionParameter *parameter) override ;
		virtual int getSessionParameter(SessionParameter *parameter) override ;
		virtual void print() override ;

		//
		SessionInfo &mSessionInfo;
		SessionParameter mSessionParameter;

		int mId = -1;
		
		AImageReader *mReader = nullptr;
		HardwareBufferReader *mHardwareBufferReader = nullptr;
		TestColorGenerator *mTestColorGenerator = nullptr;

		AImageData *mLastIncommingData = nullptr;
		AImageData *mLastIncommingAlignedData = nullptr;
		AImageData *mLastFinalImageData = nullptr;
		
		int block_height = 0;
		int block_width = 0;
		int block_num_height = 0;
		int block_num_width= 0;

		int LD_BLOCK_NUM_HOR = 0;
		int LD_BLOCK_NUM_VER = 0;
	
		int LD_PIC_DATA_HOR = 0;
		int LD_PIC_DATA_VER = 0;
	
		int LD_BLK_SIZE_HOR = 0;
		int LD_BLK_SIZE_VER = 0;
	
		int *max_value_array = nullptr;  // stored as lines : block_num_width * block_num_height
		int *means = nullptr;
		int *led_pwm_array = nullptr;
		int *led_coeff_space = nullptr;
		int *led_coeff_gamma = nullptr;
		int *backlight_output = nullptr;
		int *vt_bl = nullptr;

		int *led_blur = nullptr;		// width*height
		int *bl_dim = nullptr; 			// width*height  // width*height*3
		double *d_indata = nullptr;		// width*height*3
		int *out_data_2 = nullptr;		// width*height*3
		double *out_data_4 = nullptr;	// width*height*3
		double *pic_alph = nullptr;		// width*height*3
		double *out_data = nullptr; 	// width*height*3

		long max_value_histo[256] = {0};
		long maxvalue2pwm[256] = {0};
		long back_light_gamma[256] = {0};
		long gamma_table[256] = {0};
		int de_gamma_table[4096] = {0};
		long spread_tab[256] = {0};
		double dim_tab[256] = {0.0f};
		double alph_table[256] = {0.0f};


	};

	FootballPPCpu();
	virtual ~FootballPPCpu() override;
	virtual int buildSession(int session_type, SessionInfo &session, int *session_id) override;
	
};



};



#endif

