#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>

#include <sys/resource.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()
//#include<linux/fb.h>
#include <errno.h>

#if 1  // for userspace spidev
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>


#if 0
#include <xf86drm.h>
#include <xf86drmMode.h>
// Intentionally included after xf86 headers so that they in-turn include libdrm version of drm.h
// that doesn't use keyword "virtual" for a variable name. Not doing so leads to the kernel version
// of drm.h being included causing compilation to fail
#include "drm/msm_drm.h"
#endif

//#include "CyclingColorConfig.h"

//#include "cycling_utils/CyclingUtils.h"
//#include "cycling_utils/cy_utils.h"
//#include "Test1.h"


#include "CmdLineUtils.h"

#include "TestCmdMiniLed.h"

#include "mbi6322_reference_settings.h"

#if 0

#undef JNI_FR_DEBUG_H_
#if 1  // must @ last
#undef config_fr_DEBUG_EN
#undef config_fr_DEBUG_TO_CONSOLE
#define config_fr_DEBUG_EN 1  //  (1)
#define config_fr_DEBUG_TO_CONSOLE 1  //  1 //  0
#include "cycling_utils/cy_debug.h"
#endif
#undef LOG_TAG
#define LOG_TAG "TestCmdMiniLed"

#endif

#undef UNUSED_
#define UNUSED_(x) ((void)x)

#undef LOG_TAG
#define LOG_TAG stderr


#define WITH_HW_SPI (1)

