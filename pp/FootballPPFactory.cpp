
#include "FootballConfig.h"

#include "FootballPPFactory.h"

#include "impl/FootballPPCpu.h"
#include "impl/FootballPPGles.h"
#include "impl/FootballPPVk.h"

#undef __CLASS__
#define __CLASS__ "FootballPPFactory"

namespace football {

/*static*/ std::string FootballPPFactory::getPPTypeDesc(int type) {
   if(PP_CPU == type) {
	   return "PP_CPU";
   }
   else if(PP_GLES == type) {
	   return "PP_GLES";
   }
   else if(PP_VK == type) {
	   return "PP_VK";
   }
   return "PP_none";
}

/*static*/ FootballPP* FootballPPFactory::createFootballPP(int type) {
	DLOGD( "createFootballPP:%s \r\n", getPPTypeDesc(type).c_str());
	if(PP_CPU == type) {
		return new FootballPPCpu();
	}
	else if(PP_GLES == type) {
		return new FootballPPGles();
	}
	else if(PP_VK == type) {
		return new FootballPPVk();
	}
	return nullptr;
}


void SessionInfo::_on_frame_call() {
	if (cb_ != nullptr) {
		(*cb_)(cb_ctx);
	}
}

/*static*/ int FootballPP::s_SessionId_generator = 0;
int FootballPP::addSession(FootballSession *session_) {
	int session_id = ++s_SessionId_generator;
	mSessionIds.push_back(session_id);
	mSessions.insert(std::pair<int, FootballSession*>(session_id, session_)); // *** Note: if the key is exist , insert will failed !!!
	return session_id;
}
int FootballPP::removeSession(int session_id) {
	// should wait this session's processing is finished !
	int found;
	DLOGD( "closeSession id:%d \r\n", session_id);

	{
		found = 0;
		std::map<int, FootballSession*>::iterator iter2;
		/*
		for (iter2 = mSessions.begin(); iter2 != mSessions.end(); iter2++) {
			//cout << iter2->first << ": " << iter2->second << endl;
			if (iter2->first == session_id) {
				FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
			}
		}
		*/
		iter2 = mSessions.find(session_id);
		if (iter2 != mSessions.end()) {
			found = 1;
			DLOGD( "mSessions %d found \r\n", session_id);
			FootballSession* session_ = (FootballSession*)iter2->second;
			mSessions.erase(session_id);
			delete session_; // ### delete ###
		}
		
	}

	{
		found = 0;
		std::vector<int>::iterator iter1 = std::find(mSessionIds.begin(),mSessionIds.end(), session_id);
		if(iter1 != mSessionIds.end()) {
			mSessionIds.erase(iter1);
			found = 1;
			DLOGD( "mSessionIds %d found \r\n", session_id);
		}
	}



	return 0;
}

int FootballPP::closeSession(int session_id) {
	return removeSession(session_id);
}
std::vector<int> FootballPP::getSessionIds() {
	return mSessionIds;
}
int FootballPP::getSession(int session_id, SessionInfo *&session) {
	return 0;
}
int FootballPP::setSessionParameter(int session_id, SessionParameter *parameter) {
	std::map<int, FootballSession*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootballSession* session_ = (FootballSession*)iter2->second;
		session_->setSessionParameter(parameter);
	}
	return -1;
}
int FootballPP::getSessionParameter(int session_id, SessionParameter *parameter) {
	std::map<int, FootballSession*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootballSession* session_ = (FootballSession*)iter2->second;
		session_->getSessionParameter(parameter);
	}
	return -1;
}
void FootballPP::print(int session_id) {
	std::map<int, FootballSession*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootballSession* session_ = (FootballSession*)iter2->second;
		session_->print();
	}
}



};


