#ifndef _H_PARAMETER
#define _H_PARAMETER

class Re8map;
class Recap8;
class Pcre8;
class Token8;

#include <phpcpp.h>

namespace pun {
	 Token8* check_Token8(Php::Parameters& params, unsigned int offset=0);
	 bool option_Array(Php::Parameters& params, unsigned int offset=0);
     Re8map* check_Re8map(Php::Parameters& params, unsigned int offset=0);
     Recap8* option_Recap8(Php::Parameters& params, unsigned int offset=0);
    
     Pcre8* check_Pcre8(Php::Parameters& params,unsigned int offset=0);
     int check_IntString(Php::Parameters& params);
     bool check_String(Php::Parameters& params,unsigned int offset = 0);
     int check_Int(Php::Parameters& params,unsigned int offset = 0);
     bool option_Int(Php::Parameters& params,unsigned int offset);
     std::string missingParameter(const char* shouldBe, unsigned int offset);
     std::string invalidCharacter(char32_t unc8);
	
};



#endif