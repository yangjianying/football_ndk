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

#include "FootballConfig.h"

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPCpu.h"
#include "FootballPPTester.h"

#undef __CLASS__
#define __CLASS__ "FootballPPCpu"

namespace football {

#if 1

#define MAX_PATH_LEN 256
#ifdef WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

static int32_t createDirectory(const std::string &directoryPath) {
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH_LEN) {
        return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i) {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/') {
            if (ACCESS(tmpDirPath, 0) != 0) {
				DLOGD( "createDirectory:%s \r\n", tmpDirPath);
                int32_t ret = MKDIR(tmpDirPath);
                if (ret != 0) {
					DLOGD( "%s,%d error! \r\n", __func__, __LINE__);
                    return ret;
                }
            }
        }
    }
	DLOGD( "%s ok! \r\n", __func__);
    return 0;
}

class TestColorGenerator {
public:
#define _color_values_SIZE 10 // (sizeof(_color_values)/sizeof(_color_values[0]))
		
	TestColorGenerator(): _color_values{
		// ABGR
		0xff0000ff,
		0xff00ff00, 
		0xffff0000, 
		0xff00ffff, 
		0xffffff00, 
		0xffff00ff, 
		0xff000000, 
		0xffffffff, 
		0x80ffffff, // alpha channel not works
		0x0,
		} {

	}
	~TestColorGenerator() {

	}
	uint32_t getColor() {
		uint32_t r_clolor = 0;
		if (_color_index >= _color_values_SIZE) {
			_color_index = 0;
		}
		r_clolor = _color_values[_color_index];
		_color_index++;
		return r_clolor;
	}
	void resetColor() {
		_color_index = 0;
	}

	uint32_t _color_index = 0;
	uint32_t _color_values[_color_values_SIZE] = {0};

};
#endif

static void print_block_data(long *buf, int w_, int h_) {
	int line_end = 0;
	int line_num = w_;
	for(int line = 0; line < h_; line++) {
		for(uint32_t i = 0; i < w_; i++ ) {
			line_end = 0;
			DLOGD( "%4ld ", buf[line*w_ + i]);
			if(i % line_num == (line_num - 1)) {
				line_end = 1;
				DLOGD( "\r\n");
			}
		}
		if (line_end == 0) {
			DLOGD( "\r\n");
		}
	}
}
static void print_block_data(int *buf, int w_, int h_) {
	int line_end = 0;
	int line_num = w_;
	for(int line = 0; line < h_; line++) {
		for(uint32_t i = 0; i < w_; i++ ) {
			line_end = 0;
			DLOGD( "%4d ", buf[line*w_ + i]);
			if(i % line_num == (line_num - 1)) {
				line_end = 1;
				DLOGD( "\r\n");
			}
		}
		if (line_end == 0) {
			DLOGD( "\r\n");
		}
	}
}
static void print_block_data(double *buf, int w_, int h_) {
	int line_end = 0;
	int line_num = w_;
	for(int line = 0; line < h_; line++) {
		for(uint32_t i = 0; i < w_; i++ ) {
			line_end = 0;
			DLOGD( "%2.5f ", buf[line*w_ + i]);
			if(i % line_num == (line_num - 1)) {
				line_end = 1;
				DLOGD( "\r\n");
			}
		}
		if (line_end == 0) {
			DLOGD( "\r\n");
		}
	}
}
static void print_block_data(int *buf, int w_, int h_, int x, int y, int width, int height) {
	int line_end = 0;
	int line_num = width;
	for(int line = y; line < h_ && line < (y + height); line++) {
		for(uint32_t i = x; i < w_ && i < (x + width); i++ ) {
			line_end = 0;
			DLOGD( "%4d ", buf[line*w_ + i]);
			if(i % line_num == (line_num - 1)) {
				line_end = 1;
				DLOGD( "\r\n");
			}
		}
		if (line_end == 0) {
			DLOGD( "\r\n");
		}
	}
}

