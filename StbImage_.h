#ifndef __STB_IMAGE_LOADER___H___
#define __STB_IMAGE_LOADER___H___



namespace football {

class StbImage_ {
public:
	int w;
	int h;
	int n;
	int tw;
	int th;
	uint8_t *mData = nullptr;
	StbImage_(char const *filename, int tWidth = -1, int tHeight = -1);
	~StbImage_();
	int getColor(int x, int y, uint32_t *outColor);

	static int write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes);
	static int stbir_resize_simple_uint32_t_com1(const void *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                         void *output_pixels, int output_w, int output_h, int output_stride_in_bytes, int num_channels);

	// test
	static int test1();
};

};

#endif

