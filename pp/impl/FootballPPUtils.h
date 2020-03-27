#ifndef __FOOTBALLPPUTILS_H__
#define __FOOTBALLPPUTILS_H__

namespace football {

class SyncTaskHandlerImpl_;

class SyncTaskHandler {
public:

	SyncTaskHandler() {}
	virtual ~ SyncTaskHandler() {}
	void init(void *ctx_);
	void deinit();

	int post_parameter(int type_, void *parameter_);
	int process(void *something);

	virtual int onStart(void *ctx_) = 0;
	virtual int onParameter(int type_, void *parameter_) = 0;
	virtual int onProcess(void *something) = 0;
	virtual int onStop() = 0;

private:
	SyncTaskHandlerImpl_ *impl_ = nullptr;;

};


};

#endif