FootballPPCpu::FootSessionCpu::FootSessionCpu(SessionInfo &session):
	ImageReaderImageListenerWrapper()
		, mSessionInfo(session)
		, mTestColorGenerator(new TestColorGenerator()) {

	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	::football::MemTrace::print();

	if (mSessionInfo.final_image == nullptr
		|| mSessionInfo.backlight_data == nullptr) {
		DLOGD( "%s,%d error\r\n", __func__, __LINE__);
		return ;
	}
	int32_t width_ = 0;
	int32_t height_ = 0;
	int32_t format_ = 0;
	int32_t maxImages_ = 0;

	width_ = ANativeWindow_getWidth(mSessionInfo.final_image);
	height_ = ANativeWindow_getHeight(mSessionInfo.final_image);
	format_ = ANativeWindow_getFormat(mSessionInfo.final_image);
	if (mSessionInfo.width != width_) {
		mSessionInfo.width = width_;
	}
	if (mSessionInfo.height != height_) {
		mSessionInfo.height = height_;
	}
	if (mSessionInfo.format != format_) {
		mSessionInfo.format = format_;
		mSessionInfo.format = AIMAGE_FORMAT_RGBA_8888;
	}

	width_ = ANativeWindow_getWidth(mSessionInfo.backlight_data);
	height_ = ANativeWindow_getHeight(mSessionInfo.backlight_data);
	format_ = ANativeWindow_getFormat(mSessionInfo.backlight_data);
	if (mSessionInfo.bl_width != width_) {
		mSessionInfo.bl_width = width_; 
	}
	if (mSessionInfo.bl_height != height_) {
		mSessionInfo.bl_height = height_; 
	}
	if (mSessionInfo.bl_format != format_) {
		mSessionInfo.bl_format = format_; 
	}

#if 1
{
	mHardwareBufferReader = new HardwareBufferReader(this, mSessionInfo.width, mSessionInfo.height, mSessionInfo.format
		, 3
		, AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
			//| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER
			| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
			//| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
		, HardwareBufferReader::PROC_IMAGE
	);
	mSessionInfo.input_window = mHardwareBufferReader->getANativeWindow();
	mSessionInfo.input_window = mSessionInfo.input_window;
}
#endif
	DLOGD( "# session window size: %04d x %04d format:%08x / bl data size:%04d x %04d format:%08x \r\n", 
		mSessionInfo.width, mSessionInfo.height, mSessionInfo.format,
		mSessionInfo.bl_width, mSessionInfo.bl_height, mSessionInfo.bl_format);

	// maxvalue2pwm
	for(int i=0;i<256;i++) {
		maxvalue2pwm[i] = 0;
	}
	for(int i=1;i<256;i++) {
		if ( i >= 30) {
			maxvalue2pwm[i] = i;
		} else {
			maxvalue2pwm[i] = 30;
		}
	}
	DLOGD( "maxvalue2pwm: \r\n");
	print_block_data(maxvalue2pwm, 16, 16);

	// back_light_gamma
	for(int i=0;i<256;i++) {
		back_light_gamma[i] = (i*4095)/255;
	}
	DLOGD( "back_light_gamma: \r\n");
	print_block_data(back_light_gamma, 16, 16);

	// gamma_table
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%gamma table&&&&&&&&&&&&&&&&&&&&&&&
Q_G=12;
a =2.2;                %gamma
x=0:255;
x=x./255;
y=(x.^a).*(2^Q_G-1);
gamma_table=uint32(round(y));  % 0~255 -> 0~4095.ROM table
*/
	const double a_ = 2.2f;
	for(int i=0;i<256;i++) {
		// double pow(double x,double y)
		gamma_table[i] = (long)(pow(((double)i/255.0f), a_) * 4095.0f); 
	}
	DLOGD( "gamma_table: \r\n");
	print_block_data(gamma_table, 16, 16);

	// spread_tab
/*
%%%%%%backlight luma curve
spread_tab = uint32(0:1:255);
spread_tab = spread_tab*0.8+51;
*/
	for(int i=0;i<256;i++) {
		spread_tab[i] = (double)(i) * 0.8f + 51.0f;
	}

/*
Q_DEG=8;
c=0:(2^Q_G-1);
d=c/(2^Q_G-1);
d=d.^(1/a);
de_gamma_table=uint32(round(d.*(255*2^(Q_DEG-8))));   %%% 8bit 0~4095 -> 0~255
*/
	for(int i=0;i<4096;i++) {
		de_gamma_table[i] = (pow(((double)i/4095.0f), 1.0f/a_) * 255.0f);
	}
	DLOGD( "de_gamma_table\r\n"
						"        : 0 ~ 255 \r\n");
	print_block_data(de_gamma_table, 16, 16);
	DLOGD( 	"        : 3840 ~ 4095 \r\n");
	print_block_data(de_gamma_table + 3840, 16, 16);

/*
dim_tab = ones(1,256,'double');
dim_tab(110:180) = 1:0.003:1.21;
dim_tab(181:220) = 1.21:0.0025:1.3075;
dim_tab(221:256) = 1.315:-0.009:1;
dim_tab = (dim_tab - 1)*0.3 +1;
*/
{
	for(int i=0;i<109;i++) {
		dim_tab[i] = 1.0f;
	}
	double start = 1.0f;
	double step = 0.003f;
	for(int i=109;i<180;i++, start += step) {
		dim_tab[i] = start;
	}
	start = 1.21f;
	step = 0.0025f;
	for(int i=180;i<220;i++, start += step) {
		dim_tab[i] = start;
	}
	start = 1.315f;
	step = -0.009f;
	for(int i=220;i<=255;i++, start += step) {
		dim_tab[i] = start;
	}
	DLOGD( "dim_tab:  \r\n");
	print_block_data(dim_tab, 16, 16);
}
/*
alph_table(1:101)= 0:0.003:0.3;
alph_table(101:256)=0.3:0.002:0.61;
*/
{
	double start_ = 0.0f;
	double step_ = 0.003f;
	for(int i=0;i<=100;i++, start_+= step_) {
		alph_table[i] = start_;
	}
	start_ = 0.3f;
	step_ = 0.002f;
	for(int i=100;i<256;i++, start_+= step_) {
		alph_table[i] = start_;
	}
}
}
FootballPPCpu::FootSessionCpu::~FootSessionCpu() {
{
	if (mHardwareBufferReader != nullptr) {
		delete mHardwareBufferReader;
		mHardwareBufferReader = nullptr;
	}
}

	delete mTestColorGenerator;

	if (mLastIncommingData != nullptr) {
		delete mLastIncommingData;
		mLastIncommingData = nullptr;
	}
	if (mLastIncommingAlignedData != nullptr) {
		delete mLastIncommingAlignedData;
		mLastIncommingAlignedData = nullptr;
	}
	if (mLastFinalImageData != nullptr) {
		delete mLastFinalImageData;
		mLastFinalImageData = nullptr;
	}

	if (max_value_array != nullptr) {
		FREE_(max_value_array);
	}
	if (means != nullptr) {
		FREE_(means);
	}
	if (led_pwm_array != nullptr) {
		FREE_(led_pwm_array);
	}
	if (led_coeff_space != nullptr) {
		FREE_(led_coeff_space);
	}
	if (led_coeff_gamma != nullptr) {
		FREE_(led_coeff_gamma);
	}
	if (backlight_output != nullptr) {
		FREE_(backlight_output);
	}
	if (vt_bl != nullptr) {
		FREE_(vt_bl);
	}
	if (led_blur != nullptr) {
		FREE_(led_blur);
	}
	if (bl_dim != nullptr) {
		FREE_(bl_dim);
	}
	if (d_indata != nullptr) {
		FREE_(d_indata);
	}
	if (out_data_2 != nullptr) {
		FREE_(out_data_2);
	}
	if (out_data_4 != nullptr) {
		FREE_(out_data_4);
	}
	if (pic_alph != nullptr) {
		FREE_(pic_alph);
	}
	if (out_data != nullptr) {
		FREE_(out_data);
	}

	::football::MemTrace::print();
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}

bool FootballPPCpu::FootSessionCpu::isValid() {
	return mSessionInfo.input_window != nullptr ? true : false;
}


static int ceil(double d) {
	if((d - (int)d) == 0) {
		return (int)d;
	}
	return (int) (d + 1);
}
static int floor(double d) {
	if((d - (int)d) == 0) {
		return (int)d;
	}
	return (int) (d);
}

#define MAX_(a,b) (a > b ? a : b)
int FootballPPCpu::FootSessionCpu::max_mean_value(AImageData *image_data, int x, int y, int w_, int h_, long *max_, long *mean_) {
	memset(max_value_histo, 0, sizeof(max_value_histo));

	for(int line=y;line<image_data->height && line < (y+h_);line++) {
		for(int col=x;col<image_data->width && col < (x+w_);col++) {
			uint32_t color_ = 0;
			image_data->getPixelData(0, col, line, &color_);
			int R = (color_>>0)&0xff;
			int G = (color_>>8)&0xff;
			int B = (color_>>16)&0xff;
			int max_RGB=MAX_(R,G);
            max_RGB=MAX_(max_RGB,B);
			int hist_ind = max_RGB;
			max_value_histo[hist_ind] = max_value_histo[hist_ind]+1;
		}
	}
	int max_value = 0;
	for(int i=255; i >= 0; i--) {
		if(max_value_histo[i] > 5) {
			max_value = i;
			break;
		}
	}
	long sum = 0;
	for(int i=255; i >= 0; i--) {
		sum += max_value_histo[i] * i;
	}
	*max_ = max_value;
	*mean_ = sum / (w_*h_);
	return 0;
}

