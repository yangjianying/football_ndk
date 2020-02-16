
#include <iostream>

#define STB_IMAGE_STATIC  // frankie, add
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_STATIC
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "StbImage_.h"

using namespace std;

namespace football {

StbImage_::StbImage_(char const *filename, int tWidth, int tHeight) {
	int iw, ih;
    unsigned char *idata = stbi_load(filename, &iw, &ih, &n, 0);
	fprintf(stderr, "StbImageLoader size: %04d x %04d n = %d \r\n", iw, ih, n);
	w = iw;
	h = ih;

	tw = tWidth;
	th = tHeight;
	if(tw > 0 && th > 0 && (tw != w || th != h)) {
		fprintf(stderr, "  target size is %4d x %4d , resize it! \r\n", tw, th);
		int ow = tw;
		int oh = th;
		auto *odata = (unsigned char *) STBI_MALLOC(ow * oh * n);
		stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0, STBIR_TYPE_UINT8, n, STBIR_ALPHA_CHANNEL_NONE, 0,
			STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
			STBIR_FILTER_BOX, STBIR_FILTER_BOX,
			STBIR_COLORSPACE_SRGB, nullptr
			);
		stbi_image_free(idata);
		idata = odata;
	} else if(tw > 0 && th > 0 && tw == w && th == h) {
		fprintf(stderr, "  target size is the same as original size , no need to resize ! \r\n");
	}
	if (tw < 0) {
		tw = w;
	}
	if (th < 0) {
		th = h;
	}
	
	mData = idata;
}
StbImage_::~StbImage_() {
	if (mData != nullptr) {
		stbi_image_free(mData);
	}
}
int StbImage_::getColor(int x, int y, uint32_t *outColor) {
	uint32_t color_ = 0;
	uint8_t *pixel_byte_ptr = mData + y*tw*n + x*n;
	if (n == 1) {
		color_ = pixel_byte_ptr[0];
		*outColor = color_;
		return 0;
	} else if(n == 2) {
		color_ = pixel_byte_ptr[1];
		color_ <<= 8; color_ += pixel_byte_ptr[0];
		*outColor = color_;
		return 0;
	} else if(n == 3) {  // ok
		color_ = pixel_byte_ptr[2];
		color_ <<= 8; color_ += pixel_byte_ptr[1];
		color_ <<= 8; color_ += pixel_byte_ptr[0];
		*outColor = color_;
		return 0;
	} else if(n == 4) {
		color_ = pixel_byte_ptr[3];
		color_ <<= 8; color_ += pixel_byte_ptr[2];
		color_ <<= 8; color_ += pixel_byte_ptr[1];
		color_ <<= 8; color_ += pixel_byte_ptr[0];
		*outColor = color_;
		return 0;
	}
	return -1;
}

/*static*/ int StbImage_::write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes) {
	return stbi_write_png(filename, x, y, comp, data, stride_bytes);
}
/*static*/ int StbImage_::stbir_resize_simple_uint32_t_com1(const void *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
									 void *output_pixels, int output_w, int output_h, int output_stride_in_bytes, int num_channels) {
	return stbir_resize(input_pixels, input_w, input_h, input_stride_in_bytes, output_pixels, output_w, output_h, output_stride_in_bytes, 
			STBIR_TYPE_UINT32, num_channels, STBIR_ALPHA_CHANNEL_NONE, 0,
				 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
				 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
				 STBIR_COLORSPACE_SRGB, nullptr
				);
}

/*static*/int StbImage_::test1() {
    std::cout << "Hello, STB_Image" << std::endl;

    string inputPath = "/sdcard/Pictures/test/003.jpg_r.png";
    int iw, ih, n;
    
    unsigned char *idata = stbi_load(inputPath.c_str(), &iw, &ih, &n, 0);
	fprintf(stderr, "idata size: %04d x %04d n = %d \r\n", iw, ih, n);

	int ow = 0;
	int oh = 0;

	//
	{
	    ow = iw / 2;
	    oh = ih / 2;
	    auto *odata = (unsigned char *) STBI_MALLOC(ow * oh * n);

	    stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0, STBIR_TYPE_UINT8, n, STBIR_ALPHA_CHANNEL_NONE, 0,
	                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
	                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
	                 STBIR_COLORSPACE_SRGB, nullptr
	    			);
	    string outputPath = "/sdcard/Pictures/test/003.jpg_r_2.png";

	    stbi_write_png(outputPath.c_str(), ow, oh, n, odata, 0);
		stbi_image_free(odata);
		fprintf(stderr, "%s,%d written ! \r\n", __func__, __LINE__);
	}

	//
	{
		ow = (iw*2)/3;
		oh = (ih*2)/3;
		auto *odata = (unsigned char *) STBI_MALLOC(ow * oh * n);
		stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0, STBIR_TYPE_UINT8, n, STBIR_ALPHA_CHANNEL_NONE, 0,
					 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
					 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
					 STBIR_COLORSPACE_SRGB, nullptr
					);
	    string outputPath = "/sdcard/Pictures/test/003.jpg_r_3.png";

	    stbi_write_png(outputPath.c_str(), ow, oh, n, odata, 0);
		stbi_image_free(odata);
		fprintf(stderr, "%s,%d written ! \r\n", __func__, __LINE__);
	}
	//
	{
		ow = iw + (iw*1)/3;
		oh = (ih*2)/3;
		auto *odata = (unsigned char *) STBI_MALLOC(ow * oh * n);
		stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0, STBIR_TYPE_UINT8, n, STBIR_ALPHA_CHANNEL_NONE, 0,
					 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
					 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
					 STBIR_COLORSPACE_SRGB, nullptr
					);
	    string outputPath = "/sdcard/Pictures/test/003.jpg_r_4.png";

	    stbi_write_png(outputPath.c_str(), ow, oh, n, odata, 0);
		stbi_image_free(odata);
		fprintf(stderr, "%s,%d written ! \r\n", __func__, __LINE__);
	}

    stbi_image_free(idata);
    
    return 0;
}

};

