#ifndef _H_PATH
#define _H_PATH

#ifndef _H_PUN_TYPE
#include "puntype.h"
#endif

class Path : public Php::Base {
    static void setup_ext(Php::Extension& ext);
    static const char* PHP_NAME;

	static std::string OSPathSep;
	static Php::Value native(Php::Parameters& param);
	static Php::Value endSep(Php::Parameters& param);
	static Php::Value noEndSep(Php::Parameters& param);
};
#endif