template<class T, class DT>
static int filter_temp(T *buff, int width, int height, DT *outBuffer, float coeff_a, float coeff_b, float coeff_c) {
/*
%% filter  
%%	a b a	v1 v2 v3
%%	b c b	v4 v5 v6
%%	a b a	v7 v8 v9
*/
#define CHECK_COL if(v_c < 0) {v_c = 0;} else if(v_c >= block_num_width) { v_c = block_num_width-1;}
#define CHECK_ROW if(v_r < 0) {v_r = 0;} else if(v_r >= block_num_height) { v_r = block_num_height-1; }

	int block_num_width = width;
	int block_num_height = height;
	float LD_SPACIAL_FILTER_A = coeff_a;
	float LD_SPACIAL_FILTER_B = coeff_b;
	float LD_SPACIAL_FILTER_C = coeff_c;
	long v1=0,v2=0,v3=0,v4=0,v5 = 0, v6=0,v7=0,v8=0,v9=0;
	int v_c = 0, v_r = 0;
	for(int r=0;r<block_num_height;r++) {
		for(int c=0;c<block_num_width;c++) {
			v_c = c-1; v_r = r - 1; CHECK_COL; CHECK_ROW;
			v1 = buff[v_r*block_num_width + v_c];
			v_c = c; v_r = r - 1; CHECK_COL; CHECK_ROW;
			v2 = buff[v_r*block_num_width + v_c];
			v_c = c + 1; v_r = r - 1; CHECK_COL; CHECK_ROW;
			v3 = buff[v_r*block_num_width + v_c];
			v_c = c-1; v_r = r; CHECK_COL; CHECK_ROW;
			v4 = buff[v_r*block_num_width + v_c];
			v_c = c; v_r = r; //CHECK_COL; CHECK_ROW;
			v5 = buff[v_r*block_num_width + v_c];
			v_c = c+1; v_r = r; CHECK_COL; CHECK_ROW;
			v6 = buff[v_r*block_num_width + v_c];
			v_c = c-1; v_r = r+1; CHECK_COL; CHECK_ROW;
			v7 = buff[v_r*block_num_width + v_c];
			v_c = c; v_r = r+1; CHECK_COL; CHECK_ROW;
			v8 = buff[v_r*block_num_width + v_c];
			v_c = c+1; v_r = r+1; CHECK_COL; CHECK_ROW;
			v9 = buff[v_r*block_num_width + v_c];

			long convolution_ = v5*LD_SPACIAL_FILTER_C
				+ (v1+v3+v7+v9)*LD_SPACIAL_FILTER_A
				+ (v2+v4+v6+v8)*LD_SPACIAL_FILTER_B;
			outBuffer[r*block_num_width + c] = convolution_;

		}
	}
	return 0;
}
static void max__buffer(int *buf0, int *buf1, int width, int height, int *dstBuff) {
	for(int r=0;r<height;r++) {
		for(int c=0;c<width;c++) {
			int b0 = buf0[r*width + c];
			int b1 = buf1[r*width + c];
			dstBuff[r*width + c] = b0 > b1 ? b0 : b1;
		}
	}

}
int FootballPPCpu::FootSessionCpu::led_spread() {
	DLOGD( "%s start. \r\n", __func__);

	const float LD_LED_SP_A1 = 0.0277f;
	const float LD_LED_SP_B1 = 0.1110f;
	const float LD_LED_SP_C1 = 0.4452f;
	
	const float LD_LED_SP_A2 = 0.1070f;
	const float LD_LED_SP_B2 = 0.1131f;
	const float LD_LED_SP_C2 = 0.1196f;

	const float A1 = LD_LED_SP_A1;
	const float B1 = LD_LED_SP_B1;
	const float C1 = LD_LED_SP_C1;
	const float A2 = LD_LED_SP_A2;
	const float B2 = LD_LED_SP_B2;
	const float C2 = LD_LED_SP_C2;

	// generate led_blur (LD_PIC_DATA_HOR * LD_PIC_DATA_VER) from vt_bl (block_num_width * block_num_height)
	int height = block_num_height;
	int width = block_num_width;
	int o_height = 0;
	int o_width = 0;

	DLOGD( "led_blur_0 %4d x %4d \r\n", width, height);
	int *led_blur_0 = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(led_blur_0 != nullptr);
	filter_temp(vt_bl, width, height, led_blur_0, A1, B1, C1);
	max__buffer(vt_bl, led_blur_0, width, height, led_blur_0);

	o_height = height;
	o_width = width;
	height = o_height*2;
	width = o_width*2;
	int *blur_1_r_c = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(blur_1_r_c != nullptr);
	for(int r=0;r<o_height;r++) {
		for(int c=0;c<o_width;c++) {
			blur_1_r_c[(r*2 + 0)*o_width*2 + c*2 + 0] = led_blur_0[r*o_width + c];
			blur_1_r_c[(r*2 + 0)*o_width*2 + c*2 + 1] = led_blur_0[r*o_width + c];
			blur_1_r_c[(r*2 + 1)*o_width*2 + c*2 + 0] = led_blur_0[r*o_width + c];
			blur_1_r_c[(r*2 + 1)*o_width*2 + c*2 + 1] = led_blur_0[r*o_width + c];
		}
	}
	
	DLOGD( "led_blur_2 %4d x %4d \r\n", width, height);
	int *led_blur_2 = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(led_blur_2 != nullptr);
	filter_temp(blur_1_r_c, width, height, led_blur_2, A2, B2, C2);
	max__buffer(blur_1_r_c, led_blur_2, width, height, led_blur_2);

	o_height = height;
	o_width = width;
	height = o_height*2;
	width = o_width*2;
	int *blur_3_r_c = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(blur_3_r_c != nullptr);
	for(int r=0;r<o_height;r++) {
		for(int c=0;c<o_width;c++) {
			blur_3_r_c[(r*2 + 0)*o_width*2 + c*2 + 0] = led_blur_2[r*o_width + c];
			blur_3_r_c[(r*2 + 0)*o_width*2 + c*2 + 1] = led_blur_2[r*o_width + c];
			blur_3_r_c[(r*2 + 1)*o_width*2 + c*2 + 0] = led_blur_2[r*o_width + c];
			blur_3_r_c[(r*2 + 1)*o_width*2 + c*2 + 1] = led_blur_2[r*o_width + c];
		}
	}

	DLOGD( "led_blur_4 %4d x %4d \r\n", width, height);
	int *led_blur_4 = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(led_blur_4 != nullptr);
	filter_temp(blur_3_r_c, width, height, led_blur_4, A2, B2, C2);

	o_height = height;
	o_width = width;
	height = o_height*2;
	width = o_width*2;
	int *blur_5_r_c = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(blur_5_r_c != nullptr);
	for(int r=0;r<o_height;r++) {
		for(int c=0;c<o_width;c++) {
			blur_5_r_c[(r*2 + 0)*o_width*2 + c*2 + 0] = led_blur_4[r*o_width + c];
			blur_5_r_c[(r*2 + 0)*o_width*2 + c*2 + 1] = led_blur_4[r*o_width + c];
			blur_5_r_c[(r*2 + 1)*o_width*2 + c*2 + 0] = led_blur_4[r*o_width + c];
			blur_5_r_c[(r*2 + 1)*o_width*2 + c*2 + 1] = led_blur_4[r*o_width + c];
		}
	}

	DLOGD( "led_blur_6 %4d x %4d \r\n", width, height);
	int *led_blur_6 = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(led_blur_6 != nullptr);
	filter_temp(blur_5_r_c, width, height, led_blur_6, A2, B2, C2);

	o_height = height;
	o_width = width;
	height = o_height*2;
	width = o_width*2;
	int *blur_7_r_c = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(blur_7_r_c != nullptr);
	for(int r=0;r<o_height;r++) {
		for(int c=0;c<o_width;c++) {
			blur_7_r_c[(r*2 + 0)*o_width*2 + c*2 + 0] = led_blur_6[r*o_width + c];
			blur_7_r_c[(r*2 + 0)*o_width*2 + c*2 + 1] = led_blur_6[r*o_width + c];
			blur_7_r_c[(r*2 + 1)*o_width*2 + c*2 + 0] = led_blur_6[r*o_width + c];
			blur_7_r_c[(r*2 + 1)*o_width*2 + c*2 + 1] = led_blur_6[r*o_width + c];
		}
	}

	DLOGD( "led_blur_8 %4d x %4d \r\n", width, height);
	int *led_blur_8 = (int*)MALLOC_(width*height*sizeof(int) + 256);
	assert(led_blur_8 != nullptr);
	filter_temp(blur_7_r_c, width, height, led_blur_8, A2, B2, C2);

#if 1
{
	int input_pixels_SIZE = width*height*sizeof(uint32_t) + 256;
	uint32_t *input_pixels = (uint32_t *)MALLOC_(input_pixels_SIZE);
	assert(input_pixels != nullptr);
	for(int y=0;y<height;y++) {
		for(int x=0;x<width;x++) {
			input_pixels[y*width + x] = (uint32_t)led_blur_8[y*width + x];
		}
	}
	int output_pixels_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*sizeof(uint32_t) + 256;
	uint32_t *output_pixels = (uint32_t *)MALLOC_(output_pixels_SIZE);
	assert(output_pixels != nullptr);
	memset(output_pixels, 0, output_pixels_SIZE);
	
	StbImage_::stbir_resize_simple_uint32_t_com1((const void *) input_pixels, width, height, 0,
		(void *) output_pixels, LD_PIC_DATA_HOR, LD_PIC_DATA_VER, 0, 
		1);
	
	filter_temp(output_pixels, LD_PIC_DATA_HOR, LD_PIC_DATA_VER, led_blur, A2, B2, C2);

	FREE_(input_pixels);
	FREE_(output_pixels);
}
#else  // have problem !!!
{
	int *idx_r = (int*)MALLOC_(LD_PIC_DATA_VER*sizeof(int) + 256);
	assert(idx_r != nullptr);
	int *idx_c = (int*)MALLOC_(LD_PIC_DATA_HOR*sizeof(int) + 256);
	assert(idx_c != nullptr);
	for(int i=0;i<LD_PIC_DATA_VER;i++) {
		idx_r[i] = ceil(((double)i *16.0f * (double)block_num_height)/(((double)LD_PIC_DATA_VER) + 0.001f));
	}
	for(int i=0;i<LD_PIC_DATA_HOR;i++) {
		idx_c[i] = ceil(((double)i *16.0f * (double)block_num_width)/(((double)LD_PIC_DATA_HOR) + 0.001f));
	}
	int *blur_data = (int*)MALLOC_(LD_PIC_DATA_HOR*LD_PIC_DATA_VER*sizeof(int) + 256);
	assert(blur_data != nullptr);
	for(int r=0;r<LD_PIC_DATA_VER;r++) {
		for(int c=0;c<LD_PIC_DATA_HOR;c++) {
			int b8_r = idx_r[r]; if(b8_r >= LD_PIC_DATA_VER) { b8_r = LD_PIC_DATA_VER - 1; } else if(b8_r < 0) { b8_r = 0; }
			int b8_c = idx_c[c]; if(b8_c >= LD_PIC_DATA_HOR) { b8_c = LD_PIC_DATA_HOR - 1; } else if(b8_c < 0) { b8_c = 0; }
			blur_data[r*LD_PIC_DATA_HOR + c] = led_blur_8[b8_r*LD_PIC_DATA_HOR + b8_c];
		}
	}
	filter_temp(blur_data, LD_PIC_DATA_HOR, LD_PIC_DATA_VER, led_blur, A2, B2, C2);
	
	FREE_(idx_r);
	FREE_(idx_c);
	FREE_(blur_data);
-}
#endif
	FREE_(led_blur_0);
	FREE_(blur_1_r_c);
	FREE_(led_blur_2);
	FREE_(blur_3_r_c);
	FREE_(led_blur_4);
	FREE_(blur_5_r_c);
	FREE_(led_blur_6);
	FREE_(blur_7_r_c);
	FREE_(led_blur_8);

	DLOGD( "%s done! \r\n", __func__);
	return 0;
}
void FootballPPCpu::FootSessionCpu::processImage1(AImageData *image_data) {
	LD_BLOCK_NUM_HOR = mSessionInfo.bl_width;
	LD_BLOCK_NUM_VER = mSessionInfo.bl_height;
	
	LD_PIC_DATA_HOR = mSessionInfo.width;
	LD_PIC_DATA_VER = mSessionInfo.height;
	
	LD_BLK_SIZE_HOR = ceil((double)LD_PIC_DATA_HOR / (double)LD_BLOCK_NUM_HOR);
	LD_BLK_SIZE_VER = ceil((double)LD_PIC_DATA_VER / (double)LD_BLOCK_NUM_VER);

	int aligned_width = LD_BLK_SIZE_HOR*LD_BLOCK_NUM_HOR;
	int aligned_height = LD_BLK_SIZE_VER*LD_BLOCK_NUM_VER;

	int deltaHor_l = floor(((double)aligned_width - (double)LD_PIC_DATA_HOR)/2.0f);
	int deltaVer_u = floor(((double)aligned_height - (double)LD_PIC_DATA_VER)/2.0f);

	DLOGD( "bl size:%dx%d pic size:%dx%d blk size:%dx%d aligned:%dx%d delta:%d,%d \r\n", 
		LD_BLOCK_NUM_HOR, LD_BLOCK_NUM_VER, LD_PIC_DATA_HOR, LD_PIC_DATA_VER,
		LD_BLK_SIZE_HOR, LD_BLK_SIZE_VER, aligned_width, aligned_height, deltaHor_l, deltaVer_u);
	//bl size:15x31 pic size:1080x1920 blk size:72x62 aligned:1080x1922 delta:0,1


	if (mLastFinalImageData != nullptr) {
		delete mLastFinalImageData;
		mLastFinalImageData = nullptr;
	}
	mLastFinalImageData = new AImageData(image_data);

	if (mLastIncommingAlignedData != nullptr) {
		delete mLastIncommingAlignedData;
		mLastIncommingAlignedData = nullptr;
	}
	mLastIncommingAlignedData = new AImageData(aligned_width, aligned_height);

	AImageData *alignedData = mLastIncommingAlignedData;
{
	int line = deltaVer_u;
	for(;line<(deltaVer_u+LD_PIC_DATA_VER); line++) {
		int col = deltaHor_l;
		for(;col<(deltaHor_l + LD_PIC_DATA_HOR); col++) {
			uint32_t color_ = 0xff000000;
			image_data->getPixelData(0, col-deltaHor_l, line-deltaVer_u, &color_);
			alignedData->setPixelData(0, col, line, color_);
		}
	}
	line = 0;
	for(;line<deltaVer_u; line++) {
		int col = deltaHor_l;
		for(;col<(deltaHor_l + LD_PIC_DATA_HOR); col++) {
			uint32_t color_ = 0xff000000;
			image_data->getPixelData(0, col, 0, &color_);
			alignedData->setPixelData(0, col, line, color_);
		}
	}
	line = deltaVer_u+LD_PIC_DATA_VER;
	for(;line<aligned_height; line++) {
		int col = deltaHor_l;
		for(;col<(deltaHor_l + LD_PIC_DATA_HOR); col++) {
			uint32_t color_ = 0xff000000;
			image_data->getPixelData(0, col, LD_PIC_DATA_VER-1, &color_);
			alignedData->setPixelData(0, col, line, color_);
		}
	}
	int column = 0;
	for(;column<deltaHor_l;column++) {
		int row = 0;
		for(;row < aligned_height; row++) {
			uint32_t color_ = 0xff000000;
			alignedData->getPixelData(0, deltaHor_l, row, &color_);
			alignedData->setPixelData(0, column, row, color_);
		}
	}
	column = deltaHor_l +LD_PIC_DATA_HOR ;
	for(;column<aligned_width;column++) {
		int row = 0;
		for(;row < aligned_height; row++) {
			uint32_t color_ = 0xff000000;
			alignedData->getPixelData(0, deltaHor_l +LD_PIC_DATA_HOR - 1, row, &color_);
			alignedData->setPixelData(0, column, row, color_);
		}
	}
}

	block_height=LD_BLK_SIZE_VER;
	block_width=LD_BLK_SIZE_HOR;
	block_num_height=floor((double)aligned_height/(double)block_height);
	block_num_width=floor((double)aligned_width/(double)block_width);
	DLOGD( "block size:%dx%d num:%dx%d \r\n", block_width, block_height, block_num_width, block_num_height);

	int max_means_array_size = block_num_width*block_num_height*sizeof(int) + 256;
	DLOGD( "max_means_array_size:%d \r\n", max_means_array_size);

	if (max_value_array != nullptr) {
		FREE_(max_value_array);
	}
	max_value_array = (int*)MALLOC_(max_means_array_size);
	assert(max_value_array != nullptr);
	memset(max_value_array, 0, max_means_array_size);
	
	if (means != nullptr) {
		FREE_(means);
	}
	means = (int*)MALLOC_(max_means_array_size);
	assert(means != nullptr);
	memset(means, 0, max_means_array_size);

	if (led_pwm_array != nullptr) {
		FREE_(led_pwm_array);
	}
	led_pwm_array = (int*)MALLOC_(max_means_array_size);
	assert(led_pwm_array != nullptr);
	memset(led_pwm_array, 0, max_means_array_size);

	if (led_coeff_space != nullptr) {
		FREE_(led_coeff_space);
	}
	led_coeff_space = (int*)MALLOC_(max_means_array_size);
	assert(led_coeff_space != nullptr);
	memset(led_coeff_space, 0, max_means_array_size);

	if (led_coeff_gamma != nullptr) {
		FREE_(led_coeff_gamma);
	}
	led_coeff_gamma = (int*)MALLOC_(max_means_array_size);
	assert(led_coeff_gamma != nullptr);
	memset(led_coeff_gamma, 0, max_means_array_size);

	if (backlight_output != nullptr) {
		FREE_(backlight_output);
	}
	backlight_output = (int*)MALLOC_(max_means_array_size);
	assert(backlight_output != nullptr);
	memset(backlight_output, 0, max_means_array_size);

	if (vt_bl != nullptr) {
		FREE_(vt_bl);
	}
	vt_bl = (int*)MALLOC_(max_means_array_size);
	assert(vt_bl != nullptr);
	memset(vt_bl, 0, max_means_array_size);

	// 
	for(int blk_line=0;blk_line<block_num_height;blk_line++) {
		for(int blk_col = 0; blk_col < block_num_width;blk_col++) {
			int x = blk_col*block_width;
			int y = blk_line*block_height;
			long max_ = 0;
			long mean_ = 0;
			max_mean_value(alignedData, x, y, block_width, block_height, &max_, &mean_);
			max_value_array[blk_line*block_num_width + blk_col] = max_;
			means[blk_line*block_num_width + blk_col] = mean_;
		}
	}
	//DLOGD( "max_value_array: \r\n");
	//print_block_data(max_value_array, block_num_width, block_num_height);
	//DLOGD( "means: \r\n");
	//print_block_data(means, block_num_width, block_num_height);

	static const float LD_STASTIC_MAX_RATIO = 0.95f;

	for(int i=0;i<max_means_array_size;i++) {
		long cur_ = max_value_array[i]*LD_STASTIC_MAX_RATIO + means[i]*(1-LD_STASTIC_MAX_RATIO)
			//+ 1		// ###
			;
		if (cur_ > 255) { cur_ = 255; } else if (cur_ < 0) { cur_ = 0; }
		led_pwm_array[i] = maxvalue2pwm[cur_];
	}
	//DLOGD( "led_pwm_array: \r\n");
	//print_block_data(led_pwm_array, block_num_width, block_num_height);

	static const float LD_SPACIAL_FILTER_A = 0.028f;
	static const float LD_SPACIAL_FILTER_B = 0.042f;
	static const float LD_SPACIAL_FILTER_C = 0.72f;

	// space filter
{
	filter_temp(led_pwm_array, block_num_width, block_num_height, led_coeff_space,
		LD_SPACIAL_FILTER_A, LD_SPACIAL_FILTER_B, LD_SPACIAL_FILTER_C);

	for(int r=0;r<block_num_height;r++) {
		for(int c=0;c<block_num_width;c++) {
			long convolution_ = led_coeff_space[r*block_num_width + c];
			led_coeff_space[r*block_num_width + c] = MAX_(led_pwm_array[r*block_num_width + c], convolution_);

			// %%%%%%%%%%%%%%%%%%% Led Backlight Data output %%%%%%%%%%%%%%%%%%%%
			long coeff_ = led_coeff_space[r*block_num_width + c]; if (coeff_ > 255) { coeff_ = 255; } else if (coeff_ < 0) { coeff_ = 0; }
			coeff_ += 1;
			led_coeff_gamma[r*block_num_width + c] = back_light_gamma[coeff_];

			coeff_ = led_coeff_gamma[r*block_num_width + c];
			backlight_output[r*block_num_width + c] = floor(((double)coeff_+8.0f)/16.0f)-1;
		}
	}
}

	//DLOGD( "led_coeff_space: \r\n");
	//print_block_data(led_coeff_space, block_num_width, block_num_height);

	//DLOGD( "led_coeff_gamma: \r\n");
	//print_block_data(led_coeff_gamma, block_num_width, block_num_height);

	//DLOGD( "backlight_output: \r\n");
	//print_block_data(backlight_output, block_num_width, block_num_height);

	
	const int LD_PEAK_LUM_EN = 1;
	const int LD_PEAK_LUM_BLK_MIN = 6;  //% 2%-20%
	const int LD_PEAK_LUM_BLK_MAX = 115;
	//% LD_PEAK_LUM_BLK_PEAK = 50;
	//% LD_PEAK_LUM_PEAK_STRENTH = 250;
	const int LD_PEAK_LUM_BLK_MID = 56;  //% 
	long LD_PEAK_LUM_NORMAL_SL0 = 190; 
	long LD_PEAK_LUM_NORMAL_SL1 = 210;
	long LD_PEAK_LUM_NORMAL_SL2 = 0;

	if (LD_PEAK_LUM_EN) {
		//peak_lum_1 = uint16(floor(led_pwm_array/255));
		for(int r=0;r<block_num_height;r++) {
			for(int c=0;c<block_num_width;c++) {
			}
		}
		long sum_255_num = 0;
      	//sum_255_num = sum(peak_lum_1(:));
		LD_PEAK_LUM_NORMAL_SL1 =-0.1277f* sum_255_num* sum_255_num+7.1023f*sum_255_num+159.38f;
		LD_PEAK_LUM_NORMAL_SL2 =0.0039f* sum_255_num* sum_255_num-1.37722f*sum_255_num+227.57f;
/*
		if ((sum_255_num > LD_PEAK_LUM_BLK_MAX) || (sum_255_num < LD_PEAK_LUM_BLK_MIN))
			peak_lum = uint8(ones(LD_BLOCK_NUM_VER,LD_BLOCK_NUM_HOR).*LD_PEAK_LUM_NORMAL_SL0);
			
		else
			if sum_255_num < LD_PEAK_LUM_BLK_MID
				peak_lum =uint8(ones(LD_BLOCK_NUM_VER,LD_BLOCK_NUM_HOR).*LD_PEAK_LUM_NORMAL_SL1);			
			else
				peak_lum =uint8(ones(LD_BLOCK_NUM_VER,LD_BLOCK_NUM_HOR).*LD_PEAK_LUM_NORMAL_SL2);
			end
		end
*/
	}

/*
	%%%%%%%%%%%% for debug, make an picture of led backlight data
	for_demo_array=imresize(backlight_output,[LD_PIC_DATA_VER,LD_PIC_DATA_HOR],'nearest');
	MAX=max(for_demo_array(:));
	LED=uint8((double(for_demo_array)/double(MAX))*255);
	out_name=strcat(LED_pic_path, num2str(i), '-LED.bmp');
	imwrite(LED, out_name);
*/



	// %%%%%%%%% adjustment of backlight spread %%%%%%%%%%%%%
	for(int r=0;r<block_num_height;r++) {
		for(int c=0;c<block_num_width;c++) {
			long tmp = backlight_output[r*block_num_width + c]; if(tmp < 0) { tmp = 0; } else if(tmp > 255) { tmp = 255; }
			vt_bl[r*block_num_width + c] = spread_tab[tmp];
		}
	}
	//DLOGD( "vt_bl: \r\n");
	//print_block_data(vt_bl, block_num_width, block_num_height);

/*
	%%%%%%%%%%%%%%%%%  filter   %%%%%%%%%%%%%%%
	Param.a1 = LD_LED_SP_A1;
	Param.b1 = LD_LED_SP_B1;
	Param.c1 = LD_LED_SP_C1;
	Param.a2 = LD_LED_SP_A2;
	Param.b2 = LD_LED_SP_B2;
	Param.c2 = LD_LED_SP_C2;
	led_blur = led_spread(vt_bl, LD_PIC_DATA_VER, LD_PIC_DATA_HOR, Param);
*/
	int led_blur_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*1*sizeof(int) + 256;
	if (led_blur != nullptr) {
		FREE_(led_blur);
	}
	led_blur = (int*)MALLOC_(led_blur_SIZE);
	assert(led_blur != nullptr);
	memset(led_blur, 0x7f, led_blur_SIZE);	// fill with 0xff result -1 value !

	//DLOGD( "led_blur (0,0) ~ (16, 16) \r\n");
	//print_block_data(led_blur, LD_PIC_DATA_HOR, LD_PIC_DATA_VER, 0, 0, 16, 16);

	////////////////////////////////////////////////////////////////////////////////////
	// generate led_blur from vt_bl
	led_spread();

	////////////////////////////////////////////////////////////////////////////////////
/*
	%%%%%%%%%%%% save lum picture	%%%%%%%%%%%%%%%%
	max_lum=max(led_blur(:));
	lum_moni=uint8((double(led_blur)/double(max_lum)*255));
	%lum_moni=uint8((double(lum_sim)/double(max_lum)).^2.2*255);
	lum_moni_name=strcat(lum_moni_path, num2str(i), '_lum_moni.bmp');
	imwrite(lum_moni, lum_moni_name);
*/
#if 0 // save led_blur
	{
		int lum_moni_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(unsigned char) + 256;
		unsigned char *lum_moni = (unsigned char *)MALLOC_(lum_moni_SIZE);
		assert(lum_moni != nullptr);
		int max_lum = 0;
		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				int cur_blur_value = led_blur[y*LD_PIC_DATA_HOR + x];
				max_lum = cur_blur_value > max_lum ? cur_blur_value : max_lum;
			}
		}
		DLOGD( "max_lum : %d \r\n", max_lum);

		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				int cur_blur_value = led_blur[y*LD_PIC_DATA_HOR + x];
				unsigned char b0 = (unsigned char )(((double)cur_blur_value * 255.0f)/(double)max_lum);

			#if 0  // test
				if (y < 100) { b0 = 0x40; }
				else if(y < 400) { b0 = 0x60; }
				else if(y < 800) { b0 = 0x80; }
				else if(y < 1200) { b0 = 0xa0; }
				else if(y < 1600) { b0 = 0xb0; }
				else { b0 = 0xff; }
			#endif
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = b0;
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = b0;
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = b0;
			}
		}

		// "/sdcard/football/out/filename_led_blur.png"
		createDirectory("/sdcard/football/out/");
		StbImage_::write_png("/sdcard/football/out/led_blur.png", 
			LD_PIC_DATA_HOR, LD_PIC_DATA_VER, 3, (const void *)lum_moni, 0);

		FREE_(lum_moni);
	}
