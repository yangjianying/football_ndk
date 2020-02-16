
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

#include "CmdlineV2.h"
#include "cmdline_v1/MenuV1.h"

namespace android {

static int __quit(void *, int argc, char * const argv[]) {
	return -10000;
}

#define STR_C11_LITERAL(s) (s)

class cli_impl : public ::cli {
public:
	cli_impl(CmdlineV2 *pc) : cli(), pCmdlineV2(pc) {

	}
	~cli_impl() {
	}

	virtual int cli_cmd_process_(int flag, int argc, char * const argv[],
			       int *repeatable, ulong *ticks) override {
		if (pCmdlineV2 != nullptr) {
			return pCmdlineV2->cli_cmd_process_(flag, argc, argv, repeatable, ticks);
		}
		return 0;
	}
	virtual void cli_cmd_empty() override {
		if (pCmdlineV2 != nullptr) {
			return pCmdlineV2->cli_cmd_empty_();
		}
	}

	CmdlineV2 *pCmdlineV2 = nullptr;
};

CmdlineV2::CmdlineV2() {
	impl1 = (void *)new cli_impl(this);
	menu_impl = (void*) new ::NS_cmdline_v1::NS_menu_v1::MenuV1();
}
CmdlineV2::~CmdlineV2()   {
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	delete menu;

	cli_impl *_impl = (cli_impl*)impl1;
	delete _impl;
}

void CmdlineV2::setPrompt(const char *prompt)  {
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	menu->setPrompt(prompt);
}
void CmdlineV2::addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) {
	mEmptyCmdCallback = cb;
	mEmptyCmdCallback_ctx = ctx;
}
void CmdlineV2::onEmptyCmd() {
}

int CmdlineV2::add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx)  {
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
	return menu->menuConfig(STR_C11_LITERAL(cmd), STR_C11_LITERAL(desc), handler, ctx);
}
int CmdlineV2::loop()  {
	cli_impl *_impl = (cli_impl*)impl1;
	_impl->cli_loop();
	return 0;
}
int CmdlineV2::runCommand(const char * cmd) {
	cli_impl *_impl = (cli_impl*)impl1;
	return _impl->run_cli_command(cmd);
}

void CmdlineV2::addInternalCmd() {
	if (internalAddon == 0) {
		internalAddon = 1;

		::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
		//menu->menuConfig(STR_C11_LITERAL("clversion"), 
		//	STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION "(Based on Linux 3.18.6)"),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL("clversion"), 
			STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL("quit"),
			STR_C11_LITERAL("Quit from cmdline tool"), __quit, NULL);

	}
}
int CmdlineV2::cli_cmd_process_(int flag, int argc, char * const argv[],
				   int *repeatable, uint64_t *ticks) {
   CLI_UNUSED_(flag);
   CLI_UNUSED_(argc);
   CLI_UNUSED_(argv);
   CLI_UNUSED_(repeatable);
   CLI_UNUSED_(ticks);
   	//fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
#if 0
   	printf("%s, %d argc: %d \r\n", __func__, __LINE__, argc);
	for(int i=0;i<argc;i ++) {
		printf("%04d, (%s)\r\n", i, argv[i]);
	}
#endif
	if (menu_impl != NULL) {
		::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)menu_impl;
		return menu->cmd_process(argc, argv);
	}
   return 0;

}
void CmdlineV2::cli_cmd_empty_() {
	onEmptyCmd();
	if (mEmptyCmdCallback != nullptr) {
		mEmptyCmdCallback(mEmptyCmdCallback_ctx);
	}
}

};

