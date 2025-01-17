

#include "Mbi6322Fb.h"

namespace android {
namespace local_bl {
namespace test {

LinuxFb::LinuxFb() {
	/* Open the file for reading and writing */
	fbFd = open (devfile, O_RDWR);
	if (fbFd == -1) {
		perror ("Error: cannot open framebuffer device");
		return ;
	}
	if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		perror ("Error reading fixed information");
		goto __error;
	}
	printFixedInfo ();
	if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror ("Error reading variable information");
		goto __error;
	}
	printVariableInfo ();
	
	/* Figure out the size of the screen in bytes */
	screensize = finfo.smem_len;
	/* Map the device to memory */
	frameBuffer = (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);
	if (frameBuffer == MAP_FAILED) {
		perror ("Error: Failed to map framebuffer device to memory");
		goto __error;
	}
	mValid = 1;
	return ;

__error:
	close (fbFd);
	fbFd = -1;
	return ;
}

LinuxFb::~LinuxFb() {
	if (mValid) {
		munmap (frameBuffer, screensize);
		close (fbFd);
	}
}


void
LinuxFb::printFixedInfo ()
{
   printf ("Fixed screen info:\n"
                        "\tid: %s\n"
                        "\tsmem_start:0x%lx\n"
                        "\tsmem_len:%d\n"
                        "\ttype:%d\n"
                        "\ttype_aux:%d\n"
                        "\tvisual:%d\n"
                        "\txpanstep:%d\n"
                        "\typanstep:%d\n"
                        "\tywrapstep:%d\n"
                        "\tline_length: %d\n"
                        "\tmmio_start:0x%lx\n"
                        "\tmmio_len:%d\n"
                        "\taccel:%d\n"
           "\n",
           finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
           finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
           finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
           finfo.mmio_len, finfo.accel);
}

void
LinuxFb::printVariableInfo ()
{
   printf ("Variable screen info:\n"
                        "\txres:%d\n"
                        "\tyres:%d\n"
                        "\txres_virtual:%d\n"
                        "\tyres_virtual:%d\n"
                        "\tyoffset:%d\n"
                        "\txoffset:%d\n"
                        "\tbits_per_pixel:%d\n"
                        "\tgrayscale:%d\n"
                        "\tred: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tgreen: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tblue: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\ttransp: offset:%2d, length: %2d, msb_right: %2d\n"
                        "\tnonstd:%d\n"
                        "\tactivate:%d\n"
                        "\theight:%d\n"
                        "\twidth:%d\n"
                        "\taccel_flags:0x%x\n"
                        "\tpixclock:%d\n"
                        "\tleft_margin:%d\n"
                        "\tright_margin: %d\n"
                        "\tupper_margin:%d\n"
                        "\tlower_margin:%d\n"
                        "\thsync_len:%d\n"
                        "\tvsync_len:%d\n"
                        "\tsync:%d\n"
                       "\tvmode:%d\n"
           "\n",
           vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
           vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
           vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
            vinfo.red.msb_right,vinfo.green.offset, vinfo.green.length,
           vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
           vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
           vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
           vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
           vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
           vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
           vinfo.sync, vinfo.vmode);
}
 
void
LinuxFb::drawRect_rgb32 (int x0, int y0, int width,int height, int color)
{
   const int bytesPerPixel = 4;
   const int stride = finfo.line_length / bytesPerPixel;
 
   int *dest = (int *) (frameBuffer)
       + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);
 
   int x, y;
   for (y = 0; y < height; ++y)
    {
       for (x = 0; x < width; ++x)
       {
           dest[x] = color;
        }
       dest += stride;
    }
}

void
LinuxFb::drawRect_rgb16 (int x0, int y0, int width,int height, int color)
{
   const int bytesPerPixel = 2;
   const int stride = finfo.line_length / bytesPerPixel;
   const int red = (color & 0xff0000) >> (16 + 3);
   const int green = (color & 0xff00) >> (8 + 2);
   const int blue = (color & 0xff) >> 3;
   const short color16 = blue | (green << 5) | (red << (5 +6));
 
   short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 +vinfo.xoffset);
 
   int x, y;
   for (y = 0; y < height; ++y)
    {
       for (x = 0; x < width; ++x)
       {
           dest[x] = color16;
       }
       dest += stride;
    }
}
 