#endif
/*
	%%%%%%%%%%%% Compensating the picture %%%%%%%%%%%%%%
	adj_tab = dim_tab(uint16(led_blur)+1);
	bl_dim(:,:,1) = (led_blur+0.1).*adj_tab;
	bl_dim(:,:,2) = bl_dim(:,:,1);
	bl_dim(:,:,3) = bl_dim(:,:,1);
*/
//#define bl_dim_3DIM 1
#ifdef bl_dim_3DIM
	int bl_dim_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(int) + 256;
#else
	int bl_dim_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*1*sizeof(int) + 256;
#endif
	if (bl_dim != nullptr) {
		FREE_(bl_dim);
	}
	bl_dim = (int*)MALLOC_(bl_dim_SIZE);
	assert(bl_dim != nullptr);
	memset(bl_dim, 0, bl_dim_SIZE);
	for(int y=0;y<LD_PIC_DATA_VER;y++) {
		for(int x=0;x<LD_PIC_DATA_HOR;x++) {
			int cur_blur_value = led_blur[y*LD_PIC_DATA_HOR + x];
			int cur_blur_value_index = cur_blur_value;
			if (cur_blur_value_index < 0) { cur_blur_value_index =0; } else if(cur_blur_value_index > 255) { cur_blur_value_index = 255; }
			double adj_tab = dim_tab[cur_blur_value_index];
			double bl_dim_value = ((double)cur_blur_value + 0.1f)*adj_tab;

		#ifdef bl_dim_3DIM
			bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = bl_dim_value;
			bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = bl_dim_value;
			bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = bl_dim_value;
		#else
			bl_dim[y*LD_PIC_DATA_HOR + x + 0] = bl_dim_value;
		#endif
		}
	}

