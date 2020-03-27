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
		/*DLOGD( "%s,%d %s \r\n", __func__, __LINE__, (ctx_ != NULL ? "ctx_!=null" : "ctx_==null"));*/ \
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

/**/
typedef void (*PF_on_empty_cmd)(void *);
#define DECLARE_on_empty_cmd(func__) \
	static void __s_empty_##func__(void *ctx); \
	void func__(void);

#define IMPL_on_empty_cmd(NS__, ctx_type__, func__) \
	/* static */ void NS__::__s_empty_##func__(void *ctx_) { \
		if (ctx_ != NULL) {\
			ctx_type__ *__p = (ctx_type__ *)ctx_;\
			__p->func__();\
		}\
	}
#define on_empty_cmd_FUNC(NS__, func__) NS__::__s_empty_##func__
#define ADD_on_empty_cmd(cmd, NS__, func__, ctx) \
	cmd->add_on_empty_cmd(on_empty_cmd_FUNC(NS__, func__), ctx)

/**/
typedef int (*PF_on_intercept_command)(void *ctx_, const char *cmd, int flag);
#define DECLARE_on_intercept_command(func__) \
	static int __s_on_intercept_command_##func__(void *ctx, const char *cmd, int flag); \
	int func__(const char *cmd, int flag);
#define IMPL_on_intercept_command(NS__, ctx_type__, func__) \
	/* static */ int NS__::__s_on_intercept_command_##func__(void *ctx_, const char *cmd, int flag) { \
		if (ctx_ != NULL) {\
			ctx_type__ *__p = (ctx_type__ *)ctx_;\
			return __p->func__(cmd, flag);\
		}\
		return 0; \
	}
#define on_intercept_command_FUNC(NS__, func__) NS__::__s_on_intercept_command_##func__
#define ADD_on_intercept_command(cmd, NS__, func__, ctx) \
		cmd->add_on_intercept_command(on_intercept_command_FUNC(NS__, func__), ctx)

enum {
	CLI_OPT_console_none = 0x00,
	CLI_OPT_console_reader = 0x01,
};
class Cmdline
	// : public RefBase
{

public:
	static const char *kCmd_quit;
	static const char *kCmd_q_;
	static const char *kCmd_help;
	static const char *kCmd_h_;
	static const char *kCmd_clversion;

	Cmdline(int opt_ = CLI_OPT_console_reader):mOpt(opt_) {}
	virtual ~Cmdline() {}
	virtual void setPrompt(const char *prompt) = 0;
	virtual void add_on_empty_cmd(PF_on_empty_cmd cb, void *ctx) = 0;
	virtual void add_on_intercept_command(PF_on_intercept_command, void *ctx) = 0;
	virtual void on_empty_cmd_i() = 0;
	virtual int add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) = 0;
	virtual int loop() = 0;
	virtual int check_command_matched(const char * cmd, const char *matched_) = 0;
	virtual int postCommand(const char * cmd) = 0;
	virtual int runCommand(const char * cmd) = 0;
	int mOpt = 0;
};

};

#endif