void
LinuxFb::drawRect_rgb15 (int x0, int y0, int width,int height, int color)
{
   const int bytesPerPixel = 2;
   const int stride = finfo.line_length / bytesPerPixel;
   const int red = (color & 0xff0000) >> (16 + 3);
   const int green = (color & 0xff00) >> (8 + 3);
   const int blue = (color & 0xff) >> 3;
   const short color15 = blue | (green << 5) | (red << (5 + 5))| 0x8000;
 
   short *dest = (short *) (frameBuffer)
       + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);
 
   int x, y;
   for (y = 0; y < height; ++y)
    {
       for (x = 0; x < width; ++x)
       {
           dest[x] = color15;
       }
       dest += stride;
    }
}
 
void
LinuxFb::drawRect (int x0, int y0, int width, int height, int color)
{
   switch (vinfo.bits_per_pixel)
    {
   case 32:
       drawRect_rgb32 (x0, y0, width, height, color);
       break;
   case 16:
       drawRect_rgb16 (x0, y0, width, height, color);
       break;
   case 15:
       drawRect_rgb15 (x0, y0, width, height, color);
       break;
   default:
       printf ("Warning: drawRect() not implemented for color depth%i\n",
                vinfo.bits_per_pixel);
       break;
    }
}
 
#define PERFORMANCE_RUN_COUNT 5
void
LinuxFb::performSpeedTest (void *fb, int fbSize)
{
   int i, j, run;
   struct timeval startTime, endTime;
   unsigned long long results[PERFORMANCE_RUN_COUNT];
   unsigned long long average;
   unsigned int *testImage;
 
   unsigned int randData[17] = {
        0x3A428472, 0x724B84D3, 0x26B898AB,0x7D980E3C, 0x5345A084,
       0x6779B66B, 0x791EE4B4, 0x6E8EE3CC, 0x63AF504A, 0x18A21B33,
       0x0E26EB73, 0x022F708E, 0x1740F3B0, 0x7E2C699D, 0x0E8A570B,
       0x5F2C22FB, 0x6A742130
   };
 
   printf ("Frame Buffer Performance test...\n");
   for (run = 0; run < PERFORMANCE_RUN_COUNT; ++run)
    {
       /* Generate test image with random(ish) data: */
       testImage = (unsigned int *) malloc (fbSize);
       j = run;
       for (i = 0; i < (int) (fbSize / sizeof (int)); ++i)
       {
           testImage[i] = randData[j];
           j++;
           if (j >= 17)
                j = 0;
       }
 
       gettimeofday (&startTime, NULL);
       memcpy (fb, testImage, fbSize);
       gettimeofday (&endTime,NULL);
 
       long secsDiff = endTime.tv_sec - startTime.tv_sec;
		results[run] = secsDiff * 1000000 +(endTime.tv_usec - startTime.tv_usec);
 
       free (testImage);
    }
 
   average = 0;
   for (i = 0; i < PERFORMANCE_RUN_COUNT; ++i)
       average += results[i];
   average = average / PERFORMANCE_RUN_COUNT;
 
   printf (" Average: %llu usecs\n", average);
   printf (" Bandwidth: %.03f MByte/Sec\n",
           (fbSize / 1048576.0) / ((double) average / 1000000.0));
   printf (" Max. FPS: %.03f fps\n\n",
           1000000.0 / (double) average);
 
   /* Clear the framebuffer back to black again: */
   memset (fb, 0, fbSize);
}
 
int
LinuxFb::test_main (int argc, char **argv) {
	performSpeedTest (frameBuffer, screensize);

	printf ("Will draw 3 rectangles on the screen,\n"
			"they should be coloredred, green and blue (in that order).\n");
	drawRect (vinfo.xres / 8, vinfo.yres / 8, 			vinfo.xres / 4, vinfo.yres /4, 0xffff0000);
	drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8, 	vinfo.xres / 4, vinfo.yres /4, 0xff00ff00);
	drawRect (vinfo.xres * 5 / 8, vinfo.yres * 5 / 8, 	vinfo.xres / 4, vinfo.yres /4, 0xff0000ff);
	printf (" Done.\n");
	return 0;
}





};
};
};