namespace NS_test_miniled {

/*

*/

#define GPIOSYS_PINCTRL_BASE (1134)
#define GPIOSYS_PINCTRL_GPIO(x) (GPIOSYS_PINCTRL_BASE + (x))
#define PIN_VLED_DCDC_EN 			GPIOSYS_PINCTRL_GPIO(5)
#define PIN_VDD_LDO_EN 				GPIOSYS_PINCTRL_GPIO(59)
#define PIN_VDDIO_LDO_EN 			GPIOSYS_PINCTRL_GPIO(60)
#define PIN_CHIP_EN 				GPIOSYS_PINCTRL_GPIO(61)
#define PIN_CHIP_MTP_EN_1 			GPIOSYS_PINCTRL_GPIO(62)
#define PIN_CHIP_MTP_EN_2 			GPIOSYS_PINCTRL_GPIO(67)
#define PIN_CHIP_MTP_EN_3 			GPIOSYS_PINCTRL_GPIO(68)
#define PIN_CHIP_SYNC_1 			GPIOSYS_PINCTRL_GPIO(56)
#define PIN_CHIP_SYNC_2 			GPIOSYS_PINCTRL_GPIO(66)

#define PIN_CHIP_MISO 			GPIOSYS_PINCTRL_GPIO(0)
#define PIN_CHIP_MOSI 			GPIOSYS_PINCTRL_GPIO(1)
#define PIN_CHIP_SCLK 			GPIOSYS_PINCTRL_GPIO(2)
#define PIN_CHIP_CS1_N 			GPIOSYS_PINCTRL_GPIO(3)
#define PIN_CHIP_CS2_N 			GPIOSYS_PINCTRL_GPIO(12)


static const char *s_gpiosys_path_export = "/sys/class/gpio/export";
static const char *s_gpiosys_path_unexport = "/sys/class/gpio/unexport";

static const char *s_VALUE_1 = "1";
static const char *s_VALUE_0 = "0";

static const char *s_DIR_out = "out";
static const char *s_DIR_high = "high";
static const char *s_DIR_low = "low";
static const char *s_DIR_in = "in";


static int echo_str_2sys_file(char *fullpath, char *str) {
	int fd = open(fullpath, O_WRONLY);
	if(fd == -1) {
	  fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
	  return -1;
	}
	write(fd, str , strlen(str)); 
	close(fd); 
	return 0;
}
static int read_int_from_sys_file(char *fullpath, int *value_) {
	char buffer[8];
	int fd = open(fullpath, O_RDONLY);
	if(fd == -1) {
	  fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
	  return -1;
	}
    if((read(fd, buffer, sizeof(buffer))) < 0) {
		fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
        return -1;
    }
	close(fd); 
	*value_ = atoi(buffer);
	return 0;
}
static uint32_t get_file_access_flags(char *filename) {
	uint32_t r = 0;
	if(access(filename,F_OK)==0){ r |= 0x01;
		if(access(filename,R_OK)==0){ r |= 0x02;}		
		if(access(filename,W_OK)==0){ r |= 0x04;}
		if(access(filename,X_OK)==0){ r |= 0x08;}
	}
	return r;
}
static int gpiosys_export_(int number) { UNUSED_(number);
	char gpiosys_path_[256];
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d", number);

	uint32_t access_flags = get_file_access_flags(gpiosys_path_);
	//FrLOGV(LOG_TAG, "%s, access:0x%02x", gpiosys_path_, access_flags);
	if (access_flags == 0x0f) {

		#if 0
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/direction", number);
		FrLOGV(LOG_TAG, "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
		
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
		FrLOGV(LOG_TAG, "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
		#endif
		
		return 1;
	}

	char number_str[128];
	memset(number_str, 0, 128);
	snprintf(number_str, 128, "%d", number);
	int echo_ret = echo_str_2sys_file((char *)s_gpiosys_path_export, number_str);

#if 0
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/direction", number);
	FrLOGV(LOG_TAG, "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
	
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
	FrLOGV(LOG_TAG, "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
#endif

	return echo_ret;
}
static int gpiosys_direction_output_(int number) { UNUSED_(number);
	char gpiosys_path_[256];
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/direction", number);
	return echo_str_2sys_file(gpiosys_path_, (char *)s_DIR_out);
}
static int gpiosys_direction_input_(int number) { UNUSED_(number);
	char gpiosys_path_[256];
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/direction", number);
	return echo_str_2sys_file(gpiosys_path_, (char *)s_DIR_in);
}

static int gpiosys_output_(int number, int out_value_) { UNUSED_(number); UNUSED_(out_value_);
	char gpiosys_path_[256];
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);

	int echo_ret = 0;
	if (out_value_) {
		echo_ret = echo_str_2sys_file(gpiosys_path_, (char *)s_VALUE_1);
	} else {
		echo_ret = echo_str_2sys_file(gpiosys_path_, (char *)s_VALUE_0);
	}
	return echo_ret;
}
static int gpiosys_get_input_(int number) { UNUSED_(number);
	char gpiosys_path_[256];
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);

	int value_;
	int ret = read_int_from_sys_file(gpiosys_path_, &value_);
	if(ret == 0) {
		return value_;
	}
	return 0;
}

#define PIN_OUT_VALUE(x, v) do{ gpiosys_output_(x, v); }while(0)
#define PIN_GET_VALUE(x) gpiosys_get_input_(x)


#if 0  // this opt doesn't help so much !
#define FD_MAP_SIZE (150)
static int s_fd_map[FD_MAP_SIZE] = {0};
static int gpiosys_opt_output_(int number, int out_value_) { UNUSED_(number); UNUSED_(out_value_);
	int fd;
	if (s_fd_map[number - GPIOSYS_PINCTRL_BASE] <= 0) {
		char gpiosys_path_[256];
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
		fd = open(gpiosys_path_, O_WRONLY);
		if(fd == -1) {
		  fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
		  return -1;
		}
		s_fd_map[number - GPIOSYS_PINCTRL_BASE] = fd;
		//fprintf(stderr, "%04d = %d \r\n", number -GPIOSYS_PINCTRL_BASE , fd);
	}
	fd = s_fd_map[number - GPIOSYS_PINCTRL_BASE];

	if (out_value_) {
		write(fd, s_VALUE_1 , strlen(s_VALUE_1)); 
	} else {
		write(fd, s_VALUE_0 , strlen(s_VALUE_0)); 
	}
	return 0;
}
static int gpiosys_opt_get_input_(int number) { UNUSED_(number);
	int fd;
	if (s_fd_map[number - GPIOSYS_PINCTRL_BASE] <= 0) {
		char gpiosys_path_[256];
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
		fd = open(gpiosys_path_, O_RDONLY);
		if(fd == -1) {
		  fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
		  return 0;
		}
		s_fd_map[number - GPIOSYS_PINCTRL_BASE] = fd;
		//fprintf(stderr, "%04d = %d \r\n", number - GPIOSYS_PINCTRL_BASE, fd);
	}
	fd = s_fd_map[number - GPIOSYS_PINCTRL_BASE];
	
	char buffer[8];
	int value_;
    if((read(fd, buffer, sizeof(buffer))) < 0) {
		//fprintf(stderr, "error:%s, %d \r\n", __func__, __LINE__);
        return 0;
    }
	return atoi(buffer);
}
static void gpiosys_opt_close() {
	for(int i=0;i<FD_MAP_SIZE;i++) {
		if(s_fd_map[i] > 0) {
			close(s_fd_map[i]);
			s_fd_map[i] = 0;
		}
	}
}
#define PIN_OPT_OUT_VALUE(x, v) do{ gpiosys_opt_output_(x, v); }while(0)
#define PIN_OPT_GET_VALUE(x) gpiosys_opt_get_input_(x)
#endif

static const int s_GPIOSYS_output_pins[] = {
	PIN_VLED_DCDC_EN,
	PIN_VDD_LDO_EN,
	PIN_VDDIO_LDO_EN,
	PIN_CHIP_EN,
	PIN_CHIP_MTP_EN_1,
	PIN_CHIP_MTP_EN_2,
	PIN_CHIP_MTP_EN_3,
	PIN_CHIP_SYNC_1,
	PIN_CHIP_SYNC_2,

#if 0
	PIN_CHIP_MOSI,
	PIN_CHIP_SCLK,
	PIN_CHIP_CS1_N,
	PIN_CHIP_CS2_N,
#endif
};
#define s_GPIOSYS_output_pins_SIZE (sizeof(s_GPIOSYS_output_pins)/sizeof(s_GPIOSYS_output_pins[0]))

#if 0
static const int s_GPIOSYS_input_pins[] = {
	PIN_CHIP_MISO,
};
#define s_GPIOSYS_input_pins_SIZE (sizeof(s_GPIOSYS_input_pins)/sizeof(s_GPIOSYS_input_pins[0]))
#endif

static void initControl_gpio_pins() {
	for(int i=0;i<s_GPIOSYS_output_pins_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_output_pins[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_output_(s_GPIOSYS_output_pins[i]);
			if (direction_ret == 0) {
				gpiosys_output_(s_GPIOSYS_output_pins[i], 0);
			}
		}
		//FrLOGV(LOG_TAG, "out gpio: %02d export :%d dir: %s", s_GPIOSYS_output_pins[i] - GPIOSYS_PINCTRL_BASE, export_ret
		//	, (direction_ret == 0 ? "ok" : "failed"));
	}
#if 0
	for(int i=0;i<s_GPIOSYS_input_pins_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_input_pins[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_input_(s_GPIOSYS_input_pins[i]);
		}
		FrLOGV(LOG_TAG, "in gpio: %02d export :%d dir: %s", s_GPIOSYS_input_pins[i] - GPIOSYS_PINCTRL_BASE, export_ret
			, (direction_ret == 0 ? "ok" : "failed"));
	}
#endif

}

class SpiDeviceIntf {
public:
	virtual ~SpiDeviceIntf() {
	}
	virtual int readRegister(uint16_t address, uint16_t *value_) = 0;
	virtual int readRegisterBatch(uint16_t address, int len, uint16_t *value_, int *r_len) = 0;
	virtual int writeRegister(uint16_t address, uint16_t value_) = 0;
	virtual int writeRegisterBatch(uint16_t address, uint16_t* value_, int len) = 0;
};
#define STAGING_SIZE (1024)

static const char *spi_device_0_0 = "/dev/spidev0.0";
static const char *spi_device_0_1 = "/dev/spidev0.1";


class SpiDevice: public SpiDeviceIntf {
public:
	const char *mDevice = nullptr;
	int mFd = -1;
	int mValid = 0;

	uint8_t mode_ = 0;			//
	const uint8_t bits_ = 16; // 16;
	const uint32_t speed_ = 20000000; // 1000000; // 18000000; // 50000; // 500000;
						// spi-max-frequency = <50000000>;
	const uint16_t delay_ = 0; // 1; // 0; // 10;

	uint32_t mTxStagingCnt = 0;
	uint16_t mTxStagingBuffer[STAGING_SIZE];
	uint16_t mRxStagingBuffer[STAGING_SIZE];
	
	void pabort(const char *s) {
		fprintf(stderr, "*** pabort:%s \r\n", s);
	}

	void setup() {
		// mbi6322

		//mode_ |= SPI_LOOP;
		//mode_ |= SPI_CPHA;
		//mode_ |= SPI_CPOL;
		//mode_ |= SPI_LSB_FIRST;
		//mode_ |= SPI_CS_HIGH;
		//mode_ |= SPI_3WIRE;
		//mode_ |= SPI_NO_CS;
		//mode_ |= SPI_READY;		// error: can's set mode

//#define SPI_MODE_0 (0|0)					//SCLK
//#define SPI_MODE_1 (0|SPI_CPHA)			//SCLK
//#define SPI_MODE_2 (SPI_CPOL|0)			//SCLK
//#define SPI_MODE_3 (SPI_CPOL|SPI_CPHA)	//SCLK
		mode_ |= SPI_MODE_0;

		int ret = 0;
		uint8_t mode_r;
		uint8_t bits_r;
		uint32_t speed_r;
		/*
		 * spi mode
		 */
		ret = ioctl(mFd, SPI_IOC_WR_MODE, &mode_);
		if (ret == -1) {
			pabort("can't set spi mode"); return ;
		}
		ret = ioctl(mFd, SPI_IOC_RD_MODE, &mode_r);
		if (ret == -1) {
			pabort("can't get spi mode"); return ;
		}
		/*
		 * bits per word
		 */
		ret = ioctl(mFd, SPI_IOC_WR_BITS_PER_WORD, &bits_);
		if (ret == -1) {
			pabort("can't set bits per word"); return ;
		}
		ret = ioctl(mFd, SPI_IOC_RD_BITS_PER_WORD, &bits_r);
		if (ret == -1) {
			pabort("can't get bits per word"); return ;
		}
		/*
		 * max speed hz
		 */
		ret = ioctl(mFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_);
		if (ret == -1) {
			pabort("can't set max speed hz"); return ;
		}
		ret = ioctl(mFd, SPI_IOC_RD_MAX_SPEED_HZ, &speed_r);
		if (ret == -1) {
			pabort("can't get max speed hz"); return ;
		}

		uint8_t lsb_ = 0;
		ret = ioctl(mFd, SPI_IOC_RD_LSB_FIRST, &lsb_);
		if (ret == -1) {
			pabort("can't get LSB_FIRST"); return ;
		}

		fprintf(stderr, "spi mode: 0x%02x/0x%02x \n", mode_, mode_r);
		fprintf(stderr, "bits per word: %d/%d \n", bits_, bits_r);
		fprintf(stderr, "max speed: %d Hz (%d KHz) / %d Hz (%d KHz) \n", 
			speed_, speed_/1000, speed_r, speed_r/1000);
		fprintf(stderr, "lsb_ : %d \r\n" , lsb_);

		mValid = 1;
	}

#if 1
// 16bits spidev test ok !!!
int transfer(uint16_t* txBuffer, uint16_t* rxBuffer, uint32_t size_) {
	if (mValid == 0) {
		fprintf(stderr, "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)txBuffer,		//
		.rx_buf = (unsigned long)rxBuffer,		//
		.len = size_,		// number in byte !!!
		.delay_usecs = delay_,				//
		.speed_hz = speed_, // speed_,					//
		.bits_per_word = bits_,				//
	};
	ret = ioctl(mFd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		pabort("can't send spi message");
		return -1;
	}
	//fprintf(stderr, "%s %u done! \r\n", __func__, size_);
	return 0;
}
virtual int readRegister(uint16_t address, uint16_t *value_) override {
	if (mValid == 0) {
		fprintf(stderr, "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	int ret = 0;
	address <<= 1; address += 1;
	mTxStagingCnt = 0;
	mTxStagingBuffer[mTxStagingCnt++] = address;
	mTxStagingBuffer[mTxStagingCnt++] = 0;
	ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt*2);
	if (ret == 0) {
		uint16_t r_value = mRxStagingBuffer[1];
		*value_ = r_value;
	}
	return ret;
}
virtual int readRegisterBatch(uint16_t address, int len, uint16_t *value_, int *r_len) override {
	UNUSED_(address); UNUSED_(len); UNUSED_(value_); UNUSED_(r_len);

	if (mValid == 0) {
		fprintf(stderr, "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	fprintf(stderr, "%s len = %d \n", __func__, len);
	int ret = 0;
	address <<= 1; address += 1;
	mTxStagingCnt = 0;
	mTxStagingBuffer[mTxStagingCnt++] = address;
	for(int i=0;i<len;i++) {
		mTxStagingBuffer[mTxStagingCnt++] = 0;
	}
	ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt*2);
	if (ret == 0) {
		for(int i=1;i<=len;i++) {
			uint16_t r_value = mRxStagingBuffer[i];
			value_[i-1] = r_value;
		}
		*r_len = len;
	}

	return 0;
}
virtual int writeRegister(uint16_t address, uint16_t value_) override {
	if (mValid == 0) {
		fprintf(stderr, "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	int ret = 0;
	address <<= 1; address += 0;
	mTxStagingCnt = 0;
	mTxStagingBuffer[mTxStagingCnt++] = address;
	mTxStagingBuffer[mTxStagingCnt++] = value_;
	ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt*2);
	return ret;
}
virtual int writeRegisterBatch(uint16_t address, uint16_t* value_, int len) override {
	UNUSED_(address); UNUSED_(len); UNUSED_(value_); 
	if (mValid == 0) {
		fprintf(stderr, "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	fprintf(stderr, "%s len = %d \n", __func__, len);
	int ret = 0;
	address <<= 1; address += 0;
	mTxStagingCnt = 0;
	mTxStagingBuffer[mTxStagingCnt++] = address;
	for(int i=0;i<len;i++) {
		mTxStagingBuffer[mTxStagingCnt++] = value_[i];
	}
	ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt*2);
	return ret;
}

#else
	// error code !!!
	int transfer(uint8_t* txBuffer, uint8_t* rxBuffer, uint32_t size_) {
		if (mValid == 0) {
			return -1;
		}
		int ret;
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)txBuffer,		//
			.rx_buf = (unsigned long)rxBuffer,		//
			.len = size_,		// number in byte !!!
			.delay_usecs = delay_,				//
			//.speed_hz = speed_/2, // speed_,					//
			//.bits_per_word = bits_,				//
			//.tx_nbits = bits_/2,
			//.rx_nbits = bits_/2,
			.cs_change = 1,
		};
		ret = ioctl(mFd, SPI_IOC_MESSAGE(1), &tr);
		if (ret < 1) {
			pabort("can't send spi message");
			return -1;
		}
		//fprintf(stderr, "%s %u done! \r\n", __func__, size_);
		return 0;
	}
	virtual int readRegister(uint16_t address, uint16_t *value_) override {
		if (mValid == 0) {
			return -1;
		}
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		fprintf(stderr, "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		if (ret == 0) {
			uint16_t r_value = mRxStagingBuffer[2];
			r_value <<= 8;
			r_value += mRxStagingBuffer[3];
			*value_ = r_value;
		}
		return ret;
	}
	virtual int readRegisterBatch(uint16_t address, int len, uint16_t *value_, int *r_len) override {
		UNUSED_(address); UNUSED_(len); UNUSED_(value_); UNUSED_(r_len);

		if (mValid == 0) {
			return -1;
		}
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		fprintf(stderr, "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = 0;
			mTxStagingBuffer[mTxStagingCnt++] = 0;
		}
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		if (ret == 0) {
			for(int i=1;i<=len;i++) {
				uint16_t r_value = mRxStagingBuffer[i*2 + 0];
				r_value <<= 8;
				r_value += mRxStagingBuffer[i*2 + 1];
				value_[i] = r_value;
			}
			*r_len = len;
		}

		return 0;
	}
	virtual int writeRegister(uint16_t address, uint16_t value_) override {
		if (mValid == 0) {
			return -1;
		}
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>0)&0xff;
		//fprintf(stderr, "-> %02X %02X %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1], mTxStagingBuffer[2], mTxStagingBuffer[3]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}
	virtual int writeRegisterBatch(uint16_t address, uint16_t* value_, int len) override {
		UNUSED_(address); UNUSED_(len); UNUSED_(value_); 
		if (mValid == 0) {
			return -1;
		}
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>8)&0xff;
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>0)&0xff;
		}
		fprintf(stderr, "-> %02X %02X %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1], mTxStagingBuffer[2], mTxStagingBuffer[3]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}
#endif
	SpiDevice(const char * device_): mDevice(device_), mFd(open(mDevice, O_RDWR)) {
		//FrLOGV(LOG_TAG, "%s,%d  mFd:%d ", __func__, __LINE__, mFd);
		if (mFd > 0) {
			setup();
		}
	}
	virtual ~SpiDevice() {
		close(mFd);
		//FrLOGV(LOG_TAG, "%s,%d", __func__, __LINE__);
	}
	

};

static const int s_GPIOSYS_output_pins_spi[] = {
#if 1
	PIN_CHIP_MOSI,
	PIN_CHIP_SCLK,
	PIN_CHIP_CS1_N,
	PIN_CHIP_CS2_N,
#endif
};
#define s_GPIOSYS_output_pins_spi_SIZE (sizeof(s_GPIOSYS_output_pins_spi)/sizeof(s_GPIOSYS_output_pins_spi[0]))
#if 1
static const int s_GPIOSYS_input_pins_spi[] = {
	PIN_CHIP_MISO,
};
#define s_GPIOSYS_input_pins_spi_SIZE (sizeof(s_GPIOSYS_input_pins_spi)/sizeof(s_GPIOSYS_input_pins_spi[0]))
#endif

static int initSpi_gpio_pins_flag = 0;
static void initSpi_gpio_pins() {
	//FrLOGV(LOG_TAG, "%s,%d", __func__, __LINE__);
	for(int i=0;i<s_GPIOSYS_output_pins_spi_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_output_pins_spi[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_output_(s_GPIOSYS_output_pins_spi[i]);
			if (direction_ret == 0) {
				gpiosys_output_(s_GPIOSYS_output_pins_spi[i], 0);
			}
		}
		//FrLOGV(LOG_TAG, "spi out gpio: %02d export :%d dir: %s", s_GPIOSYS_output_pins_spi[i] - GPIOSYS_PINCTRL_BASE, export_ret
		//	, (direction_ret == 0 ? "ok" : "failed"));
	}
#if 1
	for(int i=0;i<s_GPIOSYS_input_pins_spi_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_input_pins_spi[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_input_(s_GPIOSYS_input_pins_spi[i]);
		}
		//FrLOGV(LOG_TAG, "spi in gpio: %02d export :%d dir: %s", s_GPIOSYS_input_pins_spi[i] - GPIOSYS_PINCTRL_BASE, export_ret
		//	, (direction_ret == 0 ? "ok" : "failed"));
	}
#endif
	PIN_OUT_VALUE(PIN_CHIP_CS1_N, 1);
	PIN_OUT_VALUE(PIN_CHIP_CS2_N, 1);
	PIN_OUT_VALUE(PIN_CHIP_SCLK, 0);
	PIN_OUT_VALUE(PIN_CHIP_MOSI, 0);
}
class SpiDeviceGpio: public SpiDeviceIntf {
public:
	int mCs = 0;
	volatile uint32_t mTxStagingCnt = 0;
	volatile uint8_t mTxStagingBuffer[STAGING_SIZE];
	volatile uint8_t mRxStagingBuffer[STAGING_SIZE];

	SpiDeviceGpio(int cs_): mCs(cs_) {
		fprintf(LOG_TAG, "%s,%d ", __func__, __LINE__);
		if (initSpi_gpio_pins_flag == 0) {
			initSpi_gpio_pins_flag = 1;
			initSpi_gpio_pins();
		}
	}
	virtual ~SpiDeviceGpio() {
		fprintf(stderr, "%s,%d ", __func__, __LINE__);
	}

#define SET_out_value_ PIN_OUT_VALUE
#define GET_in_value_ PIN_GET_VALUE

	int transfer(volatile uint8_t* txBuffer, volatile uint8_t* rxBuffer, uint32_t size_) {
		//gpiosys_opt_close();

#define transfer_debug 0 // 1 // 0
		if (mCs == 0) {SET_out_value_(PIN_CHIP_CS1_N, 0);}
		else if(mCs == 1) {SET_out_value_(PIN_CHIP_CS2_N, 0);}
		//usleep(1);
		
		for(uint32_t i=0;i<size_;i++) {
			uint8_t shift_bit = 0x80;
			uint8_t tx_ = txBuffer[i];
			uint8_t rx_ = 0;
#if transfer_debug
			fprintf(stderr, "%04d - tx:%02x ", i, tx_);
#endif
			for(int clk_ = 0; clk_ < 8; clk_++) {
#if transfer_debug
				fprintf(stderr, "%02x%s ", shift_bit, (tx_&shift_bit ? "(1)" : "(0)"));
#endif
				if(tx_&shift_bit) { SET_out_value_(PIN_CHIP_MOSI, 1); }
				else { SET_out_value_(PIN_CHIP_MOSI, 0); }

				//usleep(1);

				rx_ <<= 1;
				if (GET_in_value_(PIN_CHIP_MISO)) {
					rx_ |= 0x01;
				}

				SET_out_value_(PIN_CHIP_SCLK, 1);
				usleep(1);

				SET_out_value_(PIN_CHIP_SCLK, 0);
				//usleep(1);

				shift_bit >>= 1;
				
				usleep(1);
			}
#if transfer_debug
			fprintf(stderr, "rx:%02x ", rx_);
			fprintf(stderr, "\r\n");
#endif
			rxBuffer[i] = rx_;
		}

		//usleep(1);
		if (mCs == 0) {SET_out_value_(PIN_CHIP_CS1_N, 1);}
		else if(mCs == 1) {SET_out_value_(PIN_CHIP_CS2_N, 1);}
		//usleep(1);

		//gpiosys_opt_close();
		return 0;
	}
	virtual int readRegister(uint16_t address, uint16_t *value_) override { UNUSED_(address); UNUSED_(value_);
		fprintf(stderr, "%s:0x%04x \r\n", __func__, address);
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		fprintf(stderr, "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		if (ret == 0) {
			uint16_t r_value = mRxStagingBuffer[2];
			r_value <<= 8;
			r_value += mRxStagingBuffer[3];
			*value_ = r_value;
		}
		//fprintf(stderr, "read %04x : %04x \r\n", address, *value_);
		return ret;	
	}
	virtual int readRegisterBatch(uint16_t address, int len, uint16_t *value_, int *r_len) override {
		UNUSED_(address); UNUSED_(len); UNUSED_(value_); UNUSED_(r_len);
		fprintf(stderr, "%s:0x%04x len=%d \r\n", __func__, address, len);
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = 0;
			mTxStagingBuffer[mTxStagingCnt++] = 0;
		}
		fprintf(stderr, "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		if (ret == 0) {
			if (r_len != nullptr) {
				*r_len = len;
			}
			for(int i=1;i<=len;i++) {
				uint16_t r_value = mRxStagingBuffer[2*i + 0];
				r_value <<= 8;
				r_value += mRxStagingBuffer[2*i + 1];
				value_[i-1] = r_value;
			}
		}
		return ret;
	}
	virtual int writeRegister(uint16_t address, uint16_t value_) override { UNUSED_(address); UNUSED_(value_);
		fprintf(stderr, "%s:0x%04x value_=0x%04x \r\n", __func__, address, value_);
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>0)&0xff;
		fprintf(stderr, "%s, -> %02X %02X \r\n", __func__, mTxStagingBuffer[0], mTxStagingBuffer[1]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}
	virtual int writeRegisterBatch(uint16_t address, uint16_t* value_, int len) override {
		fprintf(stderr, "%s:0x%04x len=%d \r\n", __func__, address, len);
		UNUSED_(address); UNUSED_(len); UNUSED_(value_);
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		fprintf(stderr, "%s, -> %02X %02X \r\n", __func__, mTxStagingBuffer[0], mTxStagingBuffer[1]);
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>8)&0xff;
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>0)&0xff;
		}
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}

};

/*
PLL M>16
0.5MHz < internal oscillator/PLL N < 2MHz ··················································································· (6)
40MHz < (internal oscillator/PLL N)× PLL M < 200MHz ········································································ (7)
PLL frequency = ((internal oscillator × PLL M)/(PLL N × PLL O)) ······································································· (8)
GCLK frequency = PLL frequency / GCLK frequency divider

internal oscillator	5.85 MHz
PLL M	58
PLL N	5
PLL O	1
PLL frequency	67.86 MHz
GCLK frequency divider	[000011] (divided by 4 )
GCLK frequency	16.96 MHz

PLL M	48	PLL N	6	PLL O	3	GCLK_Frequency_Divider(5)	// PLL=15.6MHz 	GCLK = 3.12MHz (0.3205 us)
PLL M	58	PLL N	5	PLL O	1	GCLK_Frequency_Divider(5)	// 67.86MHz		13.572MHz(0.074 us)
PLL M	75	PLL N	5	PLL O	1	GCLK_Frequency_Divider(5)	// 87.75MHz 	17.55 MHz(0.056898us ~ 57ns)


Dummy time = Dummy time width x GCLK cycle > 1us

Dead time = Dead time width x GCLK cycle > 3us

Scramble time= The total number of GCLKs in Scramble × GCLK cycle time
(Dummy time + Scramble time + Dead time) × the number of scrambles	// one line
	x the number of scans < 16.6ms									// one frame 

*/
static uint16_t reg_value_override(uint16_t addr_, uint16_t value_) {
#define PWM_mode 0
	//[0]: continue mode
	//[1]: one shot mode
#define PWM_data_bit (0x02) // 0x00
	//				The total number of GCLKs
	//[000]: 14-bit		16384
	//[001]: 13-bit		8192
	//[010]: 12-bit		4096
	//[011]: 11-bit		2048
	//[100]: 10-bit		1024
	//[101]~[111]: same as code [000] 16384
#define PWM_counting_mode 0x01
	//[00]: odd channel forward counting mode/even channel backward counting mode
	//[01]: all channel forward counting mode
	//[10]: all channel backward
	//[11]: same as code [00]
#define Chip_sleep_mode_enable 0
	//[0]: Disable
	//[1]: Enable
#define Chip_sleep_select 0
	//[0]: wakeup for brightness have value
	//[1]: wakeup for vsync
#define SYNC_pin_enable 0
	//[0]: vsync command valid, SYNC pin disable
	//[1]: vsync command invalid, SYNC pin enable
#define Scramble_number 0x00 // 0x01	// 0x03
	//[00]: 1 scramble
	//[01]: 4 scramble
	//[10]: 8 scramble
	//[11]: 16 scramble
#define Scan_number 0x07 //0x7
	//[0000]: 1 lines; [1000]: 9 lines;
	//[0001]: 2 lines; [1001]: 10 lines;
	//[0010]: 3 lines; [1010]: 11 lines;
	//[0011]: 4 lines; [1011]: 12 lines;
	//[0100]: 5 lines; [1100]: 13 lines;
	//[0101]: 6 lines; [1101]: 14 lines;
	//[0110]: 7 lines; [1110]: 15 lines;
	//[0111]: 8 lines; [1111]: 16 lines;
	if (addr_ == 0X0000) { return 
(PWM_mode<<0xf | PWM_data_bit <<0xc | PWM_counting_mode<<0xa | Chip_sleep_mode_enable <<0x9 | Chip_sleep_select <<0x8 | SYNC_pin_enable<<0x6 | Scramble_number<<0x4 | Scan_number<<0x0); } // 0x8037
//-----------------------------------------------------------------------------------------------------------------------------
	// Dummy time width
	else if(addr_ == 0X0001) { return 0x0018; }	// 0x0018 // Reserve
//-----------------------------------------------------------------------------------------------------------------------------
	// Dead time width
	else if(addr_ == 0X0002) { return 0x0048; }	// 0x0048 // Reserve
//-----------------------------------------------------------------------------------------------------------------------------
	// Scan change period
	else if(addr_ == 0X0003) { return 0x0018; }	// 0x0018 // scan change period. SC[15:0] x GCLK period
//----------------------------------------------------------------------------------------------------------------------------
	// Scan separate period
	else if(addr_ == 0X0004) { return 0x0018; }	// 0x0018 // MOS separate period, MS[15:0] x GCLK period
//-----------------------------------------------------------------------------------------------------------------------------
#define Channel_parallel_defined 0x00
	//[00]: 32 channel define
	//[01]: 16 channel define
	//[10]: 8 channel define
	//[11]: same as code [00]
#define PLL_O 0x03 // 0x3
	// [00]~[11]=Div is 8/4/2/1(PLL)
	//[00]: 8
	//[01]: 4
	//[10]: 2
	//[11]: 1
#define PLL_N 6 // 5 // 0x6
	// b~7(5bits) // [00000]~[11111]: 0~31
#define PLL_M 48 // 75 // 0x30
	// 6~0(7bits) // [0000000]~[1111111]: 0~127
	//Design options (analog_sel[2:0], PLL)
	//PLL M >= 64,analog selection = [001]
	//PLL M < 64,analog selection = [000]
	else if(addr_ == 0X0005) { return
((Channel_parallel_defined)<<0xE | (PLL_O)<<0xC | (PLL_N)<<0x7 | (PLL_M)<<0x0); } // 0x3330
#define SW_Discharge_enable 1
	//[0]: Disable
	//[1]: Enable
#define SW_Discharge_voltage_level 0x0
	//[000]~[111]:
	//[000]=> VLED-VSW=4.64V
	//[100]=> VLED-VSW=2.77V
	//[111]=> VLED-VSW=1.17V
#define SW_Discharge_time 0
	//[0]: 1us
	//[1]: 2us
#define GCLK_Frequency_Divider (4)
	//[000000]: 1
	//[000001]: 2
	//[000010]: 3
	//[000011]: 4
	//...
	//[111101]: 62
	//[111110]: 63
	//[111111]: 64
	else if(addr_ == 0X0006) { return 
((SW_Discharge_enable)<<0xf | (SW_Discharge_voltage_level)<<0xc | (SW_Discharge_time)<<0xb | (GCLK_Frequency_Divider)<<0x0); }	// 0x8005
//-----------------------------------------------------------------------------------------------------------------------------
#define Precharge_voltage 0 // (0xf)
	//[0000]: 1.95V [1000]: 3.15V
	//[0001]: 2.1V  [1001]: 3.3V
	//[0010]: 2.25V [1010]: 3.45V
	//[0011]: 2.4V  [1011]: 3.6V
	//[0100]: 2.55V [1100]: 3.75V
	//[0101]: 2.7V  [1101]: 3.9V
	//[0110]: 2.85V [1110]: 4.05V
	//[0111]: 3V  [1111]: 4.2V
#define Precharge_location (0)
	//[0]: pre-charge in deadtime
	//[1]: pre-charge during channel off
#define Precharge_global_enable (0)
	//[0]: Disable
	//[1]: Enable
#define Current_gain (0x7)
	//current gain value, [000]~[111]
	//Delta-ratio rang :100%~200% step by 14.29%
	else if(addr_ == 0X0007) { return 
((Precharge_voltage)<<0xc | (Precharge_location)<<0xb | (Precharge_global_enable)<<0xa | (Current_gain)<<0x0); }	// 0x0000
//-----------------------------------------------------------------------------------------------------------------------------
#define Thermal_test_voltage 0
	//[0]: 0.5V about 120CTSD=0
	//[1]: 0.425V about 168CTSD=1 (thermal shut down)
#define Analog_selection 0x0 // 0x01 // 0x0
	// Design options (analog_sel[2:0], PLL)
#define Decrease_overshoot 0
	//[0]: Internal VD=0.1V (Def)
	//[1]: Internal VD=0.3V
#define Feedback_function_enable 0
	//[0]: Disable
	//[1]: Enable
#define Open_error_detection_voltage 0
	//*Open Error detection voltage threshold,
	//[00] ~ [11]: High ~ Low
#define Short_error_detection_voltage 0
	//*Short Error detection voltage threshold,
	//[00] ~ [11]: Low ~ High
#define Tr_2_0 0
	//Speed Control Tr, [000] ~ [111]: Low ~ High
	//[000]:50nS	[100]:155nS
	//[001]:75nS	[101]:195nS
	//[010]:105nS	[110]:240nS
	//[011]:130nS	[111]: 300nS
#define Tf_2_0 0
	//Speed Control Tf, [000] ~ [111]: Low ~ High
	//[000]:55nS	[100]:155nS
	//[001]:85nS	[101]:205nS
	//[010]:105nS	[110]:285nS
	//[011]:125nS	[111]: 400nS
	else if(addr_ == 0X0008) { return 
(Thermal_test_voltage<<0xf | Analog_selection<<0xc | Decrease_overshoot<<0xb | Feedback_function_enable<<0xa | Open_error_detection_voltage<<0x8 | Short_error_detection_voltage<<0x6 | Tr_2_0<<0x3 | Tf_2_0<<0x0); }		// 0x0000
//-----------------------------------------------------------------------------------------------------------------------------
#define MOS_switch_parallel_defined (0x01)
	//MOS switch parallel defined
	//[00]: 16 switch define
	//[01]: 8 switch define
	//[10]: 4 switch define
	//[11]: 2 switch define
#define Global_brightness_control (0x3fff)
	// Global Brightness control (14-bit)
	else if(addr_ == 0X0009) { return 
(MOS_switch_parallel_defined<<0xe | Global_brightness_control<<0x0) ; }		// 0x7FFF
//-----------------------------------------------------------------------------------------------------------------------------
	else if(addr_ == 0X000a) { return 0x0000; }		// 0x0000
	else if(addr_ == 0X000b) { return 0x0008; }		// 0x0008
	else if(addr_ == 0X000c) { return 0x00FF; }		// 0x00FF
	else if(addr_ == 0X000d) { return 0x0000; }		// 0x0000
			//0x3fff
	else if(addr_ >= 0x0010 && addr_ <= 0x010f) { return 0xfff; }	// SCAN0~SCAN7 Brightness code of Channel31~Channel0
	else if(addr_ >= 0x0110 && addr_ <= 0x020f) { return 0; }	// SCAN8~SCAN15 Brightness code of Channel31~Channel0
			// 0xffff
	else if(addr_ >= 0x0210 && addr_ <= 0x023f) { return 0; }	// SCAN0~SCAN7 dot correction
	else if(addr_ >= 0x0240 && addr_ <= 0x026f) { return 0; }		// SCAN8~SCAN15 dot correction
	else if(addr_ == 0x0009) { return 0x3fff; }
	return value_;
}
int get_PWM_data_bit_max_value() {
#if PWM_data_bit == 0x00
	return 16384;
#elif PWM_data_bit == 0x01
	return 8192;
#elif PWM_data_bit == 0x02
	return 4096;
#elif PWM_data_bit == 0x03
	return 2048;
#elif PWM_data_bit == 0x04 
	return 1024;
#else
	return 16384;
#endif
}
int map_PWM_data_bit_value(int data_) {
	int pwm_value_ = (get_PWM_data_bit_max_value()/255)*data_;
	if (pwm_value_ >= get_PWM_data_bit_max_value()) {
		pwm_value_ = get_PWM_data_bit_max_value() - 1;
	}
	return pwm_value_;
}


class MbiDevice: public virtual football::FootballBlController {
public:
	MbiDevice();
	~MbiDevice();