#if 0 // save bl_dim
	{
		int lum_moni_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(unsigned char) + 256;
		unsigned char *lum_moni = (unsigned char *)MALLOC_(lum_moni_SIZE);
		assert(lum_moni != nullptr);
		int max_lum = 0;
		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				int cur_blur_value = bl_dim[y*LD_PIC_DATA_HOR + x];
				max_lum = cur_blur_value > max_lum ? cur_blur_value : max_lum;
			}
		}
		DLOGD( "bl_dim max_lum : %d \r\n", max_lum);

		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
			#ifdef bl_dim_3DIM
				int cur_blur_value = bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
			#elses
				int cur_blur_value = bl_dim[y*LD_PIC_DATA_HOR + x];
			#endif
				unsigned char b0 = (unsigned char )(((double)cur_blur_value * 255.0f)/(double)max_lum);
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = b0;
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = b0;
				lum_moni[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = b0;
			}
		}
		// "/sdcard/football/out/filename_bl_dim.png"
		createDirectory("/sdcard/football/out/");
		StbImage_::write_png("/sdcard/football/out/bl_dim.png", 
			LD_PIC_DATA_HOR, LD_PIC_DATA_VER, 3, (const void *)lum_moni, 0);

		FREE_(lum_moni);
	}
#endif
/*
	%%%%%%%%%%%%% Backlight Dimming %%%%%%%%%%%%%%%%%%%%%
	indata_gamma=double(gamma_table(indata1+1));	// R-0~4095, G-0~4095, B-0~4095
	d_indata=double(indata_gamma);
*/
	int d_indata_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(double) + 256;
	if (d_indata != nullptr) {
		FREE_(d_indata);
	}
	d_indata = (double*)MALLOC_(d_indata_SIZE);
	assert(d_indata != nullptr);
	memset(d_indata, 0, d_indata_SIZE);

	for(int y=0;y<LD_PIC_DATA_VER;y++) {
		for(int x=0;x<LD_PIC_DATA_HOR;x++) {
			uint32_t color_ = 0xff000000;
			image_data->getPixelData(0, x, y, &color_);
			int r = (color_>>0)&0xff;	// r
			int g = (color_>>8)&0xff;	// g
			int b = (color_>>16)&0xff;	// b
			if (r < 0) { r =0; } else if(r > 255) { r = 255; }
			if (g < 0) { g =0; } else if(g > 255) { g = 255; }
			if (b < 0) { b =0; } else if(b > 255) { b = 255; }
			d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = gamma_table[r];
			d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = gamma_table[g];
			d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = gamma_table[b];
		}
	}
