#ifndef __COMPUTEDEMO1_1_MASIAEO1_H___
#define __COMPUTEDEMO1_1_MASIAEO1_H___

/**
 * frankie, 2020.03.21, 
 * SDR -> HDR conversion
 **/

#include "computedemo1_1_Parasite.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t r = 0;
}_buffer_item_r8_t;
typedef struct {
	float r = 0.0f;
}_buffer_item_r32f_t;

typedef struct {
	uint8_t r =0; uint8_t g = 0; uint8_t b = 0; uint8_t a = 0;
}_buffer_item_rgba8_t;
typedef struct {
	float r = 0.0f; float g = 0.0f; float b = 0.0f; float a = 0.0f;
}_buffer_item_rgba32f_t;

typedef struct {
	_buffer_item_rgba32f_t x;
}_buffer_item_t;

#ifdef __cplusplus
};
#endif

namespace computedemo1_1 {

// this also defines the layout of the "buffer" accessed from shader !
#define MasiaEO1_statistic_buffer_c_BIN_SIZE 256
struct MasiaEO1_statistic_buffer_c{
	//// step1
	uint32_t imNumPixels = 0;
	uint32_t constNumOfPixels = 0;
	float constMaxLuminance = 1000.0;

	float p_ov_factor = 254.0f/255.0f;  // 0 ~ 1.0f
	uint32_t p_ov_NumPixels = 0;

	// for MaxQuart
	uint32_t histoBinCount[MasiaEO1_statistic_buffer_c_BIN_SIZE];

	// for logMean
	float delta_ = 0.000001; // 1e-6 
	float log_delta_L_div_numPixels_sum_magnify = 100000.0f;
	int32_t log_delta_L_div_numPixels_sum = 0;	// sum : log(img + delta) /numPixels (1088 x 2352 = 2,558,976)
				// log(1e-6) = -13.8155
				// log(0.999999) = -1e-6

	//// step 2
	// for MaxQuart
	uint32_t cal_counter = 0;
	uint32_t histoBinAccCount[MasiaEO1_statistic_buffer_c_BIN_SIZE];
	float histoBinAccNormal[MasiaEO1_statistic_buffer_c_BIN_SIZE]; // 0.0 ~ 1.0f

		//
		float percentile_for_MaxQuart = 0.01f; // 0 ~ 1.0f
		uint32_t iMinL = 0; // 0 ~ MasiaEO1_statistic_buffer_c_BIN_SIZE
		uint32_t iMaxL = 0; // 0 ~ MasiaEO1_statistic_buffer_c_BIN_SIZE
		float fMinL = 0.0f;
		float fMaxL = 0.0f;

		float p_ov = 0.0f; // (p_ov_NumPixels/imNumPixels) * 100.0
		float Lav = 0.0f; // 0 ~ 1.0f  // Lav = exp(log_delta_L_div_numPixels_sum / log_delta_L_div_numPixels_sum_magnify )
		
		float key = 0.0f; // key = ( log(Lav) - log(minL) )/( log(maxL) - log(minL))

		float m_gamma = 0.0f; // m_gamma = 2.4379 + 0.2319 * log(Lav) - 1.1228 * key +  0.0085 * p_ov;

	///

};

class MasiaEO1: public VulkanExample_Parasite {
public:
	MasiaEO1(VulkanExample *vulkanExample);
	virtual ~MasiaEO1();

	void getComputeQueue() ;
	void createTextureTarget(
		vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags) ;

	void createDescriptorPool();

	void buildCommandBuffers();

	// external 
	virtual void prepare();

	virtual bool isTextureTargetAvailable();

	virtual vks::Texture2D & getTextureTarget();
	
	virtual void appendCommandBuffers(const VkCommandBuffer commandBuffer);

	virtual void draw();

	virtual void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor);

	virtual void setDebugWindow(int enable);

	virtual void setBHE_factor(float f0, float f1);
	virtual void setBHE_tuning(float t0, float t1) {}


public:
	uint32_t mQueueFamilyIndex = 0;	// Family index of the comute queue, used for barriers
	VkQueue mQueue = VK_NULL_HANDLE; 

	VkFence mFence = VK_NULL_HANDLE;
	VkFence mFence_draw = VK_NULL_HANDLE;

	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE; 

	Size_ mSourceTextureSize;

	Size_ mSourceAlignedSize;
	vks::Texture2D mSourceAligned;

	Size_ mSourceScaledSize;
	vks::Texture2D mSourceScaled;

	// for HDR buffer
	uint32_t buffer_memory_property_ = 0;
	vks::Buffer mSourceAlignedBuffer;
	Size_ mSourceAlignedBufferSize;
	VkDeviceSize mSourceAlignedBuffer_size = 0;
	VkBufferView mSourceAlignedBufferView = VK_NULL_HANDLE;
#define TMP__buffer_item_t_NUM (256)
	_buffer_item_t mSourceAlignedBuffer_tmp_buffer[TMP__buffer_item_t_NUM];
	void print__tmp_buffer(_buffer_item_rgba32f_t *buf, int num);
	void print__tmp_buffer(_buffer_item_rgba8_t *buf, int num);

	// L buffer
	uint32_t Lbuffer_memory_property_ = 0;
	vks::Buffer mLbuffer;
	Size_ mLbufferSize;
	VkDeviceSize mLbuffer_memory_size = 0;
	VkBufferView mLbuffer_bufferView = VK_NULL_HANDLE;
#define mLbuffer_tmp_buffer_SIZE (256)
	_buffer_item_r32f_t mLbuffer_tmp_buffer[mLbuffer_tmp_buffer_SIZE];
	void print__tmp_buffer(_buffer_item_r32f_t *buf, int num);
	void print__tmp_buffer(_buffer_item_r8_t *buf, int num);


	uint32_t mStatisticBuffer_memory_property_ = 0;
	vks::Buffer mStatisticBuffer;
	MasiaEO1_statistic_buffer_c mStatisticHostBuffer;

	// L image
	Size_ mLImageSize_;
	vks::Texture2D mLImage;

	// HDR image
	Size_ mTextureTargetSize;
	vks::Texture2D mTextureTarget; // image after processing !


	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE; // Compute shader binding layout
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;	// Layout of the compute pipeline
	VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;	// Compute shader bindings

	//
	enum {
		COMP_PROC_L,
		COMP_GEN_GAMMA,
		COMP_GEN_HDR,
	};
	struct Shader_ {
		int stage = 0;
		std::string name = "";
		const char *source;
	};
	football::MapKeyedInt<Shader_> mShaders; // frankie, add
	football::MapKeyedInt<VkPipeline> mPipelines;

//
	void test_VkFormatProperties(VkFormat format, const char *format_str);
	void test_find_format_for_FORMAT_FEATURE_(VkFormatFeatureFlags format_feature_bit, const char *desc);

};


};



#endif