	void init();

	// FootballBlController impl
	virtual int initialize();
	virtual int unInitialize();
	virtual int backlightSetData(int x, int y, int data_);
	virtual int backlightCommit(int mapType = 0);

	void backlight_data_fill(int data_);

	SpiDeviceIntf *mSpiDevice0 = nullptr;
	SpiDeviceIntf *mSpiDevice1 = nullptr;

	uint16_t mDataBuffer[DATA_BUFFER_SIZE] = {0};
	uint32_t mDataNum = 0;

	// 
#define BL_WIDTH (15)
#define BL_HEIGHT (31)
	int backlight_data_[BL_WIDTH*BL_HEIGHT + 256];

#define SCAN_NUM (16)
#define CHANNEL_NUM (32)
	uint16_t mbiData0[CHANNEL_NUM*SCAN_NUM] = {0}; // store channel in priority
	uint16_t mbiData1[CHANNEL_NUM*SCAN_NUM] = {0}; // store channel in priority

};
MbiDevice::MbiDevice() {
	backlight_data_fill(100);

	initControl_gpio_pins();
	initSpi_gpio_pins();

#if WITH_HW_SPI
	mSpiDevice0 = new SpiDevice(spi_device_0_0);
	mSpiDevice1 = new SpiDevice(spi_device_0_1);
#else
	mSpiDevice0 = new SpiDeviceGpio(0);
	mSpiDevice1 = new SpiDeviceGpio(1);
#endif

	init();
}
MbiDevice::~MbiDevice() {
	delete mSpiDevice0;
	delete mSpiDevice1;

	PIN_OUT_VALUE(PIN_VLED_DCDC_EN, 0);
	PIN_OUT_VALUE(PIN_VDD_LDO_EN, 0);
	PIN_OUT_VALUE(PIN_VDDIO_LDO_EN, 0);
	PIN_OUT_VALUE(PIN_CHIP_EN, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_1, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_2, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_3, 0);
	PIN_OUT_VALUE(PIN_CHIP_SYNC_1, 0);
	PIN_OUT_VALUE(PIN_CHIP_SYNC_2, 0);
}
void MbiDevice::init() {
	PIN_OUT_VALUE(PIN_VDD_LDO_EN, 0);
	PIN_OUT_VALUE(PIN_VDDIO_LDO_EN, 0);
	PIN_OUT_VALUE(PIN_VLED_DCDC_EN, 0);
	PIN_OUT_VALUE(PIN_CHIP_EN, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_1, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_2, 0);
	PIN_OUT_VALUE(PIN_CHIP_MTP_EN_3, 0);
	PIN_OUT_VALUE(PIN_CHIP_SYNC_1, 0);
	PIN_OUT_VALUE(PIN_CHIP_SYNC_2, 0);


	DELAY_Millis(50);

	PIN_OUT_VALUE(PIN_VDD_LDO_EN, 1);
	DELAY_Millis(3);
	PIN_OUT_VALUE(PIN_VDDIO_LDO_EN, 1);
	DELAY_Millis(3);
	PIN_OUT_VALUE(PIN_VLED_DCDC_EN, 1);
	DELAY_Millis(3);
	PIN_OUT_VALUE(PIN_CHIP_EN, 1);
	DELAY_Millis(3);

	//mSpiDevice0->writeRegister(0xf00, 1); // wake up
	//mSpiDevice0->writeRegister(0xe00, 1); // software reset
	//DELAY_Millis(3);
	//mSpiDevice0->writeRegister(0xf00, 1); // wake up

	DELAY_Millis(10);

	fprintf(stderr, "mbi6322_initial_settings_reg_size:%d (*2=%d) / real:%d \r\n", mbi6322_initial_settings_reg_size,
		mbi6322_initial_settings_reg_size*2
		, (int)(sizeof(mbi6322_initial_settings)/sizeof(mbi6322_initial_settings[0])));

#if 1
	for(int i=0;i<mbi6322_initial_settings_reg_size;i++) {
		uint16_t addr_ = mbi6322_initial_settings[i*2 + 0];
		uint16_t value_w_ = mbi6322_initial_settings[i*2 + 1];
#if 1
		if (addr_ >= 0x10 && addr_ <= 0x20f) {
			continue;
		}
#endif
		value_w_ = reg_value_override(addr_, value_w_);
#if 1
		if (addr_ >= 0x210 && addr_ <= 0x26f) {
		}
#endif
		mSpiDevice0->writeRegister(addr_, value_w_);

		//uint16_t value_ = 0;
		//mSpiDevice0->readRegister(addr_, &value_);
		//fprintf(stderr, "[%04X] : %04X \r\n", addr_, value_);
	}

	//mSpiDevice0->writeRegister(0x0f, 0x01);
	fprintf(stderr, "spi0 initial settings done! \r\n");
#endif

#if 1
	for(int i=0;i<mbi6322_initial_settings_reg_size;i++) {
		uint16_t addr_ = mbi6322_initial_settings[i*2 + 0];
		uint16_t value_w_ = mbi6322_initial_settings[i*2 + 1];
#if 1
		if (addr_ >= 0x10 && addr_ <= 0x20f) {
			continue;
		}
#endif
		value_w_ = reg_value_override(addr_, value_w_);
#if 1
		if (addr_ >= 0x210 && addr_ <= 0x26f) {
		}
#endif
		mSpiDevice1->writeRegister(addr_, value_w_);

		//uint16_t value_ = 0;
		//mSpiDevice1->readRegister(addr_, &value_);
		//fprintf(stderr, "[%04X] : %04X \r\n", addr_, value_);
	}
	//mSpiDevice1->writeRegister(0x0f, 0x01);
	fprintf(stderr, "spi1 initial settings done! \r\n");
#endif

	backlight_data_fill(get_PWM_data_bit_max_value() - 1);
	backlightCommit(1);


#if 1  // dump
{
	uint16_t reg_addr = 0x0000;
	int reg_num = 0x0f;
	int ret_len = 0;
	int ret = 0;
	fprintf(stderr, "----------------------------------------------------- \r\n");
	ret = mSpiDevice0->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	fprintf(stderr, "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "				", 16);
	}
	ret = mSpiDevice1->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	fprintf(stderr, "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "				", 16);
	}
}
#endif

}


// FootballBlController impl
int MbiDevice::initialize() {
	return 0;
}
int MbiDevice::unInitialize() {
	return 0;
}
int MbiDevice::backlightSetData(int x, int y, int data_) {
	if (x >= 0 && y >= 0 && x < BL_WIDTH && y < BL_HEIGHT) {
		backlight_data_[y*BL_WIDTH + x] = data_;
	}
	return 0;
}
int MbiDevice::backlightCommit(int mapType) {
	fprintf(stderr, "%s mapType = %d ... \r\n", __func__, mapType);

	int max_ = 0;
	// 0, A1~A8		-> SCAN0~SCAN7
	for(int y=0;y<BL_HEIGHT;y++) {
		for(int x=0;x<8;x++) {
			int data_ = backlight_data_[y*BL_WIDTH + x]; if(data_ > 65535) { data_ = 65535; } else if(data_ < 0) {data_ = 0;}
			if (mapType == 0) {
				int mapped_ = map_PWM_data_bit_value(data_); if (mapped_ > max_) { max_ = mapped_; }
				mbiData0[x*CHANNEL_NUM + y] = (uint16_t)mapped_;
			} else if (mapType == 1) {
				if (data_ > max_) { max_ = data_; }
				mbiData0[x*CHANNEL_NUM + y] = (uint16_t)data_;
			}
		}
	}

	// 1, A9~A15	-> SCAN1~SCAN7
	for(int y=0;y<BL_HEIGHT;y++) {
		for(int x=8;x<BL_WIDTH;x++) {
			int data_ = backlight_data_[y*BL_WIDTH + x];
			if (mapType == 0) {
				int mapped_ = map_PWM_data_bit_value(data_); if (mapped_ > max_) { max_ = mapped_; }
				mbiData1[(x-8 + 1)*CHANNEL_NUM + y] = (uint16_t)mapped_;
			}
			else if (mapType == 1) {
				if (data_ > max_) { max_ = data_; }
				mbiData1[(x-8 + 1)*CHANNEL_NUM + y] = (uint16_t)data_;
			}
		}
	}
	fprintf(stderr, "    max_ = %d \r\n", max_);

	mSpiDevice0->writeRegisterBatch(0x10, mbiData0, CHANNEL_NUM*8);
	mSpiDevice1->writeRegisterBatch(0x30, mbiData1, CHANNEL_NUM*7);

	mSpiDevice0->writeRegister(0x0f, 0x01);
	mSpiDevice1->writeRegister(0x0f, 0x01);

	fprintf(stderr, "%s done! \r\n", __func__);
	return 0;
}
void MbiDevice::backlight_data_fill(int data_) {
	fprintf(stderr, "%s, data_ = %5d \r\n", __func__, data_);
	for(int y=0;y<BL_HEIGHT;y++) {
		for(int x=0;x<BL_WIDTH;x++) {
			backlight_data_[y*BL_WIDTH + x] = data_;
		}
	}
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/


TestCmdMiniLed::TestCmdMiniLed(uint32_t flags) :
	// in order
	mInitFlags(flags)
	, mCmdline(::android::cmdline::factory::makeCmdline())
{
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	mMbiDevice = new MbiDevice();
	mValid = 1;
}
TestCmdMiniLed::~TestCmdMiniLed() {
	delete mMbiDevice;
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}
void TestCmdMiniLed::initCmd() {
	mCmdline->setPrompt("miniLed>>");

	CL_ADD_FUNC(mCmdline, date);
	CL_ADD_FUNC(mCmdline, pon);
	CL_ADD_FUNC(mCmdline, poff);
	CL_ADD_FUNC(mCmdline, init);
	CL_ADD_FUNC(mCmdline, w);
	CL_ADD_FUNC(mCmdline, r);
	CL_ADD_FUNC(mCmdline, bl);
	CL_ADD_FUNC(mCmdline, test1);
	CL_ADD_FUNC(mCmdline, test2);
	CL_ADD_FUNC(mCmdline, test3);
	CL_ADD_FUNC(mCmdline, test4);
}
int TestCmdMiniLed::runCommand(const char * cmd) {
	return mCmdline->runCommand(cmd);
}
void TestCmdMiniLed::loop() {
	mCmdline->loop();
}

// FootballBlController impl
int TestCmdMiniLed::initialize() { return mMbiDevice->initialize() ;}
int TestCmdMiniLed::unInitialize() { return mMbiDevice->unInitialize() ;}
int TestCmdMiniLed::backlightSetData(int x, int y, int data_) { return mMbiDevice->backlightSetData(x,y,data_) ;}
int TestCmdMiniLed::backlightCommit(int mapType) { return mMbiDevice->backlightCommit(mapType) ;}


CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, date);
int TestCmdMiniLed::date(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	fprintf(stderr, "build at %s %s \r\n", __DATE__, __TIME__);
	return 0;
}


CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, pon);
int TestCmdMiniLed::pon(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, poff);
int TestCmdMiniLed::poff(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, init);
int TestCmdMiniLed::init(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	mMbiDevice->init();
	return 0;
}

