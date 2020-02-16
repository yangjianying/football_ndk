#ifndef __FOOTBALL_PP_H__
#define __FOOTBALL_PP_H__

#include<vector>

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

namespace football {

struct FootSession {
	ANativeWindow *final_image = nullptr;

	// out data size/format should be consistent with "backlight_data"
	// if not provided, use the size/format form "backlight_data"
	uint32_t bl_width = -1;
	uint32_t bl_height = -1;
	uint32_t bl_format = -1;
	ANativeWindow *backlight_data = nullptr;

	// a input window must be requested the same size/format as final_image !!!
	// if not provided, use the size/format form "final_image"
	uint32_t width = -1;
	uint32_t height = -1;
	uint32_t format = -1;
	ANativeWindow *input_window = nullptr;  // out
};
struct SessionParameter {
	int have_algo = 1;
};
class FootballPP {
public:
	virtual ~FootballPP() {}
	virtual int buildSession(FootSession *session, int *session_id) = 0;
	virtual int closeSession(int session_id) = 0;
	virtual int setSessionParameter(int session_id, SessionParameter *parameter) = 0;
	virtual int getSessionParameter(int session_id, SessionParameter *parameter) = 0;
	virtual std::vector<int> getSessionIds() = 0;
	virtual int getSession(int session_id, FootSession *session) = 0;
	virtual void print(int session_id) = 0;
};



};



#endif

