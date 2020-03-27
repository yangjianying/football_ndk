#ifndef __COMPUTEDEMO1_1_PARASITE_H___
#define __COMPUTEDEMO1_1_PARASITE_H___


#include "computedemo1_1.h"


namespace computedemo1_1 {

/////////////////////////////////////////////////////////////////////////////////////////////
class VulkanExample_Parasite {
public:
	static VulkanExample_Parasite * create(VulkanExample *example, int type);

	VulkanExample_Parasite(VulkanExample *vulkanExample): mVulkanExample(vulkanExample){}
	virtual ~VulkanExample_Parasite() {}

	void setVulkanEnv(VkPhysicalDevice phy_, VkDevice dev_, VkPipelineCache pipecache_) {
		mPhysicalDevice = phy_; mDevice = dev_; mPipelineCache = pipecache_;
	}

	virtual void prepare() = 0;
	virtual bool isTextureTargetAvailable() = 0;
	virtual vks::Texture2D & getTextureTarget() = 0;
	virtual void appendCommandBuffers(const VkCommandBuffer commandBuffer) = 0;
	virtual void draw() = 0;
	virtual void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor) = 0;
	virtual void setDebugWindow(int enable) = 0;
	virtual void setBHE_factor(float f0, float f1) = 0;
	virtual void setBHE_tuning(float t0, float t1) = 0;

public:
	VulkanExample *mVulkanExample = nullptr;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;
	VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
};

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ALIGN_SIZE
#error ALIGN_SIZE is defined in somewhere previously !!!
#endif
#define ALIGN_SIZE(x, a) ((((x)+((a)-1))/(a))* (a))

#define __stringify(x)  __stringify_1(x)
#define __stringify_1(x)  #x

typedef struct Size_ {
	int w = 0;
	int h = 0;
}Size_;

/////////////////////////////////////////////////////////////////////////////////////////////

#define IndirectStats__c_HIST_BIN_SIZE 256 // 256 // 512 // 1024(will result the buffer overflow !!! ) //
struct IndirectStats__c{
	//
	uint32_t drawCount;						// Total number of indirect draw counts to be issued
	uint32_t lodCount[IndirectStats__c_HIST_BIN_SIZE];	// Statistics for number of draws per LOD level (written by compute shader)
	uint32_t luminance_sumed = 0;

	//
	uint32_t accHist[IndirectStats__c_HIST_BIN_SIZE];
	float accHistNormal[IndirectStats__c_HIST_BIN_SIZE];
	uint32_t accCount = 0;
	uint32_t pointIndexFlags[8];
	uint32_t pointIndex[8];	

	//
	uint32_t bhe_lut[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_lut0[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_lut1[IndirectStats__c_HIST_BIN_SIZE];

	//
	uint32_t lumi_counter = 0;
	uint32_t luminance_mean = 0;
	uint32_t luminance_std = 0;
	float lumi_mean = 0.0;
	float lumi_std = 0.0;
	uint32_t type = 0;  // 0, 1, 2, 3
	float y1 = 0.0;
	float y2 = 0.0;
	uint32_t agc_lut[IndirectStats__c_HIST_BIN_SIZE];
	uint32_t agc_counter = 0;
	
	uint32_t merged_lut[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_factor0 = 0.0f;
	float bhe_factor1 = 0.0f;
	float bhe_tuning0 = 1.0f;
	float bhe_tuning1 = 1.0f;

	//
	double lodCount_d[IndirectStats__c_HIST_BIN_SIZE];
	int bin_count = IndirectStats__c_HIST_BIN_SIZE;

	enum {
		PRINT_HIST = 0x01,
		PRINT_ACC_HIST = 0x02,
		PRINT_AGC_LUT = 0x04,
		PRINT_BHE_LUT = 0x08,
		PRINT_MERGED_LUT = 0x10,
	};
	void print(uint32_t flags = PRINT_HIST);
	void print_u32();
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct stdTempBuffer_c {

	stdTempBuffer_c(int w, int h) {
		mWidth = w; mHeight = h; length = sizeof(float)*w*h; buffer = (float *)malloc(length); }
	~stdTempBuffer_c() { if (buffer != nullptr) { free(buffer); buffer = nullptr; } }
	int getLength() { return length; }
	float *getBuffer() { return buffer; }
	
	float *buffer = nullptr;
	int mWidth = 0;
	int mHeight = 0;
	int length = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class HistogramGrapher {
public:
	static HistogramGrapher *create();

	HistogramGrapher() {}
	virtual ~HistogramGrapher() {}

	virtual void setBHE_factor(float f0, float f1) = 0;
	virtual void draw_stats(IndirectStats__c *stats) = 0;
};





};


#endif

