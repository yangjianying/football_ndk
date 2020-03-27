#ifndef __MBI6322FB_H___
#define __MBI6322FB_H___

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

namespace android {
namespace local_bl {

namespace test {

class LinuxFb {
public:
	LinuxFb();
	~LinuxFb();

	void printFixedInfo ();
	void printVariableInfo ();
	
	void drawRect_rgb32 (int x0, int y0, int width,int height, int color);
	void drawRect_rgb16 (int x0, int y0, int width,int height, int color);
	void drawRect_rgb15 (int x0, int y0, int width,int height, int color);
	void drawRect (int x0, int y0, int width, int height, int color);
	void performSpeedTest (void *fb, int fbSize);
	int test_main (int argc, char **argv);

	int mValid = 0;
	const char *devfile = "/dev/fb0";
	long int screensize = 0;
	int fbFd = 0;

	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	char *frameBuffer = 0;

};

};
};
};


#endif