//CL_func_IMPL(TestCmdMiniLed, w, TestCmdMiniLed) {
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, w);
int TestCmdMiniLed::w(int argc, char *const*argv) {
	UNUSED_(argc); UNUSED_(argv);
	if (argc <= 3) {
		fprintf(stderr, "should input: w [reg_addr(hex)] [reg_num(dec)] hex#1 ... hex#reg_num \r\n");
		return 0;
	}
	int reg_addr = _parse_hex_str(argv[1], cycling::FLAG_U16);
	if (reg_addr < 0) {
		fprintf(stderr, "reg_addr invalid! \r\n");
		return 0;
	}
	int reg_num = 0;
	if (cycling::_c__atoi(argv[2], &reg_num) < 0) {
		fprintf(stderr, "reg_num invalid! \r\n");
		return 0;
	}
	if (reg_num <= 0 || reg_num != (argc - 3)) {
		fprintf(stderr, "reg_num invalid! \r\n");
		return 0;
	}

	if(cycling::getData16_(argc -3, argv + 3, mDataBuffer, DATA_BUFFER_SIZE, &mDataNum) != 0) {
		fprintf(stderr, "data invalid! \r\n");
		return 0;
	}
	if ((int)mDataNum != reg_num) {
		fprintf(stderr, "data num invalid! \r\n");
		return 0;
	}

	fprintf(stderr, "reg:0x%04x num:%04d \r\n", reg_addr, reg_num);

	// send to
	int ret;
	ret = mMbiDevice->mSpiDevice0->writeRegisterBatch(reg_addr, mDataBuffer, reg_num);
	if (ret != 0) {
		fprintf(stderr, "spi0 writePanelReg error! \r\n");
	} else {
		fprintf(stderr, "spi0 writePanelReg ok! \r\n");
	}
	ret = mMbiDevice->mSpiDevice1->writeRegisterBatch(reg_addr, mDataBuffer, reg_num);
	if (ret != 0) {
		fprintf(stderr, "spi1 writePanelReg error! \r\n");
	} else {
		fprintf(stderr, "spi1 writePanelReg ok! \r\n");
	}

	return 0;
}