/*
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	outdata = d_indata.*((255./bl_dim));			// d_indata * { 1/ (bl_dim/255)} //  bl_dim 0~255
	outdata4 = uint32(min(outdata,4095));
	out_data_2 = uint16(de_gamma_table(outdata4+1));
*/
	int out_data_2_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(int) + 256;
	if (out_data_2 != nullptr) {
		FREE_(out_data_2);
	}
	out_data_2 = (int*)MALLOC_(out_data_2_SIZE);
	assert(out_data_2 != nullptr);
	memset(out_data_2, 0, out_data_2_SIZE);
	for(int y=0;y<LD_PIC_DATA_VER;y++) {
		for(int x=0;x<LD_PIC_DATA_HOR;x++) {
			double R = d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
			double G = d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 1];
			double B = d_indata[y*LD_PIC_DATA_HOR*3 + x*3 + 2];

		#ifdef bl_dim_3DIM
			double dim0 = bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
			double dim1 = bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 1];
			double dim2 = bl_dim[y*LD_PIC_DATA_HOR*3 + x*3 + 2];
		#else
			double dim0 = bl_dim[y*LD_PIC_DATA_HOR + x + 0];
			double dim1 = dim0;
			double dim2 = dim1;
		#endif
			double outdata_;
			outdata_ = (R * 255.0f)/dim0;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = outdata_;
			
			outdata_ = (G * 255.0f)/dim1;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = outdata_;
			
			outdata_ = (B * 255.0f)/dim2;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = outdata_;

		#if 0  // only gamma_table then de_gamma_table back 
			outdata_ = R;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = outdata_;
			
			outdata_ = G;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = outdata_;
			
			outdata_ = B;
			if (outdata_ > 4095) { outdata_ = 4095; } outdata_ = de_gamma_table[(int)outdata_];
			out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = outdata_;
		#endif
		}
	}



	const int LD_COMPENSATION_BLENDING_EN = 0; // 0; // 1;
	if (LD_COMPENSATION_BLENDING_EN) {
/*
    %%%%%%%%%%%%%%% blending  %%%%%%%%%%%%%%%%%%%%%%%%%%%
    out_data_4 = double(indata1);
    pic_alph = alph_table(indata1+1);
    out_data_3 = double(out_data_2);
    out_data = out_data_4.*pic_alph + (1-pic_alph).*out_data_3 ;
*/

		int out_data_4_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(double) + 256;
		if (out_data_4 != nullptr) {
			FREE_(out_data_4);
		}
		out_data_4 = (double*)MALLOC_(out_data_4_SIZE);
		assert(out_data_4 != nullptr);
		memset(out_data_4, 0, out_data_4_SIZE);
		
		int pic_alph_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(double) + 256;
		if (pic_alph != nullptr) {
			FREE_(pic_alph);
		}
		pic_alph = (double*)MALLOC_(pic_alph_SIZE);
		assert(pic_alph != nullptr);
		memset(pic_alph, 0, pic_alph_SIZE);
		
		int out_data_SIZE = LD_PIC_DATA_HOR*LD_PIC_DATA_VER*3*sizeof(double) + 256;
		if (out_data != nullptr) {
			FREE_(out_data);
		}
		out_data = (double*)MALLOC_(out_data_SIZE);
		assert(out_data != nullptr);
		memset(out_data, 0, out_data_SIZE);
		
		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				uint32_t color_ = 0xff000000;
				image_data->getPixelData(0, x, y, &color_);
				int r = (color_>>0)&0xff;	// r
				int g = (color_>>8)&0xff;	// g
				int b = (color_>>16)&0xff;	// b
				if (r < 0) { r =0; } else if(r > 255) { r = 255; }
				if (g < 0) { g =0; } else if(g > 255) { g = 255; }
				if (b < 0) { b =0; } else if(b > 255) { b = 255; }
				out_data_4[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = r;
				out_data_4[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = g;
				out_data_4[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = b;

				pic_alph[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = alph_table[r];
				pic_alph[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = alph_table[g];
				pic_alph[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = alph_table[b];

				out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 0] = (double)r *alph_table[r] + (1-alph_table[r]) *(double)out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
				out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 1] = (double)g*alph_table[g] + (1-alph_table[g])*(double)out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 1];
				out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 2] = (double)b*alph_table[b] + (1-alph_table[b])*(double)out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 2];

			}
		}

		// fill mLastFinalImageData out_data
		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				uint32_t color_ = 0xff;
				color_ <<= 8; color_+= out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 2];
				color_ <<= 8; color_+= out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 1];
				color_ <<= 8; color_+= out_data[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
				mLastFinalImageData->setPixelData(0, x, y, color_);
			}
		}
	}
	else {
		// fill mLastFinalImageData with out_data_2
		for(int y=0;y<LD_PIC_DATA_VER;y++) {
			for(int x=0;x<LD_PIC_DATA_HOR;x++) {
				uint32_t color_ = 0xff;
				color_ <<= 8; color_+= out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 2];
				color_ <<= 8; color_+= out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 1];
				color_ <<= 8; color_+= out_data_2[y*LD_PIC_DATA_HOR*3 + x*3 + 0];
				mLastFinalImageData->setPixelData(0, x, y, color_);
			}
		}

	}

}

