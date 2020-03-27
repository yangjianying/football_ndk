
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <termios.h>
#include <unistd.h>


#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>// for std::cout, std::endl
#include <fstream>
#include <sstream>
#include <string> // for std::string
#include <vector>// for std::vector
#include <numeric> // for std::accumulate
#include <cmath>
#include <iomanip>
#include <list>

#include "utils/football_debugger.h"

#include "CmdLineUtils.h"

#undef __CLASS__
#define __CLASS__ "CmdLineUtils"

namespace cycling {

std::string getThreadIdOfString(const std::thread::id & id) {
    std::stringstream sin;
    sin << id;
    return sin.str();
}
ULL getThreadIdOfULL(const std::thread::id & id) {
    return std::stoull(getThreadIdOfString(id));
}

ULL getThreadCurrentIdOfULL() {
	return getThreadIdOfULL(std::this_thread::get_id());
}


int _parse_hex_str(char *s, int flags) {
	int r = 0;
	if (s == NULL) {
		return 0;
	}
	if (flags & FLAG_U8) {
		if (strlen(s) > 2) {
			return -1;
		}
	}
	else if (flags & FLAG_U16) {
		if (strlen(s) > 4) {
			return -1;
		}
	}
	else if(flags & FLAG_ANY_) {
		if (strlen(s) <= 0) {
			return -1;
		}
	}
	else { // not defined
		return -1;
	}
	int slen = strlen(s);
	for(int i=0;i<slen;i++) {
		if(s[i] >= '0' && s[i] <= '9') { r <<= 4; r += s[i] - '0';}
		else if(s[i] >= 'a' && s[i] <= 'f') { r <<= 4; r += s[i] - 'a' + 10;}
		else if(s[i] >= 'A' && s[i] <= 'F') { r <<= 4; r += s[i] - 'A' + 10;}
		else { return -2;}
	}

	return r;
}

int getData_(int argc, char * const *argv, uint8_t *buf, uint32_t buf_len, uint32_t *dataNum) {
	if (argc <= 0) {
		return -1;
	}
	*dataNum = 0;
	uint32_t dataCnt = 0;
	int num = argc < (int)buf_len ? argc : buf_len;
	for(int i=0;i<num;i++) {
		int ret = _parse_hex_str(argv[i], FLAG_U8);
		if (ret < 0 ) {
			return -1;
		}
		buf[i] = (uint8_t)(ret&0xff);
		dataCnt++;
	}
	*dataNum = dataCnt;
	return 0;
}
int getData16_(int argc, char * const *argv, uint16_t *buf, uint32_t buf_len, uint32_t *dataNum) {
	if (argc <= 0) {
		return -1;
	}
	*dataNum = 0;
	uint32_t dataCnt = 0;
	int num = argc < (int)buf_len ? argc : buf_len;
	for(int i=0;i<num;i++) {
		int ret = _parse_hex_str(argv[i], FLAG_U16);
		if (ret < 0 ) {
			return -1;
		}
		buf[i] = (uint16_t)(ret&0xffff);
		dataCnt++;
	}
	*dataNum = dataCnt;
	return 0;
}

void print2log_bytes(std::vector<uint8_t> &buf, const char * prefix) {
	print2log_bytes(buf, prefix, 16);
}
void print2log_bytes(uint8_t *buf, uint32_t length, const char * prefix) {
	print2log_bytes(buf, length, prefix, 16);
}

void print2log_bytes(std::vector<uint8_t> &buf, const char * prefix, const uint32_t line_num) {
	int line_end = 0;
	size_t length = buf.size();
	DLOGD( "%s", prefix);
	for(size_t i = 0; i < length; i++ ) {
		line_end = 0;
		DLOGD( "%02X ", buf[i]);
		if(i % line_num == (line_num - 1)) {
			line_end = 1;
			DLOGD( "\r\n");
			if (i != (length - 1)) {
				DLOGD( "%s", prefix);
			}
		}
	}
	if (line_end == 0) {
		DLOGD( "\r\n");
	}
}

void print2log_bytes(uint8_t *buf, uint32_t length, const char * prefix, const uint32_t line_num) {
	int line_end = 0;

	DLOGD( "%s", prefix);
	for(uint32_t i = 0; i < length; i++ ) {
		line_end = 0;
		DLOGD( "%02X ", buf[i]);
		if(i % line_num == (line_num - 1)) {
			line_end = 1;
			DLOGD( "\r\n");
			if (i != (length - 1)) {
				DLOGD( "%s", prefix);
			}
		}
	}
	if (line_end == 0) {
		DLOGD( "\r\n");
	}
}
void print2log_bytes(uint16_t *buf, uint32_t length, const char * prefix, const uint32_t line_num) {
	int line_end = 0;

	DLOGD( "%s", prefix);
	for(uint32_t i = 0; i < length; i++ ) {
		line_end = 0;
		DLOGD( "%04X ", buf[i]);
		if(i % line_num == (line_num - 1)) {
			line_end = 1;
			DLOGD( "\r\n");
			if (i != (length - 1)) {
				DLOGD( "%s", prefix);
			}
		}
	}
	if (line_end == 0) {
		DLOGD( "\r\n");
	}
}

std::string getPrefixSpace(int numSpaces) {
	cycling::StringBuilder<char> sb;
	for(int i=0;i<numSpaces;i++) {
		sb.Append(" ");
	}
	return sb.ToString();
}

static int _vscprintf(const char * format, va_list pargs){
	int retval; 
	va_list argcopy; 
	va_copy(argcopy, pargs); 
	retval = vsnprintf(NULL, 0, format, argcopy); 
	va_end(argcopy); 
	return retval;

}

std::string & std_string_format(std::string & _str, const char * _Format, ...) {
	std::string tmp;
 
	//va_list marker = nullptr;
	va_list marker;
	va_start(marker, _Format);
 
	size_t num_of_chars = _vscprintf(_Format, marker);
 
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
 
	//vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);
	vsnprintf((char *)tmp.data(), tmp.capacity(), _Format, marker);
 
	va_end(marker);
 
	_str = tmp.c_str();
	return _str;
}

int _c__atoi(char *s, int *out_result) {
	if (s == NULL) {
		return -1;
	}
	int r = 0;
	while(*s) {
		int c_ = *s++;
		if (c_ >= '0' && c_ <= '9') { r *= 10; r += c_ - '0'; }
		else {
			return -1;
		}
	}
	*out_result = r;
	return 0;
}
int _c__atoi(char *s, uint32_t *out_result) {
	int r = 0;
	int ret = _c__atoi(s, &r);
	*out_result = (uint32_t)r;
	return ret;
}
int _c__atol(char *s, long *out_result) {
	if (s == NULL) {
		return -1;
	}
	long r = 0;
	while(*s) {
		int c_ = *s++;
		if (c_ >= '0' && c_ <= '9') { r *= 10; r += c_ - '0'; }
		else {
			return -1;
		}
	}
	*out_result = r;
	return 0;
}


};


