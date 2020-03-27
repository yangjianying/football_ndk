#ifndef __FOOTBALL_PP_H__
#define __FOOTBALL_PP_H__

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

namespace football {

typedef void (*PF_SessionInfo_on_frame)(void *);

#define DECLARE_SessionInfo_on_frame(func_name); \
	static void _s_##func_name(void *ctx); \
	void func_name(void);

#define IMPL_SessionInfo_on_frame(CLASS_type_, func_name); \
	/*static*/ void CLASS_type_ ::_s_##func_name(void *ctx) { \
		if (ctx != nullptr) { \
			CLASS_type_ *p = static_cast<CLASS_type_ *>(ctx); \
			p->func_name(); \
		} \
	}
#define SessionInfo_on_frame_s_func(func_name) _s_##func_name

struct SessionInfo {
	enum {
		INPUT_OWNER_CPU = 0,
		INPUT_OWNER_VIRTUAL_DISPLAY = 1,
	};

	ANativeWindow *final_image = nullptr;

	// out data size/format should be consistent with "backlight_data"
	// if not provided, use the size/format form "backlight_data"
	uint32_t bl_width = -1;
	uint32_t bl_height = -1;
	uint32_t bl_format = -1;
	ANativeWindow *backlight_data = nullptr;

	// the owner of the input window affect the way how input window is created !
	int inputOwner = -1;

	// a input window must be requested the same size/format as final_image !!!
	// if not provided, use the size/format form "final_image"
	uint32_t width = -1;
	uint32_t height = -1;
	uint32_t format = -1;
	ANativeWindow *input_window = nullptr;  // out

	//
	void *cb_ctx = nullptr;
	PF_SessionInfo_on_frame cb_ = nullptr;
	void _on_frame_set(PF_SessionInfo_on_frame c_ , void *ctx_) { cb_ = c_; cb_ctx = ctx_; }
	void _on_frame_call();
};
struct SessionParameterTriggerData {
	int type = 0;
	// = 1, = 3
	int param_int0 = 0;
	int param_int1 = 0;
	int param_int2 = 0;
	// = 2
	float factor0 = 0.0f;
	float factor1 = 0.0f;
};
struct SessionParameter {
	int have_algo = 1;
	int trigger_request = 0;
	int request_type = 0;
	SessionParameterTriggerData trigger_data;
};
class FootballPP {
public:
	class FootballSession {
	public:
		virtual ~FootballSession() {}
		virtual int setSessionParameter(SessionParameter *parameter) = 0;
		virtual int getSessionParameter(SessionParameter *parameter) = 0;
		virtual void print() = 0;
	};
	virtual ~FootballPP() {}

	virtual int buildSession(int session_type, SessionInfo &session, int *session_id) = 0;
	
	virtual int closeSession(int session_id);
	virtual std::vector<int> getSessionIds();
	virtual int getSession(int session_id, SessionInfo *& session);
	virtual int setSessionParameter(int session_id, SessionParameter *parameter);
	virtual int getSessionParameter(int session_id, SessionParameter *parameter);
	virtual void print(int session_id);

	int addSession(FootballSession *session_);
	int removeSession(int session_id);

	static int s_SessionId_generator;
	std::vector<int> mSessionIds;
	std::map<int, FootballSession*> mSessions;

};



};



#endif

