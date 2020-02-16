#ifndef __FOOTBALL_BL_CONTROLLER_H__
#define __FOOTBALL_BL_CONTROLLER_H__



namespace football {

class FootballBlController {
public:
	FootballBlController() {}
	virtual ~FootballBlController() {}

	virtual int initialize() = 0;
	virtual int unInitialize() = 0;
	virtual int backlightSetData(int x, int y, int data_) = 0;
	virtual int backlightCommit(int mapType = 0) = 0;
};


};



#endif