// r d6a0 12
// r e100 40
//CL_func_IMPL(TestCmdMiniLed, r, TestCmdMiniLed) {
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, r);
int TestCmdMiniLed::r(int argc, char *const*argv) {
	UNUSED_(argc); UNUSED_(argv);
	if (argc != 3) {
		fprintf(stderr, "should input: r [reg_addr(hex)] [reg_num(dec)] \r\n");
		return 0;
	}
	int reg_addr = _parse_hex_str(argv[1], cycling::FLAG_U16);
	if (reg_addr < 0) {
		fprintf(stderr, "reg_addr invalid! \r\n");
		return 0;
	}
	int reg_num = 0;
	if (cycling::_c__atoi(argv[2], &reg_num) < 0) {
		fprintf(stderr, "reg_num invalid! \r\n");
		return 0;
	}
	if (reg_num <= 0 || reg_num > DATA_BUFFER_SIZE) {
		fprintf(stderr, "reg_num invalid! \r\n");
		return 0;
	}

	int ret_len = 0;
	int ret = 0;
	
	fprintf(stderr, "-----------------------------------------------------\r\n");
	ret = mMbiDevice->mSpiDevice0->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	fprintf(stderr, "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "                ", 16);
	}
	ret = mMbiDevice->mSpiDevice1->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	fprintf(stderr, "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "                ", 16);
	}

	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, bl);
