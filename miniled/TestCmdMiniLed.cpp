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

#include "FootballConfig.h"

#include "cmdline/CmdLineUtils.h"

#include "mbi6322_reference_settings.h"

#include "TestCmdMiniLed.h"


#undef UNUSED_
#define UNUSED_(x) ((void)x)

#undef __CLASS__
#define __CLASS__ "TestCmdMiniLed"


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
	  //DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
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
	  DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
	  return -1;
	}
    if((read(fd, buffer, sizeof(buffer))) < 0) {
		DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
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
	//FrLOGV( "%s, access:0x%02x", gpiosys_path_, access_flags);
	if (access_flags == 0x0f) {

		#if 0
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/direction", number);
		FrLOGV( "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
		
		memset(gpiosys_path_, 0, 256);
		snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
		FrLOGV( "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
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
	FrLOGV( "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
	
	memset(gpiosys_path_, 0, 256);
	snprintf(gpiosys_path_, 256, "/sys/class/gpio/gpio%d/value", number);
	FrLOGV( "%s, access:0x%02x", gpiosys_path_, get_file_access_flags(gpiosys_path_));
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
		  DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
		  return -1;
		}
		s_fd_map[number - GPIOSYS_PINCTRL_BASE] = fd;
		//DLOGD( "%04d = %d \r\n", number -GPIOSYS_PINCTRL_BASE , fd);
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
		  DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
		  return 0;
		}
		s_fd_map[number - GPIOSYS_PINCTRL_BASE] = fd;
		//DLOGD( "%04d = %d \r\n", number - GPIOSYS_PINCTRL_BASE, fd);
	}
	fd = s_fd_map[number - GPIOSYS_PINCTRL_BASE];
	
	char buffer[8];
	int value_;
    if((read(fd, buffer, sizeof(buffer))) < 0) {
		//DLOGD( "error:%s, %d \r\n", __func__, __LINE__);
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

static int initControl_gpio_pins_flag = 0;
static void initControl_gpio_pins() {
	if (initControl_gpio_pins_flag == 0) {
	initControl_gpio_pins_flag = 1;
	DLOGD( "%s first call ... \r\n", __func__);
	for(int i=0;i<s_GPIOSYS_output_pins_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_output_pins[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_output_(s_GPIOSYS_output_pins[i]);
			if (direction_ret == 0) {
				gpiosys_output_(s_GPIOSYS_output_pins[i], 0);
			}
		}
		//FrLOGV( "out gpio: %02d export :%d dir: %s", s_GPIOSYS_output_pins[i] - GPIOSYS_PINCTRL_BASE, export_ret
		//	, (direction_ret == 0 ? "ok" : "failed"));
	}
#if 0
	for(int i=0;i<s_GPIOSYS_input_pins_SIZE;i++) {
		int direction_ret = -1;
		int export_ret = gpiosys_export_(s_GPIOSYS_input_pins[i]);
		if (export_ret == 0 || export_ret == 1) {
			direction_ret = gpiosys_direction_input_(s_GPIOSYS_input_pins[i]);
		}
		FrLOGV( "in gpio: %02d export :%d dir: %s", s_GPIOSYS_input_pins[i] - GPIOSYS_PINCTRL_BASE, export_ret
			, (direction_ret == 0 ? "ok" : "failed"));
	}
#endif
	}
	else {
		DLOGD( "%s already called. \r\n", __func__);
	}
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

	SpiDevice(const char * device_): mDevice(device_), mFd(open(mDevice, O_RDWR)) {
		//FrLOGV( "%s,%d  mFd:%d ", __func__, __LINE__, mFd);
		initControl_gpio_pins();

		if (mFd > 0) {
			setup();
		}
	}
	virtual ~SpiDevice() {
		close(mFd);
		//FrLOGV( "%s,%d", __func__, __LINE__);
	}

	void pabort(const char *s) {
		DLOGD( "*** pabort:%s \r\n", s);
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

		DLOGD( "spi mode: 0x%02x/0x%02x \n", mode_, mode_r);
		DLOGD( "bits per word: %d/%d \n", bits_, bits_r);
		DLOGD( "max speed: %d Hz (%d KHz) / %d Hz (%d KHz) \n", 
			speed_, speed_/1000, speed_r, speed_r/1000);
		DLOGD( "lsb_ : %d \r\n" , lsb_);

		mValid = 1;
	}

#if 1
// 16bits spidev test ok !!!
int transfer(uint16_t* txBuffer, uint16_t* rxBuffer, uint32_t size_) {
	if (mValid == 0) {
		DLOGD( "%s,%d error!\r\n", __func__, __LINE__);
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
	//DLOGD( "%s %u done! \r\n", __func__, size_);
	return 0;
}
virtual int readRegister(uint16_t address, uint16_t *value_) override {
	if (mValid == 0) {
		DLOGD( "%s,%d error!\r\n", __func__, __LINE__);
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
		DLOGD( "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	DLOGD( "%s len = %d \n", __func__, len);
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
		//DLOGD( "%s,%d error!\r\n", __func__, __LINE__);
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
		DLOGD( "%s,%d error!\r\n", __func__, __LINE__);
		return -1;
	}
	DLOGD( "%s len = %d \n", __func__, len);
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
		//DLOGD( "%s %u done! \r\n", __func__, size_);
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
		DLOGD( "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
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
		DLOGD( "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
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
		//DLOGD( "-> %02X %02X %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1], mTxStagingBuffer[2], mTxStagingBuffer[3]);
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
		DLOGD( "-> %02X %02X %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1], mTxStagingBuffer[2], mTxStagingBuffer[3]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}
#endif

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
	if (initSpi_gpio_pins_flag == 0) {
		DLOGD( "%s first call ... \r\n", __func__);
		initSpi_gpio_pins_flag = 1;
		//FrLOGV( "%s,%d", __func__, __LINE__);
		for(int i=0;i<s_GPIOSYS_output_pins_spi_SIZE;i++) {
			int direction_ret = -1;
			int export_ret = gpiosys_export_(s_GPIOSYS_output_pins_spi[i]);
			if (export_ret == 0 || export_ret == 1) {
				direction_ret = gpiosys_direction_output_(s_GPIOSYS_output_pins_spi[i]);
				if (direction_ret == 0) {
					gpiosys_output_(s_GPIOSYS_output_pins_spi[i], 0);
				}
			}
			//FrLOGV( "spi out gpio: %02d export :%d dir: %s", s_GPIOSYS_output_pins_spi[i] - GPIOSYS_PINCTRL_BASE, export_ret
			//	, (direction_ret == 0 ? "ok" : "failed"));
		}
#if 1
		for(int i=0;i<s_GPIOSYS_input_pins_spi_SIZE;i++) {
			int direction_ret = -1;
			int export_ret = gpiosys_export_(s_GPIOSYS_input_pins_spi[i]);
			if (export_ret == 0 || export_ret == 1) {
				direction_ret = gpiosys_direction_input_(s_GPIOSYS_input_pins_spi[i]);
			}
			//FrLOGV( "spi in gpio: %02d export :%d dir: %s", s_GPIOSYS_input_pins_spi[i] - GPIOSYS_PINCTRL_BASE, export_ret
			//	, (direction_ret == 0 ? "ok" : "failed"));
		}
#endif
		PIN_OUT_VALUE(PIN_CHIP_CS1_N, 1);
		PIN_OUT_VALUE(PIN_CHIP_CS2_N, 1);
		PIN_OUT_VALUE(PIN_CHIP_SCLK, 0);
		PIN_OUT_VALUE(PIN_CHIP_MOSI, 0);
	}
	else {
		DLOGD( "%s already called \r\n", __func__);
	}
}

class SpiDeviceGpio: public SpiDeviceIntf {
public:
	int mCs = 0;
	volatile uint32_t mTxStagingCnt = 0;
	volatile uint8_t mTxStagingBuffer[STAGING_SIZE];
	volatile uint8_t mRxStagingBuffer[STAGING_SIZE];

	SpiDeviceGpio(int cs_): mCs(cs_) {
		DLOGD( "%s,%d ", __func__, __LINE__);
		initSpi_gpio_pins();
	}
	virtual ~SpiDeviceGpio() {
		DLOGD( "%s,%d ", __func__, __LINE__);
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
			DLOGD( "%04d - tx:%02x ", i, tx_);
#endif
			for(int clk_ = 0; clk_ < 8; clk_++) {
#if transfer_debug
				DLOGD( "%02x%s ", shift_bit, (tx_&shift_bit ? "(1)" : "(0)"));
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
			DLOGD( "rx:%02x ", rx_);
			DLOGD( "\r\n");
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
		DLOGD( "%s:0x%04x \r\n", __func__, address);
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		mTxStagingBuffer[mTxStagingCnt++] = 0;
		DLOGD( "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		if (ret == 0) {
			uint16_t r_value = mRxStagingBuffer[2];
			r_value <<= 8;
			r_value += mRxStagingBuffer[3];
			*value_ = r_value;
		}
		//DLOGD( "read %04x : %04x \r\n", address, *value_);
		return ret;	
	}
	virtual int readRegisterBatch(uint16_t address, int len, uint16_t *value_, int *r_len) override {
		UNUSED_(address); UNUSED_(len); UNUSED_(value_); UNUSED_(r_len);
		DLOGD( "%s:0x%04x len=%d \r\n", __func__, address, len);
		int ret = 0;
		address <<= 1; address += 1;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = 0;
			mTxStagingBuffer[mTxStagingCnt++] = 0;
		}
		DLOGD( "-> %02X %02X \r\n", mTxStagingBuffer[0], mTxStagingBuffer[1]);
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
		DLOGD( "%s:0x%04x value_=0x%04x \r\n", __func__, address, value_);
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (value_>>0)&0xff;
		DLOGD( "%s, -> %02X %02X \r\n", __func__, mTxStagingBuffer[0], mTxStagingBuffer[1]);
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}
	virtual int writeRegisterBatch(uint16_t address, uint16_t* value_, int len) override {
		DLOGD( "%s:0x%04x len=%d \r\n", __func__, address, len);
		UNUSED_(address); UNUSED_(len); UNUSED_(value_);
		int ret = 0;
		address <<= 1; address += 0;
		mTxStagingCnt = 0;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>8)&0xff;
		mTxStagingBuffer[mTxStagingCnt++] = (address>>0)&0xff;
		DLOGD( "%s, -> %02X %02X \r\n", __func__, mTxStagingBuffer[0], mTxStagingBuffer[1]);
		for(int i=0;i<len;i++) {
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>8)&0xff;
			mTxStagingBuffer[mTxStagingCnt++] = (value_[i]>>0)&0xff;
		}
		ret = transfer(mTxStagingBuffer, mRxStagingBuffer, mTxStagingCnt);
		return ret;
	}


};


//#include "reg_override_1.h"
//#include "reg_override_old.h"
//#include "reg_override_old_10.h"
#include "reg_override_old_10_dead_dummy.h"

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

	DLOGD( "mbi6322_initial_settings_reg_size:%d (*2=%d) / real:%d \r\n", mbi6322_initial_settings_reg_size,
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
		//DLOGD( "[%04X] : %04X \r\n", addr_, value_);
	}

	//mSpiDevice0->writeRegister(0x0f, 0x01);
	DLOGD( "spi0 initial settings done! \r\n");
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
		//DLOGD( "[%04X] : %04X \r\n", addr_, value_);
	}
	//mSpiDevice1->writeRegister(0x0f, 0x01);
	DLOGD( "spi1 initial settings done! \r\n");
#endif

	backlight_data_fill(get_PWM_data_bit_max_value() - 1);
	backlightCommit(1);


#if 1  // dump
{
	uint16_t reg_addr = 0x0000;
	int reg_num = 0x0f;
	int ret_len = 0;
	int ret = 0;
	DLOGD( "----------------------------------------------------- \r\n");
	ret = mSpiDevice0->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	DLOGD( "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "				", 16);
	}
	ret = mSpiDevice1->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	DLOGD( "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
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
	DLOGD( "%s mapType = %d ... \r\n", __func__, mapType);

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
	DLOGD( "    max_ = %d \r\n", max_);

	mSpiDevice0->writeRegisterBatch(0x10, mbiData0, CHANNEL_NUM*8);
	mSpiDevice1->writeRegisterBatch(0x30, mbiData1, CHANNEL_NUM*7);

	mSpiDevice0->writeRegister(0x0f, 0x01);
	mSpiDevice1->writeRegister(0x0f, 0x01);

	DLOGD( "%s done! \r\n", __func__);
	return 0;
}
void MbiDevice::backlight_data_fill(int data_) {
	DLOGD( "%s, data_ = %5d \r\n", __func__, data_);
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
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	mMbiDevice = new MbiDevice();
	mValid = 1;
}
TestCmdMiniLed::~TestCmdMiniLed() {
	delete mMbiDevice;
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
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
	CL_ADD_FUNC(mCmdline, vsync);

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
	DLOGD( "build at %s %s \r\n", __DATE__, __TIME__);
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
		DLOGD( "should input: w [reg_addr(hex)] [reg_num(dec)] hex#1 ... hex#reg_num \r\n");
		return 0;
	}
	int reg_addr = _parse_hex_str(argv[1], cycling::FLAG_U16);
	if (reg_addr < 0) {
		DLOGD( "reg_addr invalid! \r\n");
		return 0;
	}
	int reg_num = 0;
	if (cycling::_c__atoi(argv[2], &reg_num) < 0) {
		DLOGD( "reg_num invalid! \r\n");
		return 0;
	}
	if (reg_num <= 0 || reg_num != (argc - 3)) {
		DLOGD( "reg_num invalid! \r\n");
		return 0;
	}

	if(cycling::getData16_(argc -3, argv + 3, mDataBuffer, DATA_BUFFER_SIZE, &mDataNum) != 0) {
		DLOGD( "data invalid! \r\n");
		return 0;
	}
	if ((int)mDataNum != reg_num) {
		DLOGD( "data num invalid! \r\n");
		return 0;
	}

	DLOGD( "reg:0x%04x num:%04d \r\n", reg_addr, reg_num);

	// send to
	int ret;
	ret = mMbiDevice->mSpiDevice0->writeRegisterBatch(reg_addr, mDataBuffer, reg_num);
	if (ret != 0) {
		DLOGD( "spi0 writePanelReg error! \r\n");
	} else {
		DLOGD( "spi0 writePanelReg ok! \r\n");
	}
	ret = mMbiDevice->mSpiDevice1->writeRegisterBatch(reg_addr, mDataBuffer, reg_num);
	if (ret != 0) {
		DLOGD( "spi1 writePanelReg error! \r\n");
	} else {
		DLOGD( "spi1 writePanelReg ok! \r\n");
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
		DLOGD( "should input: r [reg_addr(hex)] [reg_num(dec)] \r\n");
		return 0;
	}
	int reg_addr = _parse_hex_str(argv[1], cycling::FLAG_U16);
	if (reg_addr < 0) {
		DLOGD( "reg_addr invalid! \r\n");
		return 0;
	}
	int reg_num = 0;
	if (cycling::_c__atoi(argv[2], &reg_num) < 0) {
		DLOGD( "reg_num invalid! \r\n");
		return 0;
	}
	if (reg_num <= 0 || reg_num > DATA_BUFFER_SIZE) {
		DLOGD( "reg_num invalid! \r\n");
		return 0;
	}

	int ret_len = 0;
	int ret = 0;
	
	DLOGD( "-----------------------------------------------------\r\n");
	ret = mMbiDevice->mSpiDevice0->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	DLOGD( "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "                ", 16);
	}
	ret = mMbiDevice->mSpiDevice1->readRegisterBatch(reg_addr, reg_num, mDataBuffer, &ret_len);
	DLOGD( "readRegister:0x%04x len:%04d ret: %d read_length:%d \r\n", reg_addr, reg_num, ret, ret_len);
	if (ret_len > 0) {
		cycling::print2log_bytes(mDataBuffer, ret_len, "                ", 16);
	}

	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, bl);
int TestCmdMiniLed::bl(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		DLOGD( "input invalid! \r\n");
		return 0;
	}
	int color_ = 0;
	if (cycling::_c__atoi(argv[1], &color_) < 0) {
		DLOGD( "color_ invalid! \r\n");
		return 0;
	}
	if (color_ < 0 || color_ > 65535) {
		DLOGD( "color_ invalid! \r\n");
		return 0;
	}

	mMbiDevice->backlight_data_fill(color_);
	mMbiDevice->backlightCommit(1);
	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, vsync);
int TestCmdMiniLed::vsync(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	int num_ = 1;
	long wait_time = 33;
	if (argc == 2) {
		if (cycling::_c__atoi(argv[1], &num_) < 0) {
			DLOGD( "num_ parse invalid! \r\n");
			return 0;
		}
		if (num_ <= 0 || num_ >= 60000) {
			DLOGD( "num_ invalid! \r\n");
			return 0;
		}
	} else if(argc == 3) {
			if (cycling::_c__atoi(argv[1], &num_) < 0) {
				DLOGD( "num_ parse invalid! \r\n");
				return 0;
			}
			if (num_ <= 0 || num_ >= 60000) {
				DLOGD( "num_ invalid! \r\n");
				return 0;
			}
			
			if (cycling::_c__atol(argv[1], &wait_time) < 0) {
				DLOGD( "wait_time parse invalid! \r\n");
				return 0;
			}
			if (wait_time <= 0 || wait_time >= 10000) {
				DLOGD( "wait_time invalid! \r\n");
				return 0;
			}
	} else if (argc > 3) { DLOGD( "invalid input! \r\n"); return 0; }

	for(int i=0;i<num_;i++) {
		mMbiDevice->mSpiDevice0->writeRegister(0x0f, 0x01);
		mMbiDevice->mSpiDevice1->writeRegister(0x0f, 0x01);
		usleep(wait_time*1000);
	}
	return 0;
}

CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test1);
int TestCmdMiniLed::test1(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	int pin_num = 0;
	if (cycling::_c__atoi(argv[1], &pin_num) < 0) {
		DLOGD( "pin_num parse invalid! \r\n");
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
		DLOGD( "pin_num invalid! \r\n");
		return 0;
	}

	int pin_state = 0;
	if (cycling::_c__atoi(argv[2], &pin_state) < 0) {
		DLOGD( "pin_state parse invalid! \r\n");
		return 0;
	}
	if (pin_state != 0 && pin_state != 1) {
		DLOGD( "pin_state invalid! \r\n");
		return 0;
	}
	DLOGD( "pin_num:%d state:%d \r\n", pin_num, pin_state);
	PIN_OUT_VALUE(GPIOSYS_PINCTRL_BASE+pin_num, pin_state);

	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test2);
int TestCmdMiniLed::test2(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		DLOGD( "should input: test2 [test_num(dec)] \r\n");
		return 0;
	}
	return 0;
}
CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test3);
int TestCmdMiniLed::test3(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		DLOGD( "should input: test3 [test_num(dec)] \r\n");
		return 0;
	}

	return 0;
}

CL_SFUNC_IMPL(TestCmdMiniLed, TestCmdMiniLed, test4);
int TestCmdMiniLed::test4(int argc, char *const*argv) { UNUSED_(argc); UNUSED_(argv);
	if (argc != 2) {
		DLOGD( "should input: test4 [test_num(dec)] \r\n");
		return 0;
	}

	int num_ = 0;
	if (cycling::_c__atoi(argv[1], &num_) < 0) {
		DLOGD( "num_ parse invalid! \r\n");
		return 0;
	}
	if (num_ <= 0 || num_ >= 60000) {
		DLOGD( "num_ invalid! \r\n");
		return 0;
	}
	
	for(int i=0;i<num_;i++) {
		mMbiDevice->mSpiDevice0->writeRegister(0x0f, 0x01);
		mMbiDevice->mSpiDevice1->writeRegister(0x0f, 0x01);
	}
	return 0;
}


/*static*/ int TestCmdMiniLed::_main(int argc, char **argv) {
	UNUSED_(argc);
	UNUSED_(argv);
	//FrLOGV( "%s,%d", __func__, __LINE__);

    static const struct option longOptions[] = {
        { "help",               no_argument,        NULL, 'h' },
        { "verbose",            no_argument,        NULL, 'v' },
        { "capi",               no_argument,  NULL, 'c' },
        { "qdcm",               no_argument,  NULL, 'q' },
        { "dummy",               required_argument,  NULL, 'd' },
        { NULL,                 0,                  NULL, 0 }
    };

	int capi = 0;
	int qdcm_enable = 0;
    while (true) {
        int optionIndex = 0;
        int ic = getopt_long(argc, argv, "", longOptions, &optionIndex);
        if (ic == -1) {
            break;
        }
		switch (ic) {
		case 'h': {
			break;
		}
		case 'v': {
			break;
		}
		case 'c': {
			capi = 1;
			break;
		}
		case 'q': {
			qdcm_enable = 1;
			break;
		}
		}
    }

	{
		uint32_t initFlags = 0;
		if (qdcm_enable) {
			initFlags |= 0x8000;
		}
		if (capi) {
			initFlags |= 0x4000;
		}
		TestCmdMiniLed* _test =
			new TestCmdMiniLed(initFlags);
		if (_test->isValid()) {
			_test->loop();
		}
		else {
			//FrLOGV( "%s,%d not valid!", __func__, __LINE__);
		}
		delete _test;
	}


	return 0;
}


};

