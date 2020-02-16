#ifndef __TESTCMD_MINILED_H__
#define __TESTCMD_MINILED_H__

#include "CmdLineFactory.h"

#include "FootballBlController.h"

namespace NS_test_miniled {

class MbiDevice;

//
class TestCmdMiniLed
	//: public virtual ::android::RefBase 
	: public virtual football::FootballBlController
{
public:
	CL_DEF_FUNC(date);
	CL_DEF_FUNC(pon);
	CL_DEF_FUNC(poff);
	CL_DEF_FUNC(init);
	CL_DEF_FUNC(test1);
	CL_DEF_FUNC(test2);
	CL_DEF_FUNC(test3);
	CL_DEF_FUNC(test4);
	CL_DEF_FUNC(w);
	CL_DEF_FUNC(r);
	CL_DEF_FUNC(bl);


	TestCmdMiniLed(uint32_t flags);
	virtual ~TestCmdMiniLed();

	int isValid() { return mValid; }
	void initCmd();
	int runCommand(const char * cmd);
	void loop();

	// FootballBlController impl
	virtual int initialize();
	virtual int unInitialize();
	virtual int backlightSetData(int x, int y, int data_);
	virtual int backlightCommit(int mapType = 0);


	uint32_t mInitFlags = 0;
	::android::Cmdline * mCmdline = nullptr;
	int mValid = 0;

	MbiDevice *mMbiDevice = nullptr;
	
#define DATA_BUFFER_SIZE (512)
	uint16_t mDataBuffer[DATA_BUFFER_SIZE] = {0};
	uint32_t mDataNum = 0;

};


};


#endif