void FootballPPCpu::FootSessionCpu::onImageProc1(AImageData *image_data) {
	if (mLastIncommingData != nullptr) {
		delete mLastIncommingData;
		mLastIncommingData = nullptr;
	}
	mLastIncommingData = new AImageData(image_data);

	processImage1(mLastIncommingData);

	//usleep(1*1000*1000);

//#define FILL_BL_DATA_FIRST

#ifdef FILL_BL_DATA_FIRST
	{
		DLOGD( ">>> backlight_data filling ...\r\n");
		ANativeWindow * inputWindow = mSessionInfo.backlight_data;
		fill_ANativeWindow_with_color(inputWindow, 0xffff0000);
	}
#endif
	{
		DLOGD( ">>> final_image filling ...\r\n");
		ANativeWindow * inputWindow = mSessionInfo.final_image;
#if 0
		uint32_t color_ = mTestColorGenerator->getColor();
		fill_ANativeWindow_with_color(inputWindow, color_);
#else
		//fill_ANativeWindow_with_AImageData(inputWindow, image_data);
		//fill_ANativeWindow_with_AImageData(inputWindow, mLastIncommingData);
		fill_ANativeWindow_with_AImageData(inputWindow, mLastFinalImageData);

#endif
	}
#ifndef FILL_BL_DATA_FIRST
	{
		DLOGD( ">>> backlight_data filling ...\r\n");
		ANativeWindow * inputWindow = mSessionInfo.backlight_data;
		//fill_ANativeWindow_with_color(inputWindow, 0xffff0000);
		fill_ANativeWindow_with_buff(inputWindow, (uint8_t *)backlight_output, block_num_width, block_num_height, block_num_width*4, 4);
	}
#endif

}
void FootballPPCpu::FootSessionCpu::processingImage(AImage *image_ ) {
	AImageData *imageData = nullptr;
	imageData = getAImageData_from_AImage(image_);
	if (imageData != nullptr) {
		onImageProc1(imageData);
		delete imageData;
	}
}

