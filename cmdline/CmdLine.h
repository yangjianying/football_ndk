#ifndef __CMDLINE_H__
#define __CMDLINE_H__


namespace android {


//
#undef STRINGFY
#define STRINGFY(s) #s

#define CMDLINE_S_FUNC(func__) __s_cmdline_##func__

#define CL_DEF_FUNC(func__) \
	static int __s_cmdline_##func__(void *ctx, int argc, char * const *argv); \
	int func__(int argc, char * const *argv);

#define CL_SFUNC_IMPL(NS__, ctx_type__, func__) \
	/* static */ int NS__::__s_cmdline_##func__(void *ctx_, int argc, char * const *argv) { \
		/*fprintf(stderr, "%s,%d %s \r\n", __func__, __LINE__, (ctx_ != NULL ? "ctx_!=null" : "ctx_==null"));*/ \
		if (ctx_ != NULL) {\
			ctx_type__ *__p = (ctx_type__ *)ctx_;\
			return __p->func__(argc, argv);\
		}\
		return 0;\
	}

#define CL_func_IMPL(NS__, func__, ctx_type__) \
	CL_SFUNC_IMPL(NS__, ctx_type__, func__) \
	int NS__::func__(int argc, char * const *argv)


#define CL_ADD_FUNC(ptrCL, func) ptrCL->add(STRINGFY(func), STRINGFY(func), CMDLINE_S_FUNC(func), this);
#define CL_ADD_FUNC_c(ptrCL, c_, func) ptrCL->add(STRINGFY(c_), STRINGFY(c_), CMDLINE_S_FUNC(func), this);


#define CL_SFUNC_NS(NS__, func__) NS__::__s_cmdline_##func__
#define CL_SFUNC_NS0(NS__, func__) NS__::func__

#define CL_ADD_FUNC_f(ptrCL__, cmd__, ctx__, func__) ptrCL__->add(STRINGFY(cmd__), STRINGFY(cmd__), func__, ctx__);

typedef void (*PF_empty_cmd_cb)(void *);
#define DECLARE_EMPTY_CB(func__) \
	static void __s_empty_cb_##func__(void *ctx); \
	void func__(void);

#define IMPL_EMPTY_CB_WRAPPER(NS__, ctx_type__, func__) \
	/* static */ void NS__::__s_empty_cb_##func__(void *ctx_) { \
		if (ctx_ != NULL) {\
			ctx_type__ *__p = (ctx_type__ *)ctx_;\
			__p->func__();\
		}\
	}
#define EMPTY_CB_FUNC(NS__, func__) NS__::__s_empty_cb_##func__
#define ADD_EMPTY_CB_FUNC(cmd, NS__, func__, ctx) \
	cmd->addEmptyCmdCallback(EMPTY_CB_FUNC(NS__, func__), ctx)

class Cmdline
	// : public RefBase
{

public:
	virtual ~Cmdline() {}
	virtual void setPrompt(const char *prompt) = 0;
	virtual void addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) = 0;
	virtual void onEmptyCmd() = 0;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) = 0;
	virtual int loop() = 0;
	virtual int runCommand(const char * cmd) = 0;

};

};

#endif

