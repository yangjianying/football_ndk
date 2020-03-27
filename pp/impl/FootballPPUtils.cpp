#include <unistd.h>

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <math.h>

#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <stdint.h>
#include <string>

#if 1  // 
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include <GLES2/gl2.h>

// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#endif

#include "FootballConfig.h"

#include "screenrecord/Program.h"
#include "screenrecord/TextRenderer.h"
#include "screenrecord/EglWindow.h"

#include "gpu_tonemapper/EGLImageBuffer_KHR.h"

#define LOG_TAG "FootballPPGles"
#include "android_logcat_.h"


#include "FootballPPUtils.h"

#undef __CLASS__
#define __CLASS__ "FootballPPUtils"
#include "gpu_tonemapper/utils/sync_task.h"

namespace football {

enum class TaskCode : int32_t {
  kCodeGetInstance,
  kSetParameter,
  kCodeBlit,
  kCodeDestroy,
};
struct GetInstanceContext : public sdm::SyncTask<TaskCode>::TaskContext {
	void *ctx_ = nullptr;
};

struct ParameterContext : public sdm::SyncTask<TaskCode>::TaskContext {
	int type = 0;
	void *p_something = nullptr;
	int result = 0;
};
struct BlitContext : public sdm::SyncTask<TaskCode>::TaskContext {
	void *p_something = nullptr;
	int result = 0;
};

class SyncTaskHandlerImpl_ : public sdm::SyncTask<TaskCode>::TaskHandler
	{
public:

	SyncTaskHandlerImpl_(SyncTaskHandler *cb_, void *ctx_)
		: handler_(cb_)
		, _task(new sdm::SyncTask<TaskCode>(*this)) 
	{
		start(ctx_);
	}
	virtual ~ SyncTaskHandlerImpl_() {
		{
			std::unique_lock<std::mutex> caller_lock(caller_mutex_);
			mDestroied = true;
		}

		stop();

		{
			std::unique_lock<std::mutex> caller_lock(caller_mutex_);
			if (_task != nullptr) {
				delete _task;
				_task = nullptr;
			}
		}
	}

	int start(void *ctx_) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (_task != nullptr) {
			GetInstanceContext ctx;
			ctx.ctx_ = ctx_;
			_task->PerformTask(TaskCode::kCodeGetInstance, &ctx);
		}
		return 0;
	}
	int post_parameter(int type_, void *parameter_) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (mDestroied) {
			return -1;
		}
		if (_task != nullptr) {
			ParameterContext ctx = {};
			ctx.type = type_;
			ctx.p_something = parameter_;
			_task->PerformTask(TaskCode::kSetParameter, &ctx);
			return ctx.result;
		}
		return 0;
	}
	int process_something(void *p_something) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (mDestroied) {
			return -1;
		}
		if (_task != nullptr) {
			BlitContext ctx = {};
			ctx.p_something = p_something;
			_task->PerformTask(TaskCode::kCodeBlit, &ctx);
			return ctx.result;
		}
		return 0;
	}
	int stop() {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (_task != nullptr) {
			_task->PerformTask(TaskCode::kCodeDestroy, nullptr);
		}
		return 0;
	}

	int call_onStart(void *ctx_) {
		if (handler_ != nullptr) {
			return handler_->onStart(ctx_);
		}
		return 0;
	}
	int call_onParameter(int type_, void *something) {
		if (handler_ != nullptr) {
			return handler_->onParameter(type_, something);
		}
		return 0;
	}
	int call_onProcess(void *something) {
		if (handler_ != nullptr) {
			return handler_->onProcess(something);
		}
		return 0;
	}
	int call_onStop() {
		if (handler_ != nullptr) {
			return handler_->onStop();
		}
		return 0;
	}
	
	// TaskHandler methods implementation.
	virtual void OnTask(const TaskCode &task_code,
						sdm::SyncTask<TaskCode>::TaskContext *task_context) override
	{
		  DLOGD( "%s, task_code:%d \r\n", __func__, (int)task_code);
		
		switch (task_code) {
		  case TaskCode::kCodeGetInstance: {
				GetInstanceContext *ctx = static_cast<GetInstanceContext *>(task_context);
				if (mSetupFlag == false) {
					call_onStart(ctx->ctx_);
					mSetupFlag = true;
				}
			}
			break;
		
		case TaskCode::kSetParameter: {
			  ParameterContext *ctx = static_cast<ParameterContext *>(task_context);
			  if (mSetupFlag) {
				  ctx->result = call_onParameter(ctx->type, ctx->p_something);
			  }
		  }
		  break;
		  case TaskCode::kCodeBlit: {
				BlitContext *ctx = static_cast<BlitContext *>(task_context);
				if (mSetupFlag) {
					ctx->result = call_onProcess(ctx->p_something);
				}
			}
			break;
		  case TaskCode::kCodeDestroy: {
				if (mSetupFlag) {
					mSetupFlag = false;
					call_onStop();
				}
			}
			break;
		  default:
			break;
		}

	}

private:
	SyncTaskHandler *handler_ = nullptr;
	sdm::SyncTask<TaskCode> *_task = nullptr;

	bool mSetupFlag = false;
	bool mDestroied = false;
	std::mutex caller_mutex_;

};

void SyncTaskHandler::init(void *ctx_) {
	impl_ = new SyncTaskHandlerImpl_(this, ctx_);
}
void SyncTaskHandler::deinit() {
	if (impl_ != nullptr) {
		delete impl_;
		impl_= nullptr;
	}
}
int SyncTaskHandler::post_parameter(int type_, void *parameter_) {
	if (impl_ != nullptr) {
		return impl_->post_parameter(type_, parameter_);
	}
	return -1;
}

int SyncTaskHandler::process(void *something) {
	if (impl_ != nullptr) {
		return impl_->process_something(something);
	}
	return -1;
}


};


