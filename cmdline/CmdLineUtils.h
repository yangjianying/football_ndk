#ifndef __CMDLINE_UTILS_H____
#define __CMDLINE_UTILS_H____

#include  <stdio.h>
#include  <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <iostream>// for std::cout, std::endl
#include <fstream>
#include <string> // for std::string
#include <vector>// for std::vector
#include <numeric> // for std::accumulate
#include <list>
#include <iomanip>
#include <thread>


namespace cycling {

#define DELAY_Millis(s) do{ if (s > 0) {usleep(s*1000); } }while(0)
#define DELAY_Seconds(s) do{ DELAY_Millis(s*1000); }while(0)
#define DELAY_Minutes(s) do{ DELAY_Seconds(s*60); }while(0)

typedef unsigned long long ULL;
std::string getThreadIdOfString(const std::thread::id & id);
ULL getThreadIdOfULL(const std::thread::id & id);
ULL getThreadCurrentIdOfULL();

#ifdef __cplusplus
extern "C" {
#endif

int utils_load_bmp(const char *bmpname, uint32_t *outWidth, uint32_t *outHeight, 
	unsigned char **outData, uint32_t *outLength);

#ifdef __cplusplus
	}
#endif

enum {
	FLAG_U8 = 0x01,
	FLAG_U16 = 0x02,
	FLAG_ANY_ = 0x04,
};

int _parse_hex_str(char *s, int flags);
int getData_(int argc, char * const *argv, uint8_t *buf, uint32_t buf_len, uint32_t *dataNum);
int getData16_(int argc, char * const *argv, uint16_t *buf, uint32_t buf_len, uint32_t *dataNum);


// Subset of http://msdn.microsoft.com/en-us/library/system.text.stringbuilder.aspx
template<typename chr>
class StringBuilder{
	typedef std::basic_string<chr> string_t;
	typedef std::list<string_t> container_t;// Reasons not to use vector below.
	typedef typename string_t::size_type size_type;// Reuse the size type in the string.

	container_t m_Data;
	size_type m_totalSize;
	void append(const string_t&src){
		m_Data.push_back(src);
		m_totalSize+= src.size();
	}

	// No copy constructor, no assignement.
	StringBuilder(const StringBuilder&);
	StringBuilder& operator= (const StringBuilder&);

public:
	StringBuilder(const string_t&src){
		if(!src.empty()){
			m_Data.push_back(src);
		}
		m_totalSize= src.size();
	}

	StringBuilder(){
		m_totalSize= 0;
	}

	// TODO: Constructor that takes an array of strings.
	StringBuilder& Append(const string_t&src){
		append(src);
		return*this;// allow chaining.
	}

	// This one lets you add any STL container to the string builder.
	template<class inputIterator>
	StringBuilder& Add(const inputIterator&first,const inputIterator&afterLast){
		// std::for_each and a lambda look like overkill here.
		// <b>Not</b> using std::copy, since we want to update m_totalSize too.
		for(inputIterator f= first;f!= afterLast;++f){
			append(*f);
		}
		return*this;// allow chaining.
	}

	StringBuilder& AppendLine(const string_t&src){
		static chr lineFeed[]{10,0};// C++ 11. Feel the love!
		m_Data.push_back(src+ lineFeed);
		m_totalSize+= 1+ src.size();
		return*this;// allow chaining.
	}

	StringBuilder& AppendLine(){
		static chr lineFeed[]{10,0};
		m_Data.push_back(lineFeed);
		++m_totalSize;
		return*this;// allow chaining.
	}

	// TODO: AppendFormat implementation. Not relevant for the article.

	// Like C# StringBuilder.ToString()

	// Note the use of reserve() to avoid reallocations.

	string_t ToString()const{
		string_t result;
		// The whole point of the exercise!
		// If the container has a lot of strings, reallocation (each time the result grows) will take a serious toll,
		// both in performance and chances of failure.
		// I measured (in code I cannot publish) fractions of a second using 'reserve', and almost two minutes using +=.
		result.reserve(m_totalSize+ 1);
		// result = std::accumulate(m_Data.begin(), m_Data.end(), result); // This would lose the advantage of 'reserve'
		for(auto iter= m_Data.begin();iter!= m_Data.end();++iter){
			result+= *iter;
		}
		return result;
	}

	// like java Array.join()
	string_t Join(const string_t&delim)const{
		if(delim.empty()){
			return ToString();

		}

		string_t result;
		if(m_Data.empty()){
			return result;
		}

		// Hope we don't overflow the size type.

		size_type st= (delim.size()* (m_Data.size()- 1))+ m_totalSize+ 1;

		result.reserve(st);

		// If you need reasons to love C++11, here is one.

		struct adder{
			string_t m_Joiner;
			adder(const string_t&s): m_Joiner(s){
				// This constructor is NOT empty.
			}
			// This functor runs under accumulate() without reallocations, if 'l' has reserved enough memory.
			string_t operator()(string_t&l,const string_t&r){
				l+= m_Joiner;
				l+= r;
				return l;

			}

		}adr(delim);

		auto iter= m_Data.begin();
		// Skip the delimiter before the first element in the container.
		result+= *iter;
		return std::accumulate(++iter,m_Data.end(),result,adr);

	}

};// class StringBuilder


static inline bool exists_test0 (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}
 
static inline bool exists_test1 (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}
 
static inline bool exists_test2 (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}
 
static inline bool exists_test3 (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

void print2log_bytes(std::vector<uint8_t> &buf, const char * prefix);
void print2log_bytes(uint8_t *buf, uint32_t length, const char * prefix);

void print2log_bytes(std::vector<uint8_t> &buf, const char * prefix, const uint32_t line_num);
void print2log_bytes(uint8_t *buf, uint32_t length, const char * prefix, const uint32_t line_num);
void print2log_bytes(uint16_t *buf, uint32_t length, const char * prefix, const uint32_t line_num);

std::string getPrefixSpace(int numSpaces);

std::string & std_string_format(std::string & _str, const char * _Format, ...) ;

int _c__atoi(char *s, int *out_result);
int _c__atoi(char *s, uint32_t *out_result);
int _c__atol(char *s, long *out_result);


};

#endif