int TestCmdMiniLed::bl(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		fprintf(stderr, "input invalid! \r\n");
		return 0;
	}
	int color_ = 0;
	if (cycling::_c__atoi(argv[1], &color_) < 0) {
		fprintf(stderr, "color_ invalid! \r\n");
		return 0;
	}
	if (color_ < 0 || color_ > 65535) {
		fprintf(stderr, "color_ invalid! \r\n");
		return 0;
	}

	mMbiDevice->backlight_data_fill(color_);
	mMbiDevice->backlightCommit(1);
	return 0;
}

CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test1);
int TestCmdMiniLed::test1(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	int pin_num = 0;
	if (cycling::_c__atoi(argv[1], &pin_num) < 0) {
		fprintf(stderr, "pin_num parse invalid! \r\n");
		return 0;
	}
	if (pin_num == 5
		|| pin_num == 59
		|| pin_num == 60
		|| pin_num == 61
		|| pin_num == 62
		|| pin_num == 67
		|| pin_num == 68
		|| pin_num == 56
		|| pin_num == 66
		) {
	} else {
		fprintf(stderr, "pin_num invalid! \r\n");
		return 0;
	}

	int pin_state = 0;
	if (cycling::_c__atoi(argv[2], &pin_state) < 0) {
		fprintf(stderr, "pin_state parse invalid! \r\n");
		return 0;
	}
	if (pin_state != 0 && pin_state != 1) {
		fprintf(stderr, "pin_state invalid! \r\n");
		return 0;
	}
	fprintf(stderr, "pin_num:%d state:%d \r\n", pin_num, pin_state);
	PIN_OUT_VALUE(GPIOSYS_PINCTRL_BASE+pin_num, pin_state);

	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test2);
int TestCmdMiniLed::test2(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		fprintf(stderr, "should input: test2 [test_num(dec)] \r\n");
		return 0;
	}
	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test3);
int TestCmdMiniLed::test3(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		fprintf(stderr, "should input: test3 [test_num(dec)] \r\n");
		return 0;
	}

	return 0;
}

CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test4);
int TestCmdMiniLed::test4(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		fprintf(stderr, "should input: test4 [test_num(dec)] \r\n");
		return 0;
	}

	int num_ = 0;
	if (cycling::_c__atoi(argv[1], &num_) < 0) {
		fprintf(stderr, "num_ parse invalid! \r\n");
		return 0;
	}
	if (num_ <= 0 || num_ >= 60000) {
		fprintf(stderr, "num_ invalid! \r\n");
		return 0;
	}
	
	for(int i=0;i<num_;i++) {
		mMbiDevice->mSpiDevice0->writeRegister(0x0f, 0x01);
		mMbiDevice->mSpiDevice1->writeRegister(0x0f, 0x01);
	}
	return 0;
}




};

