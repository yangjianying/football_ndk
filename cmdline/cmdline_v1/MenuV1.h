#ifndef __MENU_V1_H___
#define __MENU_V1_H___

namespace NS_cmdline_v1 {
namespace NS_menu_v1 {

typedef int (*PF_handler)(void *ctx, int argc, char * const argv[]);

class MenuV1 {
public:
////////////////////////////////////////////////////

	MenuV1();
	~MenuV1();

	/* Set Input Prompt */
	int setPrompt(const char * p);
	/* add cmd to menu */
	int menuConfig(const char * cmd, const char * desc, PF_handler handler, void *ctx);
	/* Menu Engine Execute */
	int loop();
	int cmd_process(int argc, char * const argv[]);

	void * mHead = nullptr;
	char *mPrompt = nullptr;

	static int Help(void *ctx, int argc, char * const argv[]);

};

};  // namespace NS_menu_v1 {
};  // end of namespace NS_cmdline_v1 {

#endif