// impl public football::ImageReaderImageListenerWrapper
void FootballPPCpu::FootSessionCpu::onImageAvailableCallback(AImageReader *reader) {
	DLOGD( "FootSessionCpu::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
	if (reader != mReader) { DLOGD( "*** impossible to reach here:%s,%d \r\n", __func__, __LINE__); return ;}

	AImageData *imageData = nullptr;
	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		imageData = getAImageData_from_AImage(image_);
		AImage_delete(image_);
	}
	if (imageData != nullptr) {
		onImageProc1(imageData);
		delete imageData;
	}

}
// impl public HardwareBufferReader::CB
int FootballPPCpu::FootSessionCpu::on_process_image(AImage *image_) {
	processingImage(image_);
	return 0;
}

// impl public ::football::FootballPP::FootballSession
int FootballPPCpu::FootSessionCpu::setSessionParameter(SessionParameter *parameter) {
	if (parameter->trigger_request) {
		parameter->trigger_request = 0;

	}

	DLOGD( "%s, have_algo = %d \r\n", __func__, parameter->have_algo);
	mSessionParameter.have_algo = parameter->have_algo;
	return 0;
}
int FootballPPCpu::FootSessionCpu::getSessionParameter(SessionParameter *parameter) {
	*parameter = mSessionParameter;
	return 0;
}
void FootballPPCpu::FootSessionCpu::print() {
	DLOGD( "%s \r\n", __func__);

}



//
FootballPPCpu::FootballPPCpu()
	{
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}

FootballPPCpu::~FootballPPCpu() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	// should close all sessions
}

int FootballPPCpu::buildSession(int session_type, SessionInfo &session, int *session_id) {
	DLOGD( "FootballPPCpu::%s,  session_type: %d \r\n", __func__, session_type);

	FootSessionCpu * session_ = new FootSessionCpu(session);
	if (!session_->isValid()) {
		delete session_;
		DLOGD( "%s,%d not valid\r\n", __func__, __LINE__);
		return -1;
	}
	session_->mId = addSession(session_);
	*session_id = session_->mId;
	DLOGD( "%s id: %d \r\n", __func__, session_->mId);
	return 0;
}


};

