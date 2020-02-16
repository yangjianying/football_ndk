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
#if 0
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaCrypto.h>
#include <media/NdkMediaDataSource.h>
#include <media/NdkMediaDrm.h>
#include <media/NdkMediaError.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#endif

#include "FootballPP.h"
#include "ANativeWindowUtils.h"

namespace football {

class TestColorGenerator;

class FootballPPCpu: public FootballPP {
public:

	class FootSessionCpu: public AImageReader_ImageListener {
	public:
		static void s_AImageReader_ImageCallback(void* context, AImageReader* reader);

		FootSessionCpu(FootSession *session);
		~FootSessionCpu();
		bool isValid();

		int max_mean_value(AImageData *image_data, int x, int y, int w_, int h_, long *max_, long *mean_);
		int led_spread();
		void processImage(AImageData *image_data);
		void onImageProc1(AImageData *image_data);
		void onImageAvailableCallback(AImageReader *reader);

		int setSessionParameter(SessionParameter *parameter);
		int getSessionParameter(SessionParameter *parameter);
		void print();

		FootSession mFootSession;
		SessionParameter mSessionParameter;

		int mId = -1;
		AImageReader *mReader = nullptr;
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
	virtual int buildSession(FootSession *session, int *session_id) override;
	virtual int closeSession(int session_id) override;
	virtual int setSessionParameter(int session_id, SessionParameter *parameter) override;
	virtual int getSessionParameter(int session_id, SessionParameter *parameter) override;
	virtual std::vector<int> getSessionIds() override;
	virtual int getSession(int session_id, FootSession *session) override;
	virtual void print(int session_id) override;

	std::vector<int> mSessionIds;
	std::map<int, FootSessionCpu*> mSessions;

	static int s_SessionId_generator;
};



};



#endif

