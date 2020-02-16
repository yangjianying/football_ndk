

#include "CmdlineV1.h"

// implementation
#include "MenuV1.h"

#if 0

#undef JNI_FR_DEBUG_H_
#if 1  // must @ last
#undef config_fr_DEBUG_EN
#undef config_fr_DEBUG_TO_CONSOLE
#define config_fr_DEBUG_EN 1  //  (1)
#define config_fr_DEBUG_TO_CONSOLE 1 //  1 //  0
#include "cycling_utils/cy_debug.h"
#endif
#undef LOG_TAG
#define LOG_TAG "CmdlineV1"

#endif

#undef NULL
#define NULL 0

/*********************************************************************/
/** USING_c_menu_impl only one instance !!! */
/*********************************************************************/
//#define USING_c_menu_impl 1



namespace android {

/* static */ int CmdlineV1::sCmdlineV1Num = 0;

static int __quit(void *, int argc, char * const argv[]) {
	return -10000;
}

#define STR_C11_LITERAL(s) (s)

CmdlineV1::CmdlineV1()    {
	//FrLOGV(LOG_TAG, "%s,%d +", __func__, __LINE__);

	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = new ::NS_cmdline_v1::NS_menu_v1::MenuV1();
	impl1 = (void*)menu;

	sCmdlineV1Num++;
}
CmdlineV1::~CmdlineV1() {
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)impl1;
	delete menu;
	impl1 = NULL;

	sCmdlineV1Num--;
	//FrLOGV(LOG_TAG, "%s,%d -", __func__, __LINE__);
}
void CmdlineV1::addInternalCmd() {
	if (internalAddon == 0) {
		internalAddon = 1;
		::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)impl1;
		//menu->menuConfig(STR_C11_LITERAL("clversion"), 
		//	STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION "(Based on Linux 3.18.6)"),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL("clversion"), 
			STR_C11_LITERAL("cmdline tool " CMDLINE_TOOL_VERSION),NULL, NULL);
		menu->menuConfig(STR_C11_LITERAL("quit"),
			STR_C11_LITERAL("Quit from cmdline tool"), __quit, NULL);
	}
}

void CmdlineV1::setPrompt(const char *prompt) {
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)impl1;
	menu->setPrompt(prompt);
}
void CmdlineV1::addEmptyCmdCallback(PF_empty_cmd_cb cb, void *ctx) {
}
void CmdlineV1::onEmptyCmd() {
}

int CmdlineV1::add(const char * cmd, const char * desc, int (*handler)(void *, int, char * const *), void *ctx) {
	addInternalCmd();
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)impl1;
	return menu->menuConfig(STR_C11_LITERAL(cmd), STR_C11_LITERAL(desc), handler, ctx);
}

int CmdlineV1::loop() {
	::NS_cmdline_v1::NS_menu_v1::MenuV1 *menu = (::NS_cmdline_v1::NS_menu_v1::MenuV1 *)impl1;
	return menu->loop();
}
int CmdlineV1::runCommand(const char * cmd) {
	return -1;
}


//////////////////////////////////////////////////////////

CmdlineV1Combined::CmdlineV1Combined() {

}
CmdlineV1Combined::~CmdlineV1Combined() {
	
}
void CmdlineV1Combined::setPrompt(const char *prompt) {
	return ;
}
int CmdlineV1Combined::add(CmdlineV1* cl) {
	return 0;
}
int CmdlineV1Combined::loop() {
	return 0;
}

};

